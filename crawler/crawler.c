
/* * crawler.c --- 
 * 
 * Author: Khaidar Kairbek
 * Created: 10-12-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "webpage.h"

int main(void) {

	char *url = "https://thayer.github.io/engs50/";
	const int depth = 0;
	int depth_test;
	webpage_t *wp = webpage_new(url, depth, NULL);

	if (wp == NULL) {
		printf("Unable to create webpage\n");
		exit(EXIT_FAILURE);
	}
		
	depth_test = webpage_getDepth(wp);
	
	printf("Hello world!\n");
	printf("Depth: %d", depth_test);

	if (!webpage_fetch(wp)){
		printf("Unable to fetch webpage\n)");
		webpage_delete(wp);
		exit(EXIT_FAILURE);
	}

	printf("Successfully fetched webpage:\n");
	printf("URL:   %s\n", webpage_getURL(wp));
	printf("Depth: %d\n", webpage_getDepth(wp));
	printf("HTML length: %d bytes\n", webpage_getHTMLlen(wp));

	
	
	
	return 0; 
}
