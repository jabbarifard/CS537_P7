#include "helper.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

struct thread_arg {
  pthread_mutex_t mutex;
  pthread_cond_t not_full;
  pthread_cond_t not_empty;
};

int num_in_buffer = 0;
int* buffer;

int threads, buffers;

char SHM_NAME[4096];
void* shm_ptr; 
slot_t* shm_slot_ptr;


// Crtl-C handler
void sighandler(int signum) {
  
  // Delete the SHM
  munmap(SHM_NAME, getpagesize());
  shm_unlink(SHM_NAME);

  exit(0);
}

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[], int *threads, int *buffers, char *SHM_NAME)
{
  if (argc < 5) {
    fprintf(stderr, "Usage: server [port_num] [threads] [buffers] [shm_name]");
    exit(1);
  }

  // Port Check
  if (atoi(argv[1]) < 2000 || atoi(argv[1]) > 65535) {
    exit(1);
  }

  // Thread Check
  if (atoi(argv[2]) < 1) {
    exit(1);
  }

  // Buffers Check
  if (atoi(argv[3]) < 1 || atoi(argv[3]) > MAXBUF) {
    exit(1);
  }

  *port = atoi(argv[1]);
  *threads = atoi(argv[2]);
  *buffers = atoi(argv[3]);
  strcpy(SHM_NAME, argv[4]);
}


void *worker_func(void* args) {

  struct thread_arg *lock = (struct thread_arg *) args;
  
  signal(SIGINT, sighandler);

  // Put each TID into SHM slot
  pthread_mutex_lock(&lock->mutex);
  for(int i = 0; i < 32; i++){
    if(shm_slot_ptr[i].TID == 0){
      shm_slot_ptr[i].TID = (int)(pthread_self());
      break;
    }
  }
  pthread_mutex_unlock(&lock->mutex);
  
  // Server handler loop
  while (1) {
    
    int request;

	  // Grab the mutex
	  pthread_mutex_lock(&lock->mutex);

	  // If empty
	  while (num_in_buffer == 0) {
		  pthread_cond_wait(&lock->not_empty, &lock->mutex);
	  }

	  // Loop through the buffer to find a slot with a request
	  for (int i = 0; i < buffers; i++) {
		  if (buffer[i] != 0) {
        request = buffer[i];

        // Mark the request as done
        buffer[i] = 0;
        num_in_buffer--;
      
        // Signal the conditional variable the main thread is waiting on
        pthread_cond_signal(&lock->not_full);
        break;
		  }
	  }

	  // Release lock
	  pthread_mutex_unlock(&lock->mutex);

    // Handle and close connection
    requestHandle(request);
    Close(request);
  }
}

int main(int argc, char *argv[])
{

  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  getargs(&port, argc, argv, &threads, &buffers, SHM_NAME);

  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //

  // Create a new shared memory object
  int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
  if(shm_fd == -1){
    exit(1);
  }

  // Truncate sharmed memory object to a single page size

  ftruncate(shm_fd, getpagesize());
  // Map the shared memory object into the address space
  shm_ptr = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // Map the shm_ptr to an the slot array to a global variable
  shm_slot_ptr = (slot_t*) shm_ptr;

  // 
  // CS537 (Part A): Create some threads...
  //
  

  if ( threads > 1 ){

    struct thread_arg lock;

    pthread_mutex_init (&lock.mutex, NULL);
    pthread_cond_init  (&lock.not_full, NULL);
    pthread_cond_init  (&lock.not_empty, NULL);

    buffer = malloc(sizeof(int) * buffers);


    pthread_t thread_pool[threads];
    for (int i = 0; i < threads; i++) {
      pthread_create(&thread_pool[i], NULL, worker_func, &lock);
    }

    listenfd = Open_listenfd(port);
    while (1) {
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

      // 
      // CS537 (Part A): In general, don't handle the request in the main thread.
      // Save the relevant info in a buffer and have one of the worker threads 
      // do the work. Also let the worker thread close the connection.
      // 

      pthread_mutex_lock(&lock.mutex);

      while (num_in_buffer == buffers) {
        pthread_cond_wait(&lock.not_full, &lock.mutex);
      } 

      for (int i = 0; i < buffers; i++) {
        if (buffer[i] == 0) {
          buffer[i] = connfd;
          num_in_buffer++;
          pthread_cond_signal(&lock.not_empty);
          break;
        } 
      }

      pthread_mutex_unlock(&lock.mutex);

    }

  } else {
    signal(SIGINT, sighandler);
    listenfd = Open_listenfd(port);
    while (1) {
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

      // 
      // CS537 (Part A): In general, don't handle the request in the main thread.
      // Save the relevant info in a buffer and have one of the worker threads 
      // do the work. Also let the worker thread close the connection.
      // 
      requestHandle(connfd);
      Close(connfd);
    }
  }


}
