#include<iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
using namespace std;
#define NUM_THREADS 5

int shared_resource = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

void *fetch(void *arg) {
    int thread_num = *(int *)arg;
    printf("Thread %d is running\n", thread_num);

    // Lock the mutex before accessing the shared resource
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    shared_resource++;
    printf("Thread %d accessed the shared resource, which is now %d\n", thread_num, shared_resource);

    // Unlock the mutex before exiting the thread
    pthread_mutex_unlock(&mutex);
    
    pthread_barrier_wait(&barrier);
    cout<<"Now barrier is ended num is: "<<thread_num;

    pthread_exit(NULL);
}
void *decode(void *arg) {
    int thread_num = *(int *)arg;
    printf("Thread %d is running\n", thread_num);

    // Lock the mutex before accessing the shared resource
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    shared_resource++;
    printf("Thread %d accessed the shared resource, which is now %d\n", thread_num, shared_resource);

    // Unlock the mutex before exiting the thread
    pthread_mutex_unlock(&mutex);
    
    pthread_barrier_wait(&barrier);
    cout<<"Now barrier is ended num is: "<<thread_num;


    pthread_exit(NULL);
}
void *execute(void *arg) {
    int thread_num = *(int *)arg;
    printf("Thread %d is running\n", thread_num);

    // Lock the mutex before accessing the shared resource
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    shared_resource++;
    printf("Thread %d accessed the shared resource, which is now %d\n", thread_num, shared_resource);

    // Unlock the mutex before exiting the thread
    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier);
    cout<<"Now barrier is ended num is: "<<thread_num;

    pthread_exit(NULL);
}
void *memory(void *arg) {
    int thread_num = *(int *)arg;
    printf("Thread %d is running\n", thread_num);

    // Lock the mutex before accessing the shared resource
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    shared_resource++;
    printf("Thread %d accessed the shared resource, which is now %d\n", thread_num, shared_resource);

    // Unlock the mutex before exiting the thread
    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier);
    cout<<"Now barrier is ended num is: "<<thread_num;

    pthread_exit(NULL);
}
void *writeback(void *arg) {
    int thread_num = *(int *)arg;
    printf("Thread %d is running\n", thread_num);

    // Lock the mutex before accessing the shared resource
    pthread_mutex_lock(&mutex);

    // Access the shared resource
    shared_resource++;
    printf("Thread %d accessed the shared resource, which is now %d\n", thread_num, shared_resource);

    // Unlock the mutex before exiting the thread
    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier);
    cout<<"Now barrier is ended num is: "<<thread_num;

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_nums[NUM_THREADS] = {1,2,3,4,5};
    pthread_barrier_init(&barrier,NULL,NUM_THREADS);

    // Create threads for each function
    // for (int i = 0; i < NUM_THREADS; i++) {
    //     thread_nums[i] = i + 1;
    //     pthread_create(&threads[i], NULL, function, &thread_nums[i]);
    // }

    pthread_create(&threads[0], NULL, fetch, &thread_nums[0]);
    pthread_create(&threads[1], NULL, decode, &thread_nums[1]);
    pthread_create(&threads[2], NULL, execute, &thread_nums[2]);
    pthread_create(&threads[3], NULL, memory, &thread_nums[3]);
    pthread_create(&threads[4], NULL, writeback, &thread_nums[4]);

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_barrier_destroy(&barrier);

    return 0;
}


// #include<iostream>
// #include<stdio.h>
// #include <stdlib.h>
// #include <pthread.h>

// using namespace std;

// void *function1(void *arg) {
//     cout<<"Function 1 is running\n";
//     pthread_exit(NULL);
// }

// void *function2(void *arg) {
//     cout<<"Function 2 is running\n";
//     pthread_exit(NULL);
// }

// void *function3(void *arg) {
//     cout<<"Function 3 is running\n";
//     pthread_exit(NULL);
// }

// void *function4(void *arg) {
//     cout<<"Function 4 is running\n";
//     pthread_exit(NULL);
// }

// void *function5(void *arg) {
//     cout<<"Function 5 is running\n";
//     pthread_exit(NULL);
// }

// int main() {
//     pthread_t threads[5];

//     // Create threads for each function
//     pthread_create(&threads[0], NULL, function1, NULL);
//     pthread_create(&threads[1], NULL, function2, NULL);
//     pthread_create(&threads[2], NULL, function3, NULL);
//     pthread_create(&threads[3], NULL, function4, NULL);
//     pthread_create(&threads[4], NULL, function5, NULL);

//     // Wait for threads to finish
//     for (int i = 0; i < 5; i++) {
//         pthread_join(threads[i], NULL);
//     }

//     return 0;
// }
