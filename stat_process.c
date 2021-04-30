#include "helper.h"
#include "request.h"

void* shm_ptr; 
slot_t* shm_slot_ptr;

void sighandler(int signum) {
  // FOR DELETING THE SHM
  shm_unlink(shm_ptr);
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "stat_process [shm_name] [sleeptime_ms] [num_threads]\n");
    exit(1);
  }

  char * shm_name  = argv[1];
  int sleeptime_ms = atoi(argv[2]);  
  int num_threads  = atoi(argv[3]);  

  int shm_fd = shm_open(shm_name, O_RDWR, 0660);
  if(shm_fd == -1){
    return 1;
  }

  ftruncate(shm_fd, getpagesize());
  shm_ptr = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  slot_t* slot_ptr = (slot_t*) shm_ptr;

  // Signal handler -> make sure to close SHM
  signal(SIGINT, sighandler);


  // \n<Iteration i>
  // <TID t1> : <Requests t1> <Static t1> <Dynamic t1>
  // <TID t2> : <Requests t2> <Static t2> <Dynamic t2>

  int count = 0;
  while(1){

    // Sleep
    sleep(sleeptime_ms / 1000);

    // Print SHM statistic
    printf("\n%d", count);
    for(int i = 0; i < num_threads; i++){
      int TID        = slot_ptr[i].TID;
      int staticReq  = slot_ptr[i].static_requests;
      int dynamicReq = slot_ptr[i].dynamic_requests;
      int req        = staticReq + dynamicReq;
      printf("%d : %d %d %d\n", TID, staticReq, dynamicReq, req);
    }

  }

}
