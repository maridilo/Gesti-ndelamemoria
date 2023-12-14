#include<sys/mman.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<semaphore>

#define SIZE 4096

int main() {
    char *shared_memory = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Crear semáforo
    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem == MAP_FAILED) {
        perror("mmap for semaphore");
        exit(EXIT_FAILURE);
    }

    // Inicializar el semáforo con un valor inicial de 0
    if (sem_init(sem, 1, 0) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Esperar a que el semáforo sea mayor que 0 (indicando que el padre ha terminado de escribir)
        sem_wait(sem);

        printf("Child reads: %s\n", shared_memory);

        // Liberar recursos
        sem_destroy(sem);
        munmap(sem, sizeof(sem_t));
        munmap(shared_memory, SIZE);

        exit(EXIT_SUCCESS);
    } else {
        strcpy(shared_memory, "Hello, child process!");

        // Incrementar el semáforo para indicar al hijo que puede continuar
        sem_post(sem);

        // Esperar a que el hijo termine
        wait(NULL);

        // Liberar recursos
        sem_destroy(sem);
        munmap(sem, sizeof(sem_t));
        munmap(shared_memory, SIZE);
    }

    return 0;
}

