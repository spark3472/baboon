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
   sem_t eastWard;
   sem_t westWard;
   sem_t mutexEast;
    sem_t mutexWest;
    sem_t mutex;
};

char* stop = "wait";


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
    sem_init(&print, 0, 1);
    char* go;
    int direction;
    key_t mem_key;
    //struct my_sems semaphores;
    
    int fd = shm_open("shmname", O_CREAT, O_RDWR);
    ftruncate(fd, sizeof(struct my_sems));
    struct my_sems *semaphores = mmap(0, 4069, PROT_READ, MAP_SHARED, fd, 0);
    //semaphores->mutex
    sem_init(&semaphores->eastWard, 0, 1);
    sem_init(&semaphores->westWard, 0, 1);
    sem_init(&semaphores->mutexEast, 0, 1);
    sem_init(&semaphores->mutexWest, 0, 1);
    sem_init(&semaphores->mutex, 0, 1);
    for(int i=0;i<3;i++){
        //int shm_id = shmget(mem_key, 4*sizeof(int), IPC_CREAT | 0666);
        //int *shm_ptr = (int *) shmat(shm_id, NULL, 0);
        direction = rand()%2;
        sleep(rand()%6);
        if(fork() == 0){
            if (direction){
                sem_wait(&semaphores->mutex);
                sem_wait(&semaphores->westWard);
                sem_wait(&semaphores->mutexEast);
                crossingE++;
                if (crossingE == 1){
                    sem_wait(&semaphores->eastWard);
                }
                sem_post(&semaphores->mutexEast);
                sem_post(&semaphores->westWard);
                sem_post(&semaphores->mutex);
                //wait(shm_ptr);
                time_t now = time(0);
                printf("Baboon %d getting on rope to go east at time %ld\n", i, now);
                sleep(1);
                cross((intptr_t)i, direction);
                sem_wait(&semaphores->mutexEast);
                crossingE--;
                if (crossingE == 0){
                    sem_post(&semaphores->eastWard);
                }
                sem_post(&semaphores->mutexEast);
            }else{
                sem_wait(&semaphores->mutex);
                sem_wait(&semaphores->eastWard);
                sem_wait(&semaphores->mutexWest);
                crossingW++;
                if(crossingW == 1){
                    sem_wait(&semaphores->westWard);
                }
                sem_post(&semaphores->mutexWest);
                sem_post(&semaphores->eastWard);
                sem_post(&semaphores->mutex);
                time_t now = time(0);
                printf("Baboon %d getting on rope to go west at time %ld\n", i, now);
                //wait(shm_ptr);
                sleep(1);
                cross((intptr_t)i, direction);
                sem_wait(&semaphores->mutexWest);
                crossingW--;
                if (crossingW == 0){
                    sem_post(&semaphores->westWard);
                }
                    sem_post(&semaphores->mutexWest);
            }
            exit(0);
        }
    }
    for(int i=0;i<3;i++){ // loop will run n times (n=5)
        wait(NULL);
    }
      
}