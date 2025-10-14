
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

int main(void) {
	char *url = "https://thayer.github.io/engs50/";
	const int depth = 0;
	webpage_t *wp, *new_wp;
	queue_t *qp;
	hashtable_t *htp;
	int pos = 0;
	char *result;
	char *url_tmp;

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

	printf("Step 4:\n");
	
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
