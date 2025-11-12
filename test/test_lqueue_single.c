/* 
 * test_lqueue_single.c --- 
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
#include <lqueue.h>

void print_elem(void *ep){
	printf("%d ", *(int *)ep);
}

bool searchfn(void *elementp, const void *keyp){
	int *ep = elementp;
	int *kp = (int *) keyp;
	return (*ep == *kp);
}

int main(void){
	printf("Running single-threaded test...\n");
	lqueue_t *lq = lqopen();

	for (int i = 0; i<5; i++){
		int *num = malloc(sizeof(int));
		*num = i;
		lqput(lq, num);
	}

	printf("Queue contents: ");
	lqapply(lq, print_elem);
	printf("\n");


	// test lqsearch
	printf("Searching for key 3...\n");
	int key = 3;
	int *result = lqsearch(lq, searchfn, &key);
	if (*result == key) {
		printf("search found element = %d\n", *result);
	}
	else{
		printf("Search: element not found \n");
		exit(EXIT_FAILURE);
	}

	// test lqremove
	key = 2;
	int *removed = lqremove(lq, searchfn, &key);
	if (*removed == key){
		printf("Remove: removed element = %d\n", *removed);
		//	free(removed);
	}
	else {
		printf("Remove: element not found\n");
		exit(EXIT_FAILURE);
	}
	printf("Queue after removal: ");
	lqapply(lq, print_elem);
	printf("\n");
			 
	
	// test lqget
	printf("Dequeueing elements: \n");
	int *val;	
	while ((val = lqget(lq)) != NULL){
		printf("Got %d\n", *val);
		free(val);
	}

	// test lqclose
	lqclose(lq);
	printf("Single-threaded test complete.\n");
	exit(EXIT_SUCCESS);
	return 0;
}
