//varianta LINUX
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_VALUE 1000
#define SHARED_MEM_NAME "/shared_counter"
#define SEMAPHORE_NAME "/counter_semaphore"

int main() {
    srand(time(NULL) ^ getpid());

    // Semafor POSIX inter-proces
    sem_t *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    //  Memorie partajata
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Setam dimensiunea memoriei
    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // Mapam memoria
    int *shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                       MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Initializare valoare doar daca e 0
    if (*shared == 0) *shared = 0;

    //Loop principal
    while (1) {
        sem_wait(sem); // intrare sectiune critica

        int val = *shared;

        if (val >= MAX_VALUE) {
            sem_post(sem);
            break;
        }

        printf("[PID %d] citeste: %d\n", getpid(), val);

        // Aruncam banul: cat timp cade 1, incrementam
        while ((rand() % 2 == 1) && val < MAX_VALUE) {
            val++;
            *shared = val;
            printf("[PID %d] scrie: %d\n", getpid(), val);
            usleep(20000); // 20ms
        }

        sem_post(sem); //iesire sectiune critica
        usleep(30000);  // pauza scurtă
    }

    // Cleaning
    munmap(shared, sizeof(int));
    close(shm_fd);
    sem_close(sem);

    return 0;
}
