/* 
 * lqueue.c --- 
 * 
 * Author: Ava D. Rosenbaum, Khaidar Kairbek
 * Created: 11-9-2025
 * Version: 1.0
 * 
 * Description: Implementation of a locked queue
 * 
 */

#include "lqueue.h"
#include "queue.h"
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>


struct lqueue {
	queue_t *q;
	pthread_mutex_t lock;
};

// create an empty queue
lqueue_t* lqopen(void){ 
	lqueue_t *lqp = malloc (sizeof (lqueue_t)); // allocate memory
	if (!lqp) {
		return NULL;
	}
	lqp->q = qopen();
	if (lqp->q == NULL){
		free(lqp);
		return NULL;
	}
	pthread_mutex_init(&(lqp->lock), NULL);
	return lqp;
}
	

// deallocate a queue, frees everything in it
void lqclose(lqueue_t *lqp){
	if (lqp == NULL) return;
	pthread_mutex_destroy(&(lqp->lock));
	qclose(lqp->q);
	free(lqp); 
}

// put element at end of queue
int32_t lqput (lqueue_t *lqp, void *elementp){
	
	if (lqp == NULL) {
		return 1;
	}
	pthread_mutex_lock(&(lqp->lock));
	int32_t status = qput(lqp->q, elementp);
	pthread_mutex_unlock(&(lqp->lock));
	return status;
}

// get the first element from queue, remove it
void* lqget(lqueue_t *lqp){
	if (!lqp) return NULL; // invalid queue
	
	pthread_mutex_lock(&(lqp->lock));
	void *result = qget(lqp->q);
	pthread_mutex_unlock(&(lqp->lock));
	return result;
}

// apply a function to every element of the queue
void lqapply (lqueue_t *lqp, void (*fn)(void* elementp)) {

	if (lqp == NULL){
		return;
	}
	pthread_mutex_lock(&(lqp->lock));
	qapply(lqp->q, fn);
	pthread_mutex_unlock(&(lqp->lock));
}

// search a queue using a supplied boolean function
void* lqsearch (lqueue_t *lqp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp){
	if (lqp == NULL) return NULL;

	pthread_mutex_lock(&(lqp->lock));
	void *result = qsearch(lqp->q, searchfn, skeyp);
	pthread_mutex_unlock(&(lqp->lock));
	
	return result; 
}

//searches a queue using a supplied boolean function,
// removes the element and returns a pointer to it
void* lqremove(lqueue_t *lqp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp){
	if (lqp == NULL) return NULL;
	pthread_mutex_lock(&(lqp->lock));
	void *result = qremove(lqp->q, searchfn, skeyp);
	pthread_mutex_unlock(&(lqp->lock));

	return result; 
}

// concatenates elements of q2 into q1
void lqconcat(lqueue_t *lq1p, lqueue_t *lq2p){
	pthread_mutex_lock(&(lq1p->lock));
	pthread_mutex_lock(&(lq2p->lock));
	qconcat(lq1p->q, lq2p->q);
	pthread_mutex_unlock(&(lq1p->lock));
	pthread_mutex_unlock(&(lq2p->lock));
}
