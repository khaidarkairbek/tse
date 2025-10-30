
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
#include <errno.h>
#include "webpage.h"
#include "queue.h"
#include "hash.h"
#include "pageio.h"

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

static char *pagedir;

// parses HTML for URLs and queues them
int parse_html_urls(queue_t *qp_, webpage_t *wp_) {
	if (wp_ == NULL) return 1;
	if (qp_ == NULL) return 2;
	
	webpage_t *wp = wp_;	
	int depth = webpage_getDepth(wp);

	char *url;
	int pos = 1; 

	while (( pos = webpage_getNextURL(wp_, pos, &url)) >= 0) {
		wp = webpage_new(url, depth+1, NULL);
 
		if (wp == NULL) continue;
		
		if (IsInternalURL(url)){
			logr("Found Internal", webpage_getDepth(wp), url);
			qput(qp_, wp);
		} else {
			logr("Ignore External", webpage_getDepth(wp), url);
			webpage_delete(wp); 
		}

		free(url); 
	}

	return 0; 
}

int crawl_from_seed(char *seed_url_, int max_depth_, queue_t *qp_, hashtable_t *htp_) {
	webpage_t *base_wp = webpage_new(seed_url_, 0, NULL);
  if (base_wp == NULL) {
    printf("Unable to create base webpage\n");
    return 1; 
  }

  // Fetch HTML
  if (!webpage_fetch(base_wp)) {
    printf("Unable to fetch initial webpage\n");
    qput(qp_, base_wp); 
    return 2; 
  }

	queue_t *base_webpage_qp = qopen();
	qput(base_webpage_qp, base_wp);

	int index = 0; 
	for(webpage_t *wp = (webpage_t *) qget(base_webpage_qp); wp != NULL; wp = (webpage_t *) qget(base_webpage_qp)) {
		char *url = webpage_getURL(wp);
		if (!NormalizeURL(url)) {
			logr("Failed to normalize url", webpage_getDepth(wp), webpage_getURL(wp));
			continue; 
		}
		if (hsearch(htp_, searchfn, url, strlen(url)) != NULL) {
			logr("Ignore repeat", webpage_getDepth(wp), webpage_getURL(wp));
			continue; 
		}; 

		// fetch html
		if (index != 0 && !webpage_fetch(wp)) {
			printf("Unable to fetch webpage for %s, skipping it\n", webpage_getURL(wp));
			webpage_delete(wp);
 			continue; 
		}

		logr("Fetched", webpage_getDepth(wp), webpage_getURL(wp)); 

		// parse html for urls that will be a depth lower
		if (webpage_getDepth(wp) != max_depth_ && parse_html_urls(base_webpage_qp, wp) > 0) {
			printf("Failed to parse HTML for %s, skipping it\n", webpage_getURL(wp));
			qput(qp_, wp);
			hput(htp_, wp, url, strlen(url)); 			
			++index; 
 			continue; 
		}

		qput(qp_, wp);
		hput(htp_, wp, url, strlen(url)); 		
		++index; 
	}

	qclose(base_webpage_qp);

	return index; 
}

const char usage[] = "usage: crawler <seedurl> <pagedir> <maxdepth>\n";

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

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf(usage);
		exit(EXIT_FAILURE); 
	}

	char *seedurl = argv[1];
	if (!NormalizeURL(seedurl)) {
		printf(usage); 
		printf("Invalid <seedurl> argument\n");
		exit(EXIT_FAILURE); 
	}

	pagedir = argv[2];
	validate_dir(pagedir); 


	char *endptr;
	errno = 0; 
	int max_depth = strtol(argv[3], &endptr, 10);
	if (endptr == argv[3] || errno != 0 || max_depth < 0) {
		printf(usage);
		printf("Invalid <max_depth> argument\n");
		exit(EXIT_FAILURE); 
	}

	queue_t *webpage_qp = qopen();
  if (webpage_qp == NULL) {
    printf("Failed to create webpage queue\n");
    exit(EXIT_FAILURE);
  }

  hashtable_t *webpage_htp = hopen(HASH_LENGTH);
  if (webpage_htp == NULL) {
    printf("Failed to create webpage hash table \n");
    exit(EXIT_FAILURE);
  }
	
	int count = crawl_from_seed(seedurl, max_depth, webpage_qp, webpage_htp);
	printf("Crawled %d URLs\n", count);

	//qapply(webpage_qp, pagesave);
	webpage_t *page;
	int id = 1;
	while (( page = qget(webpage_qp)) != NULL){
		if (pagesave(page, id++, pagedir) != 0){
			printf("Failed to save webpage\n");
		}
		webpage_delete(page);
	}
	
	printf("Crawler Complete!\n");
	// Cleanup
	//	qapply(webpage_qp, webpage_delete); 
	qclose(webpage_qp);
	hclose(webpage_htp);
	
	exit(EXIT_SUCCESS);
}
