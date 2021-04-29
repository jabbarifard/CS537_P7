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

int buffer_size = 0;
int num_in_buffer = 0;
int* buffer;
struct thread_arg lock;


// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);  
}

void worker_func(void* args) {
  
  requestHandle(connfd);
  
  while (1) {
	  //grab the mutex
	  pthread_mutex_lock(&lock.mutex);
	  
	  while (num_in_buffer == 0) {//if empty
		  pthread_cond_wait(&lock.not_empty, &lock.mutex);
	  }
	  //loop through the buffer to find a slot with a request
	  for (int i = 0; i < buffer_size; i++) {
		  if (buffer[i] != 0) {
			connfd = buffer[i];
	  		//mark the request as done
	  		buffer[i] = 0;
			num_in_buffer--;
			//signal the conditional variable the main thread is waiting on
			pthread_cond_signal(&lock.not_full);
			break;
		  }
	  }
	  //release lock
	  pthread_mutex_unlock(&lock.mutex);
	  requestHandle(connfd);
	  Close(connfd);
  }
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  struct thread_arg *a;

  pthread_mutex_init(&lock.mutex, NULL);
  pthread_mutex_init(&lock.not_full, NULL);
  pthread_mutex_init(&lock.not_empty, NULL);

  getargs(&port, argc, argv);
  int num_threads = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);

  // CS537 (Part B): Create & initialize the shared memory region...
  
  // FOR CREATING THE SHM


  // Create a new shared memory object
  int shm_fd = shm_open("SHM 1122334455", O_RDWR | O_CREAT, 0660);

  // Truncate sharmed memory object to a single page size
  ftruncate(shm_fd, getpagesize());

  // Map the shared memory object into the address space
  void* shm_ptr = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


  // FOR DELETING THE SHM
  munmap(NULL, PAGESIZE);
  shm_unlink("SHM 1122334455");

  //

  // 
  // CS537 (Part A): Create some threads...
  //
  //server will allocate the buffer and creates a bunch of worker threads, worker thread will run some worker function
  //
  
  
  buffer = malloc(sizeof(int) * buffer_size);

  pthread_t thread_pool[num_threads];

  for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread_pool[i], NULL, worker_func, NULL);
  }

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    pthread_mutex_lock(&lock.mutex);

    while (num_in_buffer == buffer_size) {
            pthread_cond_wait(&lock.not_full, &lock.mutex);
    } 
    for (int i = 0; i < buffer_size; i++) {
        if (buffer[i] == 0) {
		      buffer[i] = connfd;
		      num_in_buffer++;
		      pthread_cond_signal(&lock.not_empty);
		      break;
	}
    }

    pthread_mutex_unlock(&a->mutex);

    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
    //
    //
    requestHandle(connfd);
    Close(connfd);
  }
}
