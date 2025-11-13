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
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "pageio.h"
#include "webpage.h"
#include "lhash.h"
#include "lqueue.h"
#include "indexio.h"

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

bool document_searchfn(void *ep_, const void *keyp_) {
	document_t *ep = (document_t *) ep_;
	uint64_t *keyp = (uint64_t *) keyp_; 
	return ep->id == *keyp; 
}

void cleanup_docs(void *ep_) {
	free(ep_); 
}

void cleanup_pageid(void *ep_) {
	free(ep_); 
}

bool searchfn(void *ep_, const void *searchkeyp_) {
	word_index_t *ep = (word_index_t *) ep_;
	char *key = (char *) searchkeyp_;
	return strncmp(ep->word, key, strlen(key)) == 0;
}

static uint64_t total_count_per_word = 0;
void aggregate_count_per_word(void *ep_) {
	document_t *ep = (document_t *) ep_;
	total_count_per_word += ep->count; 
}

static uint64_t total_count = 0; 
void aggregate_count(void *ep_) {
	word_index_t *ep = (word_index_t *) ep_;
	queue_t *qp = ep->docs;
	total_count_per_word = 0; 
	qapply(qp, aggregate_count_per_word); 
	total_count += total_count_per_word; 
}

void cleanup_indices(void *ep_) {
	word_index_t *ep = (word_index_t *) ep_;

	lqapply(ep->docs, cleanup_docs); 
	lqclose(ep->docs);
	free(ep->word); 
	free(ep); 
}

static char *usage = "usage: indexer <pagedir> <indexnm> <nthreads>\n"; 

void validate_dir(char *pagedir) {
  struct stat path_stat;

  if (stat(pagedir, &path_stat) == 0) {
    if (S_ISDIR(path_stat.st_mode)) return;
    printf("%s", usage);
    printf("Error: '%s' exists but is not a directory.\n", pagedir);
    exit(EXIT_FAILURE);
  }

  if (mkdir(pagedir, 0755) != 0) {
    printf("%s", usage);
    switch(errno) {
    case ENOENT:
      printf("Error: parent directory of '%s' does not exist.\n", pagedir);
      break;
    case EACCES:
      printf("Error: permission denied when creating '%s'.\n", pagedir);
      break;
		case ENAMETOOLONG:
      printf("Error: path name too long: '%s'.\n", pagedir);
      break;
    default:
      printf("Error: invalid path '%s'.\n", pagedir);
      break;
    }
    exit(EXIT_FAILURE);
  }
}

static uint64_t INDEXER_HASH_TABLE_SIZE = 100;

lqueue_t *filename_lqp;
lhashtable_t  *htp;
char *pagedir; 

void *tfunc(void *argp) {
	uint64_t *page_idp;
	uint64_t page_id; 

	printf("in tfunc\n"); 
	// Do something
	webpage_t *page;
	while ((page_idp = (uint64_t *) lqget(filename_lqp)) != NULL) {
		char *word = NULL;
		char *normalized = NULL;
		page_id = *page_idp;
		free(page_idp); 

		printf("Processing %ld page\n", page_id);

		page = pageload(page_id, pagedir);
		if (page == NULL) {
			printf("Error: could not load page %ld from directory %s\n", page_id, pagedir);
			continue; 
		}

		word_index_t *record = NULL;
		document_t *doc_record = NULL;
		for (int pos = 0; (pos = webpage_getNextWord(page, pos, &word)) > 0; free(word)) {
			normalized = NormalizeWord(word);
			if (normalized == NULL) {
				continue; 
			}

			if ((record = lhsearch(htp, searchfn, normalized, strlen(normalized))) == NULL) {
				record = malloc(sizeof(word_index_t));
				if (record == NULL) {
					printf("Error: failed malloc call\n");
					exit(EXIT_FAILURE); 
				}

				record->word = normalized;
				record->docs = lqopen();

				lhput(htp, record, normalized, strlen(normalized)); 
			} else {
				free(normalized); 
			}

			if ((doc_record = lqsearch(record->docs, document_searchfn, &page_id)) == NULL) {
				doc_record = malloc(sizeof(document_t));
				if (doc_record == NULL) {
					printf("Error: failed malloc call\n");
					exit(EXIT_FAILURE);
				}

				doc_record->id = page_id;
				doc_record->count = 0;

				lqput(record->docs, doc_record); 
			}

			doc_record->count += 1;
		}

		webpage_delete(page);
	}

	return NULL; 
}

int main(int argc, char *argv[]){
	if (argc != 4){
		printf("%s", usage);
		exit(EXIT_FAILURE);
	}
	pagedir = argv[1];
	validate_dir(pagedir);

	// create output file (validate indexnm)
	// find what index we need to go to
	char *indexnm = argv[2];

	char *endptr; 
	uint64_t thread_count;
	errno = 0;
  thread_count = strtol(argv[3], &endptr, 10);
	if (endptr == argv[3] || errno != 0 || thread_count < 1 || thread_count > 32) {
		printf("%s", usage);
		printf("Invalid <nthreads> argument. Must be 1-32\n");
		exit(EXIT_FAILURE); 
  }	

	DIR *d = opendir(pagedir);
	if (d == NULL) {
		printf("%s", usage); 
		printf("Error: could not open page directory\n");
		exit(EXIT_FAILURE); 
	}

	pthread_t *threads = (pthread_t *) malloc(thread_count * sizeof(pthread_t));
	if (threads == NULL) {
		printf("Error: Failed to malloc threads\n");
		exit(EXIT_FAILURE);
	}

	struct dirent *dir;
  char *filename;
  char filepath[300];
  uint64_t page_id;
	uint64_t *page_idp; 

  htp = lhopen(INDEXER_HASH_TABLE_SIZE);
  if (htp == NULL) {
    printf("Error: could not create hash table\n");
    exit(EXIT_FAILURE);
  }

	filename_lqp = lqopen();
	if (filename_lqp == NULL) {
		printf("Error: failed to open lqueue\n");
		exit(EXIT_FAILURE); 
	}

	while ((dir = readdir(d)) != NULL) {
		struct stat stbuf;
		filename = dir->d_name;
    sprintf(filepath, "%s/%s", pagedir,dir->d_name);
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

		page_idp = (uint64_t *) malloc(sizeof(uint64_t));
		if (page_idp == NULL) {
			printf("Error: failed to malloc page_idp\n");
			exit(EXIT_FAILURE); 
		}

		*page_idp = page_id; 
		if (lqput(filename_lqp, page_idp) != 0){
			printf("Error: failed to put into lqueue\n");
			exit(EXIT_FAILURE);
		};
	}

	for (int i = 0; i < thread_count; ++i) {
		if (pthread_create(&threads[i], NULL, tfunc, NULL) != 0) {
      printf("Error: Failed to create a thread\n");
      exit(EXIT_FAILURE);
    }
  }

	for (int i = 0; i < thread_count; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			printf("Error: Failed to join a thread\n"); 
			exit(EXIT_FAILURE); 
		}
	}

	closedir(d); 
	if (indexsave(htp, indexnm) != 0) {
		printf("Failed saving indexes\n");
		exit(EXIT_FAILURE); 
	}; 

	free(threads); 
	lqapply(filename_lqp, cleanup_pageid);
	lqclose(filename_lqp); 
	lhapply(htp, cleanup_indices); 
	lhclose(htp); 
}
