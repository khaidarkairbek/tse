/* 
 * pageio.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-21-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdlib.h>
#include <pageio.h>
#include <webpage.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


int32_t pagesave(webpage_t *pagep, int id, char *dirnm){
	char filename[200];

 	FILE *fp;

	if (access(dirnm, F_OK) != 0) {  // check if directory exists
		if (mkdir(dirnm, 0755) != 0) { // create directory w/ rwxr-xr-x
			printf("Mkdir Failed\n");
			return(-1);
		}
	}

	sprintf(filename, "%s/%d", dirnm, id); // make filename for dirname/id

	fp = fopen(filename, "w"); // open file for writing

	if (fp == NULL){
		printf("Error: cannot open file %s\n", filename);
		return(-2);
	}

	// write to file
	fprintf(fp, "%s\n", webpage_getURL(pagep));
	fprintf(fp, "%d\n", webpage_getDepth(pagep));
	fprintf(fp, "%d\n", webpage_getHTMLlen(pagep));
	fprintf(fp, "%s\n", webpage_getHTML(pagep));

	fclose(fp);

	printf("Saved '%s' into %d file\n", webpage_getURL(pagep), id);
	id++;
	return(0);
}

webpage_t *pageload(int id, char *dirnm){
	char filename[200];
	FILE *fp;
	char url[200];
	int depth;
	int html_len;
	char *html;
	int c;
	int i = 0;

	sprintf(filename, "%s/%d", dirnm, id);
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Error: cannot open file %s for reading\n", filename);
		return NULL;
	}

	// read URL
	if (fscanf(fp, "%255s\n", url) != 1) {
		printf("Error: failed to read URL from %s\n", filename);
		fclose(fp);
		return NULL;
	}

	// read depth
	if (fscanf(fp, "%d\n", &depth) != 1){
		printf("Error: failed to read depth from %s\n", filename);
		fclose(fp);
		return NULL;
	}

	if (fscanf(fp, "%d\n", &html_len) != 1){
		printf("Error: failed to read HTML length from %s\n", filename);
		fclose(fp);
		return NULL;
	}

	html = malloc(html_len + 2);
	while ( (c = fgetc(fp)) != EOF && i < html_len) {
		html[i++] = c;
	}
	html[i] = '\0';

	fclose(fp);

	webpage_t *page = webpage_new(url, depth, html);
	
	if (page == NULL){
		printf("Error: webpage_new failed for %s\n", url);
		free(html);
		return NULL;
	}

	return page;
}
