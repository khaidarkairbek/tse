/* 
 * test_lhash_double.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 11-10-2025
 * Version: 1.0
 * 
 * Description: verify locked hash behaves correctly without concurrency
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>    // for sleep()
#include "lhash.h"

#define N 5

void print_elem(void *ep){
	printf("%d ", *(int *) ep);
}

bool searchfn(void *elementp, const void *keyp){
	int *ep = elementp;
	const char *kp = keyp;
	int keyval = atoi(kp);
	return(*ep == keyval);
}

void *producer(void *arg) {
	lhashtable_t *lh = arg;
	for (int i = 0; i < N; i++) {
		int *val = malloc(sizeof(int));
		*val = i;
		char *key = malloc(10);
		sprintf(key, "%d", i);
		printf("[Producer] Attempting to lock and put %d\n", *val);
		lhput(lh, val, key, strlen(key));  // lqput internally locks/unlocks
		printf("[Producer] Put %d successfully\n", *val);
		printf("Hashtable contents: ");
		lhapply(lh, print_elem);
		printf("\n");
		sleep(3);
		
	}
	return NULL;
}

void *consumer(void *arg) {
	lhashtable_t *lh = arg;
	for (int i = 0; i < N; i++) {
		printf("    [Consumer] Attempting to get element...\n");
		char *keybuf = malloc(10);
		sprintf(keybuf, "%d", i);
		int *val = lhsearch(lh, searchfn, keybuf, strlen(keybuf));   // internally locked
		if (val) {
			printf("    [Consumer] Found %d\n", *val);
			lhremove(lh, searchfn, keybuf, strlen(keybuf));
			printf("    [Consumer] Removed %d\n", *val);
							 
		} else {
			printf("    [Consumer] Key = %s not found\n", keybuf);
						 
		}
		printf("Hashtable contents: ");
		lhapply(lh, print_elem);
		printf("\n");
		sleep(5);
	}
	return NULL;
}

int main(void) {
	printf("Running dual-threaded hashtable lock test...\n");
	
	lhashtable_t *lh = lhopen(10);
	pthread_t prod, cons;
	
	pthread_create(&prod, NULL, producer, lh);
	pthread_create(&cons, NULL, consumer, lh);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	printf("Final hash contents:\n");
	lhapply(lh, print_elem);
	printf("\n");
				 
	// clean up
	lhclose(lh);
	printf("Dual-threaded test complete.\n");
	return 0;
}
