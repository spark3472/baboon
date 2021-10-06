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
struct my_sems{
   sem_t mutex;
   sem_t wrt;
   sem_t rd;
   int crossingE;
   int crossingW;
};

void cross(int baboon, int direction){
    char* way;
    if (direction){
        way = "east";
    }else{
        way = "west";
    }
    time_t now = time(0);
    printf("Baboon %d getting on rope to go %s at time %ld\n", baboon,way, now);
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

    int fd = shm_open("open", O_CREAT|O_RDWR, S_IRWXU);
    ftruncate(fd, sizeof(struct my_sems));
    if (fd < 0){
        perror("shm_open");
    }
    struct my_sems *semaphores = mmap(NULL, sizeof(struct my_sems), PROT_READ | PROT_WRITE,
    MAP_SHARED, fd, 0);

    sem_init(&semaphores->mutex, 1, 1);
    sem_init(&semaphores->wrt, 1, 1);
    sem_init(&semaphores->rd, 1, 1);
    semaphores-> crossingW = 0;

    for(int i=0;i<100;i++){
        direction = rand()%2;
        sleep((rand()%6)+1);
        if(fork() == 0){
            if (direction){
                sem_wait(&semaphores->wrt);
                sem_wait(&semaphores->mutex);
                crossingE++;
                sem_post(&semaphores->mutex);
                if (crossingE == 1){
                    sem_wait(&semaphores->rd);
                }
                sem_post(&semaphores->wrt);
                cross(i, direction);
                sem_wait(&semaphores->mutex);
                crossingE--;
                sem_post(&semaphores->mutex);
                if (crossingE == 0){
                    sem_post(&semaphores->rd);
                }
            }else{
                sem_wait(&semaphores->wrt);
                sem_wait(&semaphores->mutex);
                cross(i, direction);
                sem_post(&semaphores->rd);
                sem_post(&semaphores->wrt);
            }
            exit(0);
        }
    }
    shm_unlink("open");
      
}