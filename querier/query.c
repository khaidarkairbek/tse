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
#include <stdbool.h>


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
static int count_for_word(hashtable_t *index, const char *word, const uint64_t docid){
	if (!index || !word) return 0;

	// find word_index struct in hashtable
	word_index_t *entry = hsearch(index, match_word, word, strlen(word));
	if (entry == NULL || entry->docs == NULL) return 0;

	document_t *doc = qsearch(entry->docs, match_docid, &docid);
	if (doc == NULL) return 0;

	return doc->count;
}

typedef struct doc_url {
	uint64_t docid;
	char *url; 
} doc_url_t; 

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
			
			doc_url_t *d = malloc(sizeof(doc_url_t));
			d->url = url_;
			d->docid = page_id; 
			hput(htp, d, filename, strlen(filename));
		} else {
			printf("Failed to read url for %s\n", filepath);
		}

		fclose(fp);
	}

	closedir(d);

	return htp; 
}

bool word_search_fn(void *ep, const void *searchkeyp) {
	char *element = (char *) ep;
	const char *key = (char *) searchkeyp;

	return strlen(element) == strlen(key) && strcmp(element, key) == 0; 
}

static int min_count = -1;
static uint64_t current_docid = 1; 
static hashtable_t *index_table = NULL;
void apply_word_count(void *ep) {
	char *element = (char *) ep;
	int count = count_for_word(index_table, element, current_docid);
	if (min_count == -1 || count < min_count) min_count = count;
}

void cleanup_word(void *ep) {
	free(ep); 
}

void cleanup_index(void *ep) {
	word_index_t *element = (word_index_t *) ep;
	free(element->word);
	qapply(element->docs, cleanup_word); 
	qclose(element->docs);
	free(element); 
}

void cleanup_url(void *ep) {
	doc_url_t *element = (doc_url_t *) ep;
	free(element->url);
	free(element); 
}

static hashtable_t *url_map = NULL;
static int query_max_count = -1; 
void apply_andseq_word_count(void *ep) {
	queue_t *element = (queue_t *) ep;
	qapply(element, apply_word_count);
	if (min_count > query_max_count) query_max_count = min_count; 
	min_count = -1;
}

static queue_t *final_query = NULL;
void apply_per_doc_query_rank(void *ep) {
	doc_url_t *element = (doc_url_t *) ep;
	uint64_t docid = element->docid;
	char *url = element->url;

	current_docid = docid;
	qapply(final_query, apply_andseq_word_count);
	printf("rank: %d : doc: %ld : %s\n", (query_max_count == -1 ? 0 : query_max_count), docid, url);
	query_max_count = -1;
	current_docid = 1; 
}

void cleanup_andseq(void *ep) {
	queue_t *element = (queue_t *) ep;
	qclose(element);
}

static int andseq_word_count = 0; 
void apply_print_andseq(void *ep) {
	char *element = (char *) ep;
	if (andseq_word_count != 0) {
		printf(" and "); 
	}
	printf("%s", element);
	andseq_word_count++; 
}

static int andseq_count = 0; 
void apply_print_query(void *ep) {
	queue_t *element = (queue_t *) ep;
	if (andseq_count == 0) {
		printf("("); 
	} else {
		printf(" or \n("); 
	}
	qapply(element, apply_print_andseq);
	andseq_word_count = 0; 
	printf(")");
	andseq_count++; 
}

void print_query(queue_t *query) {
	printf("[");
  qapply(query, apply_print_query);
	printf("]\n");
  andseq_count = 0;
}

queue_t *build_query(char *line) {
	bool valid = true;
	bool isprev_and = false;
	bool isprev_or = false;
	int word_count = 0; 
	// query is queue of andsequences
	// andsequence is queue of words
	queue_t *query = qopen();
	if (query == NULL) {
		printf("Error: qopen failed for query\n");
		exit(EXIT_FAILURE); 
	}

	queue_t *current_andseq = NULL;
	line[strcspn(line, "\n")] = '\0'; 
	for (char *token = strtok(line, " "); token != NULL; token = strtok(NULL, " ")) {
		char *lowercase = NormalizeWord(token);
		if (lowercase == NULL) {
			valid = false;
			break; 
		}
		
		if (strcmp(lowercase, "and") == 0) {
			if (current_andseq == NULL || isprev_and || isprev_or) {
				valid = false;
				break; 
			}

			isprev_and = true;
			isprev_or = false; 
		} else if (strcmp(lowercase, "or") == 0) {
			if (current_andseq == NULL || isprev_and || isprev_or) {
				valid = false;
				break;
			}

			qput(query, current_andseq);
			current_andseq = NULL;

			isprev_or = true;
			isprev_and = false; 
		} else if (strlen(lowercase) > 3){
			if (current_andseq == NULL) {
				current_andseq = qopen();
				if (current_andseq == NULL) {
					printf("Error: qopen failed for current_andseq");
					exit(EXIT_FAILURE);
				}
			}

			qput(current_andseq, lowercase);
			word_count++; 
			isprev_or = false;
			isprev_and = false; 
		} else {
			valid = false;
			break; 
		}

		if (isprev_or || isprev_and) {
			free(lowercase); 
		}
	}

	if (!valid || word_count == 0 || isprev_or || isprev_and) {
		if (current_andseq != NULL) {
			qapply(current_andseq, cleanup_word);
			qclose(current_andseq); 
		}

		qapply(query, cleanup_andseq);
		qclose(query);

		return NULL;
	}

	if (current_andseq != NULL) {
		qput(query, current_andseq);
	}


	return query; 
}

int main(void) {
	char line[LINE_LEN];
	char *index_file = "index_file";
	
	index_table = indexload(index_file);
	if (index_table == NULL) {
		printf("Error: count not load index file '%s'\n", index_file);
		exit(EXIT_FAILURE);
	}

	url_map = urlload("pages");
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

		queue_t *query = build_query(line);
		if (query == NULL) {
			printf("[invalid query]\n");
		} else {
			final_query = query; 
			print_query(query);
			happly(url_map, apply_per_doc_query_rank);
			final_query = NULL; 
		}

		qapply(query, cleanup_andseq);
		qclose(query); 
		min_count = -1;
	}
	
	happly(index_table, cleanup_index); 
	hclose(index_table);
	happly(url_map, cleanup_url);
	hclose(url_map); 
	
	return 0;
}
