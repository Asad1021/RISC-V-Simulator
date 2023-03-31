#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *function1(void *arg) {
    printf("Function 1 is running\n");
    pthread_exit(NULL);
}

void *function2(void *arg) {
    printf("Function 2 is running\n");
    pthread_exit(NULL);
}

void *function3(void *arg) {
    printf("Function 3 is running\n");
    pthread_exit(NULL);
}

void *function4(void *arg) {
    printf("Function 4 is running\n");
    pthread_exit(NULL);
}

void *function5(void *arg) {
    printf("Function 5 is running\n");
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[5];

    // Create threads for each function
    pthread_create(&threads[0], NULL, function1, NULL);
    pthread_create(&threads[1], NULL, function2, NULL);
    pthread_create(&threads[2], NULL, function3, NULL);
    pthread_create(&threads[3], NULL, function4, NULL);
    pthread_create(&threads[4], NULL, function5, NULL);

    // Wait for threads to finish
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
