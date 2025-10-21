
/* * crawler.c --- 
 * 
 * Author: Khaidar Kairbek & Ava Rosenbaum
 * Created: 10-12-2025
 * Version: 1.0
 * 
 * Description: 
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

void print_webpage (void* wp){
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

int main(void) {
	char *url = "https://thayer.github.io/engs50/";
	const int depth = 0;
	webpage_t *wp, *new_wp;
	queue_t *qp;
	hashtable_t *htp;
	int pos = 0;
	char *result;
	char *url_tmp;
	int32_t save;

	printf("Step 2:\n");
	
	// create intiial webpage 
	wp = webpage_new(url, depth, NULL);
	if (wp == NULL) {
		printf("Unable to create webpage\n");
		exit(EXIT_FAILURE);
	}

	// Fetch HTML
	if (!webpage_fetch(wp)){
		printf("Unable to fetch webpage\n)");
		webpage_delete(wp);
		exit(EXIT_FAILURE);
	}

	printf("Successfully fetched webpage:\n");
	logr("Fetched", webpage_getDepth(wp), webpage_getURL(wp));

	qp = qopen(); // create queue
	if (qp == NULL){
		printf("Failed to create queue\n");
		exit(EXIT_FAILURE);
	}


	// scan HTML for URLS
	while (( pos = webpage_getNextURL(wp, pos, &result)) > 0) {
		if (IsInternalURL(result)){
			logr("Found Internal", webpage_getDepth(wp), webpage_getURL(wp));
			
			// create new webpage
			new_wp = webpage_new(result, depth+1, NULL);
			if (new_wp != NULL){
				logr("Saved", webpage_getDepth(new_wp), webpage_getURL(new_wp));
				qput(qp, new_wp); // add new webpage to queue
			}
 		}
		else {
			logr("Ignore External", webpage_getDepth(wp), webpage_getURL(wp)); // print but ignore external
		}
		free(result);
	}

	printf("Step 3:\n");
	// print all webpages in the queue
	printf("Queue Contents\n");
	qapply(qp, print_webpage);

	// Free up memory in queue, end Step 3
	qapply(qp, webpage_delete);
	qclose(qp);
	webpage_delete(wp);

	// restart for steps 4 & 5
	
	htp = hopen(HASH_LENGTH); // create hash table

	// create initial webpage
	wp = webpage_new(url, depth, NULL);
	if (wp == NULL) {
		printf("Unable to create webpage\n");
		exit(EXIT_FAILURE);
	}
	
	// Fetch HTML
	if (!webpage_fetch(wp)){
		printf("Unable to fetch webpage\n)");
		webpage_delete(wp);
		exit(EXIT_FAILURE);
  }
	printf("Successfully fetched webpage:\n");
	logr("Fetched", webpage_getDepth(wp), webpage_getURL(wp));

	printf("Step 5: Save One Page\n");
	save = pagesave(wp, 1, "pages"); // save webpage to ~/engs50/tse/pages
	if (save != 0){
		printf("Failed to save page 1\n");
	}
	else {
		printf("Successfully saved file '1' to ~./engs50/tse/pages/\n");
	}

	save = pagesave(wp, 2, "pages");
	if (save != 0) {
		printf("Failed to save page 2\n");
	}
	else {
		printf("Successfully saved file '2' to ~./engs50/tse/pages/\n");
	}
	printf("Step 4:\n");

	qp = qopen(); // create queue
	if (qp == NULL){
		printf("Failed to create queue\n");
	  exit(EXIT_FAILURE);
  }

	pos = 0;
	
	// scan HTML for URLS
	while (( pos = webpage_getNextURL(wp, pos, &result)) > 0) {
		if (IsInternalURL(result)){
			logr("Found Internal", webpage_getDepth(wp), webpage_getURL(wp));

			// create new webpage
			new_wp = webpage_new(result, depth+1, NULL);
			if (new_wp != NULL){
				url_tmp = webpage_getURL(new_wp);
				
				// check if URL exists in hash table
				if (hsearch(htp, searchfn, url_tmp, strlen(url_tmp)) == NULL){
					hput(htp, new_wp, url_tmp, strlen(url_tmp)); // add to hash table
					qput(qp, new_wp); // add new webpage to queue
					logr("Saved", webpage_getDepth(new_wp), url_tmp);
				}
				else {
					logr("Ignore Repeat", webpage_getDepth(wp), webpage_getURL(wp));
 	 			}
			}
		}
		else {
			logr("Ignore External", webpage_getDepth(wp), webpage_getURL(wp)); // print but ignore external
		}
		free(result);
	}

	printf("Queue Contents\n");
	qapply(qp, print_webpage);

	// clear memory
	qapply(qp, webpage_delete);
	qclose(qp);
 	hclose(htp);
	webpage_delete(wp);
	
	printf("Crawler Complete!\n");
	exit(EXIT_SUCCESS);
	
	
	
	return 0; 
}
