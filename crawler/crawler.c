
/* * crawler.c --- 
 * 
 * Author: Khaidar Kairbek & Ava Rosenbaum
 * Created: 10-12-2025
 * Version: 1.0
 * 
 * Description: Implementation of the crawler module of Tiny Search Engine. 
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "webpage.h"
#include "queue.h"
#include "hash.h"

#define HASH_LENGTH 20

// log one word (1-9 chars) about a given url
inline static void logr(const char *word, const int depth, const char *url)
{
	printf("%2d %*s%25s: %s\n", depth, 2*depth, "", word, url);
}

void print_webpage(void* wp){
	webpage_t *page = (webpage_t *) wp;
	logr("Queued URL", webpage_getDepth(page), webpage_getURL(page));
}

bool searchfn(void *elementp, const void *keyp) {
	webpage_t *p = (webpage_t *) elementp;
	const char *key = (const char *) keyp;
	return strcmp(webpage_getURL(p), key) == 0;
}

int32_t pagesave(webpage_t *pagep, int id, char *dirname){
	char filename[200];
	char dir_path[200];
	FILE *fp;

	sprintf(dir_path, "../../%s", dirname);
	
	if (access(dir_path, F_OK) != 0) {  // check if directory exists
		if (mkdir(dir_path, 0755) != 0) { // create directory w/ rwxr-xr-x
			printf("Mkdir Failed\n");
			return -1;
		}
	}
	
	sprintf(filename, "../../%s/%d", dirname, id); // make filename for dirname/id

	fp = fopen(filename, "w"); // open file for writing

	if (fp == NULL){
		printf("Error: cannot open file %s\n", filename);
		return -1;
	}

	// write to file
	fprintf(fp, "%s\n", webpage_getURL(pagep));
	fprintf(fp, "%d\n", webpage_getDepth(pagep));
	fprintf(fp, "%d\n", webpage_getHTMLlen(pagep));
	fprintf(fp, "%s\n", webpage_getHTML(pagep));

	fclose(fp);

	return 0;
}

// parses HTML for URLs and queues them
int parse_html_urls(queue_t *qp_, hash_table_t *htp_, webpage_t *wp_) {
	char *url;
	int pos = 0;
	webpage_t *wp = wp_;

	if (wp == NULL) return 1;
	if (qp_ == NULL) return 2;

	while (( pos = webpage_getNextURL(wp, pos, &url)) >= 0) {
		wp = webpage_new(url, depth+1, NULL);
 
		if (wp == NULL) continue;

		if (htp_ != NULL && hsearch(htp_, searchfn, url, strlen(url)) == NULL) {
			logr("Ignore Repeat", webpage_getDepth(wp), url);
			free(url); 
			continue; 
		}
		
		if (IsInternalURL(url)){
			logr("Found Internal", webpage_getDepth(wp_), url);
			hput(htp_, wp, url, strlen(url)); 
			qput(qp_, wp_);
		} else {
			logr("Ignore External", webpage_getDepth(wp_), url);
		}

		free(url); 
	}

	return 0; 
}

int main(void) {
	char *url = "https://thayer.github.io/engs50/";
	const int depth = 0;
	webpage_t *wp, *new_wp;
	queue_t *qp;
	hashtable_t *htp;
	int pos = 0;
	char *url_tmp;

	printf("Step 2:\n");

	webpage_t *wp = webpage_new(url, depth, NULL);
	if (wp == NULL) {
		printf("Unable to create initial webpage\n");
		exit(EXIT_FAILURE); 
	}

	// Fetch HTML
	if (!webpage_fetch(wp)) {
		printf("Unable to fetch initial webpage\n");
		webpage_delete(wp);
		exit(EXIT_FAILURE); 
	}

	logr("Fetched", webpage_getDepth(wp), webpage_getURL(wp));

	qp = qopen();
	if (qp == NULL) {
		printf("Failed to create queue\n");
		exit(EXIT_FAILURE);
	}

	// scan HTML for URLS
	if (parse_html_urls(qp, wp) > 0) {
		printf("Failed to parse HTML for %s\n", webpage_getURL(wp));
		exit(EXIT_FAILURE); 
	};


	
	
	printf("Crawler Complete!\n");
	exit(EXIT_SUCCESS);
	
	
	
	return 0; 
}
