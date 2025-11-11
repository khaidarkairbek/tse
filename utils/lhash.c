/* 
 * lhash.c -- implements a generic locked hash table as an indexed set of queues.
 *
 */
#include <stdint.h>
#include "lhash.h"
#include "hash.h"
#include "queue.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


struct lhashtable {
	hashtable_t *h;
	pthread_mutex_t lock;
};

lhashtable_t *lhopen(uint32_t hsize) {
	lhashtable_t *table = malloc(sizeof(lhashtable_t));
	if (!table) return NULL;
	
	table->h = hopen(hsize);
	if (!table->h) {
		free(table);
		return NULL;
	}

	pthread_mutex_init(&table->lock, NULL);
	
	return table; 
}

void lhclose(lhashtable_t *htp){
	if (htp == NULL) return;
	pthread_mutex_lock(&htp->lock);
	hclose(htp->h);
	pthread_mutex_unlock(&htp->lock);
	pthread_mutex_destroy(&htp->lock);
	free(htp);
}

int32_t lhput(lhashtable_t *htp, void *ep, const char *key, int keylen){

	pthread_mutex_lock(&htp->lock);
	int32_t result = hput(htp->h, ep, key, keylen);
	pthread_mutex_unlock(&htp->lock);

	return result;
}

void lhapply(lhashtable_t *htp, void (*fn)(void* ep)) {
	pthread_mutex_lock(&htp->lock);
	happly(htp->h, fn);
	pthread_mutex_unlock(&htp->lock);
}

void *lhsearch(lhashtable_t *htp, bool(*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen){
	pthread_mutex_lock(&htp->lock);
	void *result = hsearch(htp->h, searchfn, key, keylen);
	pthread_mutex_unlock(&htp->lock);

	return result;

}

void *lhremove(lhashtable_t *htp,
							bool (*searchfn)(void* elementp, const void* searchkeyp),
							const char *key,
							int32_t keylen) {
	pthread_mutex_lock(&htp->lock);
	int32_t *result = hremove(htp->h, searchfn, key, keylen);
	pthread_mutex_unlock(&htp->lock);

	return result;
}


