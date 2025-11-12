/* 
 * test_lqueue_double_locked.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 11-09-2025
 * Version: 1.0
 * 
 * Description: verify locked queue behaves correctly without concurrency
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>    // for sleep()
#include "lqueue.h"

#define N 5

void print_elem(void *ep){
	
	printf("%d ", *(int *)ep);
}					 


void *producer(void *arg) {
	lqueue_t *lq = arg;
	for (int i = 0; i < N; i++) {
		int *val = malloc(sizeof(int));
		*val = i;
		printf("[Producer] Attempting to lock and put %d\n", *val);
		if (lqput(lq, val) == 0){  // lqput internally locks/unlocks
			printf("[Producer] Put %d successfully\n", *val);
		}
		else {
			printf("[Producer] Unable to put %d \n", *val);
		}
		printf("Queue contents: ");
		lqapply(lq, print_elem);
		printf("\n");

	}
	return NULL;
}

void *consumer(void *arg) {
	lqueue_t *lq = arg;
	sleep(4);
	for (int i = 0; i < N; i++) {
		printf("    [Consumer] Attempting to get element...\n");
		int *val = lqget(lq);   // internally locked
		if (val) {
			printf("    [Consumer] Got %d\n", *val);
			free(val);
		} else {
			printf("    [Consumer] Queue empty\n");
		}
		printf("Queue contents: ");
		lqapply(lq, print_elem);
		printf("\n");
	}
	return NULL;
}

int main(void) {
	printf("Running dual-threaded lock test...\n");
	
	lqueue_t *lq = lqopen();
	pthread_t prod, cons;
	
	pthread_create(&prod, NULL, producer, lq);
	pthread_create(&cons, NULL, consumer, lq);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	printf("Print Queue contents: \n");
	lqapply(lq, print_elem);
	lqclose(lq);
	printf("Dual-threaded test complete.\n");
	return 0;
}
