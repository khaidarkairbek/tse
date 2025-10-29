#pragma once
/* 
 * indexio.h --- 
 * 
 * Author: Ava D. Rosenbaum, Khaidar Kairbek
 * Created: 10-28-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <hash.h>

const uint64_t HASH_TABLE_SIZE = 100;

typedef struct document {
  uint64_t id;
  uint64_t count;
} document_t;

typedef struct word_index {
  char *word;
  queue_t *docs;
} word_index_t;

/*
 * indexsave -- writes your in-memory hashtable to a file
 *  returns 0 if successfully saved to file
 *  returns -1 if error
 */
int32_t indexsave(hashtable_t *htp, char *indexnm);



/*
 * indexload -- reads file and reconstructs the hashtable in memory
 * returns NULL if unsuccessful
 * returns hashtable if successful
 */
hashtable_t *indexload(char *indexnm);
