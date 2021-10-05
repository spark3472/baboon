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


int crossingE = 0;
int crossingW = 0;
sem_t print;
struct my_sems{
   sem_t mutex;
   sem_t db;
};



void cross(int baboon, int direction){
    char* way;
    if (direction){
        way = "east";
    }else{
        way = "west";
    }
    sem_wait(&print);
    time_t now = time(0);
    printf("Baboon %d traveling %s at time %ld\n", baboon, way, now);
    sleep(4);//change to four later;
    now = time(0);
    printf("Baboon %d arrived at time %ld\n", baboon, now);
    sem_post(&print);
}
  
int main(){
    srand(time(NULL));
    sem_init(&print, 1, 1);
    int direction;
    
    int fd = shm_open("shmname", O_CREAT, O_RDWR);
    ftruncate(fd, sizeof(struct my_sems));
    struct my_sems *semaphores = mmap(NULL, sizeof(struct my_sems), PROT_READ | PROT_WRITE,
    MAP_SHARED, fd, 0);
    sem_init(&semaphores->mutex, 1, 1);
    sem_init(&semaphores->db, 1, 1);

    for(int i=0;i<3;i++){
        direction = rand()%2;
        sleep(rand()%6);
        if(fork() == 0){
            if (direction){
                sem_wait(&semaphores->mutex);
                crossingE++;
                if (crossingE == 1){
                    sem_wait(&semaphores->db);
                }
                sem_post(&semaphores->mutex);
                time_t now = time(0);
                printf("Baboon %d getting on rope to go east at time %ld\n", i, now);
                sleep(1);
                cross((intptr_t)i, direction);
                sem_wait(&semaphores->mutex);
                crossingE--;
                if (crossingE == 0){
                    sem_post(&semaphores->db);
                }
                sem_post(&semaphores->mutex);
            }else{
                time_t now = time(0);
                printf("Baboon %d getting on rope to go west at time %ld\n", i, now);
                //wait(shm_ptr);
                sleep(1);
                sem_wait(&semaphores->db);
                cross((intptr_t)i, direction);
                sem_post(&semaphores->db);
            }
            exit(0);
        }
    }
    for(int i=0;i<3;i++){ // loop will run n times (n=5)
        wait(NULL);
    }
      
}