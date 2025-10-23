/* 
 * test_pagio1.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-21-2025
 * Version: 1.0
 * 
 * Description: Create webpage, save it with
 * pagesave(), then load saved page, and check they are the same.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pageio.h>
#include <webpage.h>

inline static void logr(const char *word, const int depth, const char *url)
{
	printf("%2d %*s%25s: %s\n", depth, 2*depth, "", word, url);
}


static bool compare_webpages(webpage_t *wp1, webpage_t *wp2)
{
	if (wp1 == NULL || wp2 == NULL) return false;

	if (strcmp(webpage_getURL(wp1), webpage_getURL(wp2)) != 0) {
		printf("URL mismatch.\n");
		return false;
	}
	if (webpage_getDepth(wp1) != webpage_getDepth(wp2)) {
		printf("Depth mismatch.\n");
		return false;
	}
	if (strcmp(webpage_getHTML(wp1), webpage_getHTML(wp2)) != 0) {
		printf("HTML mismatch.\n");
		return false;
	}
	return true;
}

int main(void) {
	char *url = "https://thayer.github.io/engs50/";
	webpage_t *wp;
	webpage_t *loaded;
	// create intiial webpage
	wp = webpage_new(url, 0, NULL);
	if (wp == NULL) {
		printf("Unable to create webpage\n");
		exit(EXIT_FAILURE);
	}
	logr("Initialized Webpage", webpage_getDepth(wp), webpage_getURL(wp));
	// Fetch HTML
	if (!webpage_fetch(wp)){
		printf("Unable to fetch webpage\n)");
		webpage_delete(wp);
		exit(EXIT_FAILURE);
	}

	printf("Successfully fetched webpage:\n");
	logr("Fetched", webpage_getDepth(wp), webpage_getURL(wp));
	if (pagesave(wp, 1, "pages") != 0){
		printf("Error: Pagesave failed\n");
		exit(EXIT_FAILURE);
	}
	
	logr("Saved", webpage_getDepth(wp), webpage_getURL(wp));

	loaded = pageload(1, "pages");

	if (loaded == NULL){
		printf("Error: Pageload failed\n");
		exit(EXIT_FAILURE);
	}

	logr("Loaded", webpage_getDepth(loaded), webpage_getURL(loaded));

	if (pagesave(loaded, 2, "pages") != 0){
		printf("Error: Pagesave on loaded page failed\n");
		exit(EXIT_FAILURE);
	}

	if (compare_webpages(wp, loaded)){
		printf("PageIO test passed! Webpages are identical!\n");
		exit(EXIT_SUCCESS);
	}
	else {
		printf("PageIO test failed. Webpages are no identical.\n");
		exit(EXIT_SUCCESS);
	}
}
