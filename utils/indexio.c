/* 
 * indexio.c --- 
 * 
 * Author: Khaidar Kairbek, Ava D. Rosenbaum
 * Created: 10-29-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lhash.h"
#include "hash.h"
#include "lqueue.h"
#include "indexio.h"

static FILE *index_fp;
static uint64_t INDEXER_HASH_TABLE_SIZE = 100;

void print_doc(void *ep_doc){
	document_t *doc = (document_t *) ep_doc;
	fprintf(index_fp, " %lu %lu", doc->id, doc->count);
}

void save_word(void *ep) {
	word_index_t *record = (word_index_t *) ep;
	fprintf(index_fp, "%s", record->word);

	lqapply(record->docs, print_doc);
	fprintf(index_fp, "\n");
}

void lsave_word(void *ep) {
	word_index_t *record = (word_index_t *) ep;
	fprintf(index_fp, "%s", record->word);
	lqapply(record->docs, print_doc);
	fprintf(index_fp, "\n"); 
}

int32_t lindexsave(lhashtable_t *htp, char *indexnm) {
	if (htp == NULL || indexnm == NULL) return -1;

	index_fp = fopen(indexnm, "w");

	if (index_fp == NULL) {
		printf("Error: fopen failed\n");
		return -1; 
	}

	lhapply(htp, lsave_word);
	fclose(index_fp);
	return 0; 
}

int32_t indexsave(hashtable_t *htp, char *indexnm){
	if (htp == NULL || indexnm == NULL) return -1;

	index_fp = fopen(indexnm, "w");

	if (index_fp == NULL){
		printf("Error: fopen failed\n");
		return -1;
	}

	happly(htp, save_word);
	fclose(index_fp);
	return 0;
}


bool word_match(void *ep, const void *keyp){
	word_index_t *record = (word_index_t *) ep;
	char *word = (char *) keyp;
	return strcmp(record->word, word) == 0;
}

hashtable_t *indexload(char *indexnm){
	if ( indexnm == NULL ) return NULL;

	FILE *fp = fopen(indexnm, "r");
	if (fp == NULL) {
		printf("Error: fopen failed\n");
		return NULL;
	}

	hashtable_t *htp = hopen(INDEXER_HASH_TABLE_SIZE);
	if (htp == NULL) {
		fclose(fp);
		return NULL;
	}

	char word[128]; // stack buffer for reading word strings
	uint64_t docID, count;
	int n;

	int word_read = fscanf(fp, "%127s", word);
	size_t len;
	
	while (word_read == 1) {
		word_index_t *record = malloc(sizeof(word_index_t));
		if (record == NULL) {
			fclose(fp);
			return htp;
		}

		len = strlen(word);
		record->word = malloc(len+1);
		if (record->word == NULL){
			free(record);
			fclose(fp);
			return htp;
		}
		strcpy(record->word, word);
		
		record->docs = qopen();
		if (record->docs == NULL){
			free(record->word);
			free(record);
			fclose(fp);
			return htp;
		}
		
		n = fscanf(fp, "%lu %lu", &docID, &count);
		while (n == 2){ // scan docID count pair
			document_t *doc = malloc(sizeof(document_t));
			if (doc == NULL) {
				qclose(record->docs);
				free(record->word);
				free(record);
				fclose(fp);
				return htp;
			}

			doc->id = docID;
			doc->count = count;
			qput(record->docs, doc);

			n = fscanf(fp, "%lu %lu", &docID, &count);
		}

		hput(htp, record, record->word, strlen(record->word));

		word_read = fscanf(fp, "%127s", word); // next word
	}

	fclose(fp);
	return htp;
}
