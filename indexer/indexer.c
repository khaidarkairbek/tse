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
#include "pageio.h"
#include "webpage.h"
#include "hash.h"
#include "queue.h"
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

	qapply(ep->docs, cleanup_docs); 
	qclose(ep->docs);
	free(ep->word); 
	free(ep); 
}

void validate_dir(char *pagedir) {
  struct stat path_stat;

  if (stat(pagedir, &path_stat) == 0) {
    if (S_ISDIR(path_stat.st_mode)) return;
    printf(usage);
    printf("Error: '%s' exists but is not a directory.\n", pagedir);
    exit(EXIT_FAILURE);
  }

  if (mkdir(pagedir, 0755) != 0) {
    printf(usage);
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

int main(int argc, char *argv[]){
	if (argc != 3){
		printf("usage: indexer <pagedir> <indexnm>\n");
		exit(EXIT_FAILURE);
	}
	char *pagedir = argv[1];
	validate_dir(page_dir);

	// create output file (validate indexnm)
	// find what index we need to go to
	
	
	char *endptr;
	errno = 0;
	uint64_t max_page_id = strtol(argv[2], &endptr, 10);
	if (endptr == argv[2] || errno != 0 || max_page_id < 1) {
		printf("usage: indexer <page_directory> <index>");
		printf("Invalid <index> argument\n");
		exit(EXIT_FAILURE);
	}

	hashtable_t *htp = hopen(HASH_TABLE_SIZE);
	if (htp == NULL) {
		printf("Error: could not create hash table\n");
		exit(EXIT_FAILURE);
	}

	webpage_t *page; 
	for (uint64_t page_id = 1; page_id <= max_page_id; ++page_id) {
		
		page = pageload(page_id, pagedir); 
		char *word = NULL;
		char *normalized = NULL;
	
		if (page == NULL){
			printf("Error: could not load page %ld from directory %s\n", page_id, pagedir);
			continue; 
		}

		word_index_t *record = NULL;
		document_t *doc_record = NULL; 
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
				record->docs = qopen();

				hput(htp, record, normalized, strlen(normalized));
			} else {
				free(normalized); 
			}

			if ((doc_record = qsearch(record->docs, document_searchfn, &page_id)) == NULL){
				doc_record = malloc(sizeof(document_t));
				if (doc_record == NULL) {
					printf("Error: failed malloc call\n");
					exit(EXIT_FAILURE);
				}
			
				doc_record->id = page_id;
				doc_record->count = 0;

				qput(record->docs, doc_record);
			}

			doc_record->count += 1;
		}
	}
	
	happly(htp, aggregate_count);
	printf("Sum of all counts is %ld \n", total_count); 
	
	happly(htp, cleanup_indices); 
	hclose(htp); 
 	webpage_delete(page);
}
