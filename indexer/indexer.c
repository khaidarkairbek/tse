/* 
 * indexer.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-21-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pageio.h>
#include <webpage.h>
#include <hash.h>
#include <queue.h>

#define HASH_LENGTH 100

typedef struct wordcount {
	char *word;
	int count;
} wordcount_t;

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);
	int i;
	char c;

	if (len < 3) return NULL; // discard short words
	
	char *normalized = malloc(len + 1);
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

void wordcount_delete(void *item){
	if (item != NULL){
		wordcount_t *wc = (wordcount_t *) item;
		free(wc->word);
		free(wc);
	}
}

// search function
bool searchfn (void *elementp, const void *keyp){
	wordcount_t *wc = (wordcount_t *) elementp;
	const char *key = (const char *) keyp;
	return (strcmp(wc-> word, key) == 0);
}

static int total_sum = 0;

// apply function to sum total counts
void sumcounts (void *elementp){
	wordcount_t *wc = (wordcount_t *) elementp;
	total_sum += wc->count;
}

int main(int argc, char *argv[]){
	if (argc != 2){
		printf("usage: indexer <page_directory>\n");
		exit(EXIT_FAILURE);
	}
	char *pagedir = argv[1];
	webpage_t *page = pageload(1, pagedir);
	int pos = 0;
	char *word = NULL;
	char *normalized;
	hashtable_t *index;
	wordcount_t *existing;
	wordcount_t *wc;
	int total_wc = 0;
	
	if (page == NULL){
		printf("Error: could not load page 1 from directory %s\n", pagedir);
		exit(EXIT_FAILURE);
	}

	index = hopen(HASH_LENGTH);

	if (index == NULL){
		printf("Error: could not creat hashtable.\n");
		webpage_delete(page);
		exit(EXIT_FAILURE);
	}
	// extract words
	/*
	printf("Original words:\n");
	while ((pos = webpage_getNextWord(page, pos, &word)) > 0){
		printf("%s\n", word);
		free(word);
	}
	*/
	
  pos = 0;
	
	// normalized words
	while ((pos = webpage_getNextWord(page, pos, &word)) > 0){
		normalized = NormalizeWord(word);
		free(word);
		if (normalized != NULL) {
			total_wc++;
			existing = hsearch(index, searchfn, normalized, strlen(normalized));

			if (existing != NULL){
				existing->count++;
				printf("Repeat: %s\n", normalized);
				free(normalized);
 			}
			else {
				wc = malloc(sizeof(wordcount_t));
				printf("%s\n", normalized);
				wc->word = normalized;
				wc->count = 1;
				hput(index, wc, wc->word, strlen(wc->word));
			}
		}
	}

	happly(index, sumcounts);

	printf("Total normalized words: %d\n", total_wc);
	printf("Sum of counts in hash table: %d\n", total_sum);

	happly(index, wordcount_delete);
	hclose(index);
	webpage_delete(page);
					 
}
