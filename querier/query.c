/* 
 * query.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-30-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <hash.h>
#include <queue.h>
#include <indexio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

static uint64_t LINE_LEN = 256;
#define INDEX_FILE "indexfile"

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);

	char *normalized = malloc((len + 1) * sizeof(char));
	if (!normalized) return NULL;
	for (int i = 0; i < len; i++) {
		if (!isalpha((unsigned char)word[i])) {
			free(normalized);
			return NULL;
		}
		normalized[i] = tolower((unsigned char)word[i]);
	}
	normalized[len] = '\0';
	return normalized;
}

// hsearch helper for document_t
static bool match_docid(void *elementp, const void *keyp){
	document_t *doc = (document_t *) elementp;
	uint64_t target = *(uint64_t *) keyp;
	return(doc->id == target);
}

// hsearch helper for word_index_t
static bool match_word(void *elementp, const void *keyp){
	word_index_t *w = (word_index_t *) elementp;
	return strcmp(w->word, (const char *)keyp) == 0;
}

// get count for a given word in document ID 1
static int count_for_word(hashtable_t *index, const char *word){
	if (!index || !word) return 0;

	// find word_index struct in hashtable
	word_index_t *entry = hsearch(index, match_word, word, strlen(word));
	if (entry == NULL || entry->docs == NULL) return 0;

	uint64_t docid = 1;
	document_t *doc = qsearch(entry->docs, match_docid, &docid);
	if (doc == NULL) return 0;

	return doc->count;
}

int main(void) {
	char line[LINE_LEN];
	char **wordList = NULL; // growing list of words
	char **tmp;
	char *token;
	char *lowercase;
	int wordCount = 0;
	int invalid = 0;
	int count;
	int minCount;

	hashtable_t *index = indexload(INDEX_FILE);
	if (index == NULL) {
		printf("Error: count not load index file '%s'\n", INDEX_FILE);
		exit(EXIT_FAILURE);
	}
	
	printf("> ");
	while (fgets(line, sizeof(line), stdin) != NULL){
		line[strcspn(line, "\n")] = '\0'; // remove trailing newline

		if (line[0] == '\0') {
			printf("> ");
			continue;
		}

		token = strtok(line, " ");
		
		while (token != NULL) {
			if (strlen(token) < 3 || strcmp(token, "and")  == 0 || strcmp(token, "or") == 0) {
				token = strtok(NULL, " ");
				continue;
			}
			
			lowercase =  NormalizeWord(token);
			if (lowercase == NULL){
				invalid = 1;
			}
			else if (!invalid) {
				tmp = realloc(wordList, (wordCount + 1)* sizeof(char *));
				if (tmp == NULL){
					printf("realloc failed\n");
					exit(EXIT_FAILURE);
				}
				wordList = tmp;
				wordList[wordCount++] = lowercase;
			}
			else {
				free(lowercase);
			}

			token = strtok(NULL, " ");
		}
		
		if (invalid || wordCount == 0){
			printf("[invalid query]\n");
		}
		else if (wordCount > 0){
			minCount = count_for_word(index, wordList[i]);
			for (int i = 0; i < wordCount; i++){
				count = count_for_word(index, wordList[i]);
				printf("%s:%d ", wordList[i], count);
				if (count < minCount) minCount = count;

				free(wordList[i]);
			}
			printf("-- %d\n", minCount);			 
		}
		free(wordList);
		wordList = NULL;
		wordCount = 0;
		invalid = 0;
		printf("> ");
	}
	
	hclose(index);
	
	return 0;
}
