/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 */
#include <stdint.h>
#include "hash.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* 
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 * 
 * The following (rather complicated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash (const char *data,int len,uint32_t tablesize) {
  uint32_t hash = len, tmp;
  int rem;
  
  if (len <= 0 || data == NULL)
		return 0;
  rem = len & 3;
  len >>= 2;
  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }
  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }
  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash % tablesize;
}


typedef struct hnode {
	char *key;
	
	void *value;
	
	struct hnode *next;
} hnode_t_; 

typedef struct hashtable {
	hnode_t_ **data;
	uint32_t size; // the number of buckets 
} hashtable_t_; 

hashtable_t *hopen(uint32_t hsize) {
	hashtable_t_ *table = malloc(sizeof(hashtable_t_));
	
	table->size = hsize;
	table->data = (hnode_t_ **) malloc(sizeof(hnode_t_ *) * hsize);

	// if data can not be allocated, cleanup and return
	if (!table->data) {
		free(table);
		return NULL; 
	}; 

	// need to nullify pointers to the nodes
	for (uint32_t idx = 0; idx < hsize; ++idx) {
		table->data[idx] = NULL; 
	}	
	
	return (hashtable_t *) table; 
}

void hclose(hashtable_t *htp){
	if (htp == NULL) return;

	hashtable_t_ *table = (hashtable_t_ *) htp;
	hnode_t_ *next, *curr;
	
	for (uint32_t idx = 0; idx < table->size; idx++){
		curr = table->data[idx]; // get hashtable entry / linked list for given index
		while ( curr != NULL){
			next = curr->next; // loop through all list entries

			// free memory
			free(curr->key); 
			free(curr);
			curr = next;
		}
	}
	free(table->data); 
	free(table);
}

int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen){
	if (htp == NULL || ep == NULL || key == NULL || keylen <= 0 ){
		return 1;
	}

	hashtable_t_ *table = (hashtable_t_ *) htp;
	uint32_t index = SuperFastHash(key, keylen, table->size);
	hnode_t_ *node = malloc(sizeof(hnode_t_));

	if (node == NULL) return 2;

	node->key = malloc(keylen);
	if (node->key == NULL){
		free(node);
		return 3;
	}
 
	strncpy(node->key, key, keylen);
	node->value = ep;

	node->next = table->data[index]; // points new node to previous list back
	table->data[index] = node; // sets node as the new back of the list
	return 0;
}

void happly(hashtable_t *htp, void (*fn)(void* ep)) {
	hashtable_t_ *htp_ = (hashtable_t_ *) htp;
	if (htp_ == NULL) return; 

	// iterate over buckets
	for (int idx = 0; idx < htp_->size; ++idx) {
		// iterate over linked-list within a bucket
		for (hnode_t_ *curr = htp_->data[idx]; curr != NULL; curr = curr->next) {
			fn(curr->value); 
		}
	}

	return; 
}

void *hsearch(hashtable_t *htp, bool(*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen){
	if (htp == NULL || searchfn == NULL || key == NULL || keylen <= 0) return NULL;

	hashtable_t_ *table = (hashtable_t_ *) htp;
	uint32_t index = SuperFastHash(key, keylen, table->size);
	hnode_t_ *curr;
	void *found = NULL;
	bool match = false;

	for (curr = table->data[index]; curr != NULL && !match; curr=curr->next){
		match = searchfn(curr->value, (const void *)key);

		if (match) {
			found = curr->value;
		}
	}

	return found;

}

void *hremove(hashtable_t *htp,
							bool (*searchfn)(void* elementp, const void* searchkeyp),
							const char *key,
							int32_t keylen) {
	hashtable_t_ *htp_ = (hashtable_t_ *) htp;
	
	if (!htp_ || !key || keylen < 0) return NULL;

	uint32_t idx = SuperFastHash(key, keylen, htp_->size);
	void *result = NULL;

	for (hnode_t_ *curr = htp_->data[idx], *prev = NULL; curr != NULL; curr = curr->next) {
		if (searchfn(curr->value, (const void *)key)) {
			if (prev) {
				prev->next = curr->next;
			} else {
				htp_->data[idx] = curr->next;
			}

			result = (void *) curr->value;

			free(curr->key); 
			free(curr);
			break;
		}

		prev = curr; 
	}

	return result; 
}


