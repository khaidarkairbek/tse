/* 
 * test_lhash_single.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 11-09-2025
 * Version: 1.0
 * 
 * Description: verify locked hash behaves correctly without concurrency
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <lhash.h>
#include <string.h>

void print_elem(void *ep){
	printf("%d ", *(int *)ep);
}

bool searchfn(void *elementp, const void *keyp){
	int *ep = elementp;
	char *kp = (char *) keyp;
	int keyval = atoi(kp);
	return (*ep == keyval);
}

int main(void){
	printf("Running single-threaded test...\n");
	lhashtable_t *lh = lhopen(10);

	for (int i = 0; i<5; i++){
		int *num = malloc(sizeof(int));
		*num = i;
		char *key = malloc(10);
		sprintf(key, "%d", i);
		lhput(lh, num, key, strlen(key));
		free(key);
	}

	printf("Hash contents: ");
	lhapply(lh, print_elem);
	printf("\n");


	// test lhsearch
	printf("Searching for key 3...\n");
	int keyval = 3;
	char *key = malloc(10);
	sprintf(key, "%d", keyval);
	int *result = lhsearch(lh, searchfn, key, strlen(key));
	if (*result == keyval) {
		printf("search found element = %d\n", *result);
	}
	else{
		printf("Search: element not found \n");
		exit(EXIT_FAILURE);
	}

	// test lhremove
	keyval = 2;
	sprintf(key, "%d", keyval);
	int *removed = lhremove(lh, searchfn, key, strlen(key));
	if (removed && *removed == keyval){
		printf("Remove: removed element = %d\n", *removed);
		//	free(removed);
	}
	else {
		printf("Remove: element not found\n");
		exit(EXIT_FAILURE);
	}
	printf("Hash after removal: ");
	lhapply(lh, print_elem);
	printf("\n");
			 
	
	// clean up
	lhapply(lh, free);
	lhclose(lh);
	printf("Single-threaded test complete.\n");
	free(key);
	exit(EXIT_SUCCESS);
	return 0;
}
