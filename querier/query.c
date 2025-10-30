/* 
 * query.c --- 
 * 
 * Author: Ava D. Rosenbaum
 * Created: 10-30-2025
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

static uint64_t LINE_LEN = 256;

// converts a word to lowercase
char *NormalizeWord(const char *word){
	if (word == NULL) return NULL;
	int len = strlen(word);

	char *normalized = malloc((len + 1) * sizeof(char));
	if (!normalized) return NULL;
	for (int i = 0; i < len; i++) {
		if (!isalpha((unsigned char)word[i])) {
			free(normalized);
			return NULL;
		}
		normalized[i] = tolower((unsigned char)word[i]);
	}
	normalized[len] = '\0';
	return normalized;
}

int main(void) {
	char line[LINE_LEN];
	char **wordList = NULL; // growing list of words
	char **tmp;
	char *token;
	char *lowercase;
	int wordCount = 0;
	int invalid = 0;

	printf("> ");
	while (fgets(line, sizeof(line), stdin) != NULL){
		line[strcspn(line, "\n")] = '\0'; // remove trailing newline

		if (line[0] == '\0') {
			printf("> ");
			continue;
		}

		token = strtok(line, " ");
		while (token != NULL) {
			lowercase =  NormalizeWord(token);
			if (lowercase == NULL){
				invalid = 1;
			}
			else if (!invalid) {
				tmp = realloc(wordList, (wordCount + 1)* sizeof(char *));
				if (tmp == NULL){
					printf("realloc failed\n");
					exit(EXIT_FAILURE);
				}
				wordList = tmp;
				wordList[wordCount++] = lowercase;
			}
			else {
				free(lowercase);
			}

			token = strtok(NULL, " ");
		}
		
		if (invalid){
			printf("[invalid query]\n");
		}
		else if (wordCount > 0){
			for (int i = 0; i < wordCount; i++){
				printf("%s ", wordList[i]);
				free(wordList[i]);
			}
			printf("\n");
		}
		free(wordList);
		wordList = NULL;
		wordCount = 0;
		invalid = 0;
		printf("> ");
	}
	return 0;
}
