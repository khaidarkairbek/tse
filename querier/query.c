/* 
 * query.c --- 
 * 
 * Author: Ava D. Rosenbaum, Khaidar Kairbek
 * Created: 10-30-2025
 * Version: 1.0
 * 
 * Description: Query implementation.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "hash.h"
#include "queue.h"
#include "indexio.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

static uint64_t LINE_LEN = 256;

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);

	char *normalized = malloc((len + 1) * sizeof(char));
	if (!normalized) return NULL;
	for (int i = 0; i < len; i++) {
		if (!isalpha(word[i])) {
			free(normalized);
			return NULL;
		}
		normalized[i] = tolower(word[i]);
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

static hashtable_t *urlload(const char *pagedir) {
	DIR *d = opendir(pagedir);
	if (d == NULL) {
		printf("Error: could not open page directory\n");
		exit(EXIT_FAILURE); 
	}
	
	struct dirent *dir;
	char *filename;
	char filepath[300];
	uint64_t page_id;
	char *endptr;
	FILE *fp; 
	char buffer[256];
	char *url_; 
	
	hashtable_t *htp = hopen(100);
	if (htp == NULL) {
		printf("Error: could not create hash table\n");
		exit(EXIT_FAILURE); 
	}

	while ((dir = readdir(d)) != NULL) {
		struct stat stbuf;
		filename = dir->d_name;
		sprintf(filepath, "%s/%s", pagedir, filename);
    if (stat(filepath, &stbuf) == -1) {
      printf("Unable to stat file: %s\n",filepath);
      continue;
    }

		if (S_ISDIR(stbuf.st_mode)) {
      printf("Skipping directory: %s\n", filepath);
      continue;
    }

    errno = 0;
    page_id = strtol(filename, &endptr, 10);
    if (endptr == filename || errno != 0 || page_id < 1) {
      printf("Skipping file since it is not integer: %s\n", filename);
      continue;
    }

		fp = fopen(filepath, "r");
		if (fp == NULL) {
			printf("Failed to open file %s\n", filepath);
			continue; 
		}

		if (fgets(buffer, sizeof(buffer), fp) != NULL) {
			buffer[strcspn(buffer, "\n")] = 0;
			url_ = malloc(strlen(buffer) + 1);
			strcpy(url_, buffer); 
			printf("URL: %s for doc: %s\n", url_, filename); 
			hput(htp, url_, filename, strlen(filename));
		} else {
			printf("Failed to read url for %s\n", filepath);
			continue; 
		}
	}

	closedir(d); 

	return htp; 
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

	char *index_file = "index_file";
	
	hashtable_t *index = indexload(index_file);
	if (index == NULL) {
		printf("Error: count not load index file '%s'\n", index_file);
		exit(EXIT_FAILURE);
	}

	hashtable_t *url_map = urlload("pages");
	if (url_map == NULL) {
		printf("Error: could not load url map\n");
		exit(EXIT_FAILURE);
	}

	

	while (true) {
		printf("> ");
		// get line
		if (fgets(line, sizeof(line), stdin) == NULL) {
			break; 
		}
		
		line[strcspn(line, "\n")] = '\0'; // remove trailing newline			
		// process each token in a line
		for (token = strtok(line, " "); token != NULL; token = strtok(NULL, " ")) {
			lowercase = NormalizeWord(token); 
			// ignore reserved and short words
			if (strlen(token) < 3 || strcmp(token, "and") == 0 || strcmp(token, "or") == 0) {
				continue; 
			}

			if (lowercase == NULL) {
				invalid = 1; 
				break; 
			}

			tmp = realloc(wordList, (wordCount + 1)* sizeof(char *));
      if (tmp == NULL){
				printf("realloc failed\n");
				exit(EXIT_FAILURE);
			}
      wordList = tmp;
			wordList[wordCount++] = lowercase;
		}

		if (invalid || wordCount == 0) {
			printf("[invalid query]\n"); 
		} else if (wordCount > 0){
      minCount = count_for_word(index, wordList[0]);
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
	}
	
	hclose(index);
	
	return 0;
}
