#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

sem_t print;
struct my_sems{
   sem_t mutex;
   sem_t db;
   int crossingE;
};



void cross(int baboon, int direction){
    char* way;
    if (direction){
        way = "east";
    }else{
        way = "west";
    }
    time_t now = time(0);
    printf("Baboon %d getting on rope to go %s at time %ld\n", baboon, way, now);
    sleep(1);
    now = time(0);
    printf("Baboon %d traveling %s at time %ld\n", baboon, way, now);
    sleep(4);//change to four later;
    now = time(0);
    printf("Baboon %d arrived at time %ld\n", baboon, now);
}
  
int main(){
    srand(time(NULL));
    int direction;
    
    int fd = shm_open("shmname", O_CREAT|O_RDWR, S_IRWXU);
    ftruncate(fd, sizeof(struct my_sems));
    if (fd < 0){
        perror("shm_open");
    }
    struct my_sems *semaphores = mmap(NULL, sizeof(struct my_sems), PROT_READ | PROT_WRITE,
    MAP_SHARED, fd, 0);
    sem_init(&semaphores->mutex, 1, 1);
    sem_init(&semaphores->db, 1, 1);
    semaphores->crossingE = 0;

    for(int i=0;i<100;i++){
        direction = rand()%2;
        sleep((rand()%6)+1);
        if(fork() == 0){
            if (direction){
                sem_wait(&semaphores->mutex);
                semaphores->crossingE++;
                if (semaphores->crossingE == 1){
                    sem_wait(&semaphores->db);
                }
                sem_post(&semaphores->mutex);
                cross(i, direction);
                sem_wait(&semaphores->mutex);
                semaphores->crossingE--;
                if (semaphores->crossingE == 0){
                    sem_post(&semaphores->db);
                }
                sem_post(&semaphores->mutex);
            }else{
                sem_wait(&semaphores->db);
                cross(i, direction);
                sem_post(&semaphores->db);
            }
            exit(0);
        }
    }
    shm_unlink("shmname");
      
}