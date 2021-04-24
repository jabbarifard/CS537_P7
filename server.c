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

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  getargs(&port, argc, argv);

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
