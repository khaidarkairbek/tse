/* 
 * indexer.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-21-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pageio.h>
#include <webpage.h>

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);
	int i;
	char c;

	if (len < 3) return NULL; // discard short words
	
	char *normalized = malloc(len + 1);
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

int main(int argc, char *argv[]){
	if (argc != 2){
		printf("usage: indexer <page_directory>\n");
		exit(EXIT_FAILURE);
	}
	char *pagedir = argv[1];
	webpage_t *page = pageload(1, pagedir);
	int pos = 0;
	char *word = NULL;
	char *normalized;
	
	if (page == NULL){
		printf("Error: could not load page 1 from directory %s\n", pagedir);
		exit(EXIT_FAILURE);
	}

	// extract words
	//	printf("Original words:\n");
	while ((pos = webpage_getNextWord(page, pos, &word)) > 0){
		//	printf("%s\n", word);
		free(word);
	}
	
	
  pos = 0;
	// normalized words
	while ((pos = webpage_getNextWord(page, pos, &word)) > 0){
		normalized = NormalizeWord(word);
		free(word);
		if (normalized != NULL) {
			printf("%s\n", normalized);
			free(normalized);
		}
	}
	
	webpage_delete(page);
					 
}
