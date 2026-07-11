#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *hello_thread(void *arg)
{
    int thread_num = *(int *)arg;
    printf("Thread %d: Ola, mundo!\n", thread_num);
    printf("Thread %d: PID do processo: %d\n", thread_num, getpid());
    printf("Thread %d: TID da thread: %lu\n", thread_num, pthread_self());
    return NULL;
}

int main()
{
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;

    printf("Processo principal PID: %d\n", getpid());
    printf("Thread principal TID: %lu\n", pthread_self());

    pthread_create(&thread1, NULL, hello_thread, &id1);
    pthread_create(&thread2, NULL, hello_thread, &id2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Todas as threads finalizaram\n");
    return 0;
}