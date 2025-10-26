/* 
 * indexer.c --- 
 * 
 * Author: Ava D. Rosenbaum and Khaidar Kairbek
 * Created: 10-21-2025
 * Version: 1.0
 * 
 * Description: The indexer implementation. 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pageio.h"
#include "webpage.h"
#include "hash.h"

const uint64_t HASH_TABLE_SIZE = 100; 

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);
	int i;
	char c;

	if (len < 3) return NULL; // discard short words
	
	char *normalized = malloc((len + 1) * sizeof(char));
	if (!normalized) return NULL;
	
	for (i=0; i<len; i++){
		c = word[i];

		if ( c >= 'A' && c <= 'Z'){
			normalized[i] = c + 32;
		}
		else if (c >= 'a' && c <= 'z'){
			normalized[i] = c;
		}
		else {
			free(normalized); // discard non letters
			return NULL;
		}
	}
	normalized[len] = '\0';
	return normalized;
}

typedef struct word_index {
	char *word;
	uint64_t count; 
} word_index_t;

bool searchfn(void *ep_, const void *searchkeyp_) {
	word_index_t *ep = (word_index_t *) ep_;
	char *key = (char *) searchkeyp_;
	return strncmp(ep->word, key, strlen(key)) == 0;
}

static uint64_t total_count = 0; 
void aggregate_count(void *ep_) {
	word_index_t *ep = (word_index_t *) ep_;
	total_count += ep->count; 
}

void cleanup_indices(void *ep_) {
	word_index_t *ep = (word_index_t *) ep_;
	free(ep->word); 
	free(ep); 
}

int main(int argc, char *argv[]){
	if (argc != 2){
		printf("usage: indexer <page_directory>\n");
		exit(EXIT_FAILURE);
	}
	char *pagedir = argv[1];
	webpage_t *page = pageload(1, pagedir);
	
	char *word = NULL;
	char *normalized = NULL;
	
	if (page == NULL){
		printf("Error: could not load page 1 from directory %s\n", pagedir);
		exit(EXIT_FAILURE);
	}

	hashtable_t *htp = hopen(HASH_TABLE_SIZE);
	if (htp == NULL) {
		printf("Error: could not create hash table\n");
		exit(EXIT_FAILURE); 
	}

	word_index_t *record = NULL;
	for (int pos = 0; (pos = webpage_getNextWord(page, pos, &word)) > 0; free(word)){ 
		normalized = NormalizeWord(word);
		if (normalized == NULL) {
			continue; 
		}
		
		if ((record = hsearch(htp, searchfn, normalized, strlen(normalized))) == NULL) {
			record = malloc(sizeof(word_index_t));
			if (record == NULL) {
				printf("Error: failed malloc call\n");
				exit(EXIT_FAILURE); 
			}

			record->word = normalized;
			record->count = 0; 

			hput(htp, record, normalized, strlen(normalized));
		} else {
			free(normalized); 
		}

		record->count += 1;
	}

	happly(htp, aggregate_count);
	printf("Sum of all counts is %ld \n", total_count); 
	
	happly(htp, cleanup_indices); 
	hclose(htp); 
 	webpage_delete(page);
}
