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
    //int is_ready;
    //int val;
};

int buffer_size = 0;
int num_in_buffer = 0;


/*void *my_thread(void *arg) {
    struct thread_arg *a = (struct thread_arg *)arg;

    while (1) {
        pthread_mutex_lock(&a->mutex);

        while (!a->is_ready)
            pthread_cond_wait(&a->ready, &a->mutex);

        //printf("Got it: %d!\n\tThread id % 10: %d\n", a->val, pthread_self() % 10);
        a->is_ready = 0;

        pthread_mutex_unlock(&a->mutex);
    }

    return NULL;
}*/

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);  
}

void worker_func(void* args, int *buffer[], int *connfd) {
  struct thread_arg *a = (struct thread_arg *)arg;
  //fetch a request from buffer
  //call requestHandle()
  requestHandle(connfd);
  //
  //assume we already have the scope for everything we need
  while (1) {
	  //grab the mutex
	  pthread_mutex_lock(&a->mutex);
	  if (num_in_buffer == 0) {//if empty
		  pthread_cond_wait(&a->not_empty, &a->mutex);
	  }
	  //loop through the buffer to find a slot with a request
	  else {
	      for (int i = 0; i < buffer_size; i++) {
		  if (buffer[i] != 0 && connfd = buffer[i]) {
			//connfd = buffer[i];
	  		//mark the request as done
	  		buffer[i] = 0;
			num_in_buffer--;
			//signal the conditional variable the main thread is waiting on
			pthread_cond_signal(&arg.not_full);
			break;
		  }
	      }
	  //release lock
	  pthread_mutex_unlock(&a->mutex);
	  requestHandle(connfd);
  }
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  struct thread_arg *a;
  //long sleeptime = (long)arg;

  getargs(&port, argc, argv);
  int num_threads = atoi(argv[2]);
  int buffer_size = atoi(argv[3]);
  //buffer: how many request
  //thread pool : our threads

  //
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
  
  //int buffer[buffer_size];

  //we need 1 mutex lock (for the buffer) and 2 conditional variables(main thread vs worker threads)
  //  if the buffer is full, then main thread cannot write a new request to the buffer
  //  if the buffer is empty, then working threads cannot read any request from the buffer
  //we need the scope of buffer, the mutex lock and conditional variables
  //we can either pass them in as parameters, or make them global
  
  int buffer[buffer_size];

  pthread_t thread_pool[num_threads];
  for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread_pool[i], NULL, void *(*worker_func)(void* args, buffer, connfd), &arg); //*arguments foi worker_func, type void* 
	
  }
  //worker_func is a function pointer, like syscall.c in xv6

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    pthread_mutex_lock(&a->mutex);

    if (num_in_buffer == buffer_size) {
            pthread_cond_wait(&a->not_full, &a->mutex);
    } 
    else {
    	    for (int i = 0; i < buffer_size; i++) {
	        if (buffer[i] == 0) {
		      buffer[i] = connfd;
		      num_in_buffer++;
		      pthread_cond_signal(&arg.not_empty);
		      break;
	        }
	    }
    }

    pthread_mutex_unlock(&a->mutex);

    // grab the lock, go through the buffer to find an empty slot, and put connfd to that slot
    // but if the buffer is full, then the main thread needs to sleep on one conditional variable
    // when we put the connfd into the buffer, signal on the conditional variable worker threads are waiting on//worker_func is a function pointer, like syscall.c in xv6

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
