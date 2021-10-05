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

sem_t print;
sem_t rope;
int crossingE = 0;
int crossingW = 0;
sem_t eastWard;
sem_t westWard;
sem_t mutexEast;
sem_t mutexWest;
sem_t mutex;

char* stop = "wait";


void cross(int baboon){
    char* direction;
    if (baboon > 0){
        direction = "east";
    }else{
        direction = "west";
        baboon = baboon*-1;
    }
    sem_wait(&print);
    time_t now = time(0);
    printf("Baboon %d traveling %s at time %ld\n", baboon, direction, now);
    sleep(4);//change to four later;
    now = time(0);
    printf("Baboon %d arrived at time %ld\n", baboon, now);
    sem_post(&print);
}
  
int main(){
    sem_init(&print, 0, 1);
    sem_init(&rope, 0, 1);
    sem_init(&eastWard, 0, 1);
    sem_init(&westWard, 0, 1);
    sem_init(&mutexEast, 0, 1);
    sem_init(&mutexWest, 0, 1);
    sem_init(&mutex, 0, 1);
    srand(time(NULL));
    char* go;
    int direction;
    key_t mem_key;
    for(int i=0;i<3;i++){
        int shm_fd = shm_open(stop, O_RDONLY, 0666);
        void* ptr = mmap(0, 4069, PROT_READ, MAP_SHARED, shm_fd, 0);
        int shm_id = shmget(mem_key, 4*sizeof(int), IPC_CREAT | 0666);
        int *shm_ptr = (int *) shmat(shm_id, NULL, 0);
        direction = rand()%2;
        sleep(rand()%6);
        if(fork() == 0){
            if (direction){
                sem_wait(&mutex);
                sem_wait(&westWard);
                sem_wait(&mutexEast);
                crossingE++;
                if (crossingE == 1){
                    sem_wait(&eastWard);
                }
                sem_post(&mutexEast);
                sem_post(&westWard);
                sem_post(&mutex);
                wait(shm_ptr);
                time_t now = time(0);
                printf("Baboon %d getting on rope to go east at time %ld\n", i, now);
                sleep(1);
                cross((intptr_t)i);
                //sleep(4);
                //now = time(0);
                //printf("Baboon %d arrived at time %ld\n", i, now);
                crossingE--;
                if (crossingE == 0){
                    sem_post(&eastWard);
                }
                sem_post(&mutexEast);
            }else{
                sem_wait(&mutex);
                sem_wait(&eastWard);
                sem_wait(&mutexWest);
                crossingW++;
                if(crossingW == 1){
                    sem_wait(&westWard);
                }
                sem_post(&mutexWest);
                sem_post(&eastWard);
                sem_post(&mutex);
                time_t now = time(0);
                printf("Baboon %d getting on rope to go west at time %ld\n", i, now);
                wait(shm_ptr);
                sleep(1);
                cross((intptr_t)i*-1);
                //sleep(4);
                //now = time(0);
                //printf("Baboon %d arrived at time %ld\n", i, now);
                sem_wait(&mutexWest);
                crossingW--;
                if (crossingW == 0){
                    sem_post(&westWard);
                }
                    sem_post(&mutexWest);
            }
            exit(0);
        }
    }
    for(int i=0;i<3;i++){ // loop will run n times (n=5)
        wait(NULL);
    }
      
}