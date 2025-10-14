/* 
 * list.c --- 
 * 
 * Author: Khaidar Kairbek, Ava Rosenbaum
 * Created: 09-28-2025
 * Version: 1.0
 * 
 * Description: The implementation of linked list ADT. 
 * 
 */

#include "list.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>

static car_t *front = NULL;

int32_t lput(car_t *cp) {
	cp->next = front;
	front = cp;

	return 0; 
}

car_t *lget() {
	car_t *tmp = front;
	if (front == NULL) { // if list is empty
		return NULL; 
	}

	front = front->next; // remove front
	tmp->next = NULL; 
	return tmp;
}

void lapply(void (*fn)(car_t *cp)) {
	car_t *curr;
	for (curr = front; curr != NULL; curr = curr->next) {
		fn(curr);
	}

	return; 
}

car_t *lremove(char *platep) {
	car_t *curr;
	car_t *prev = NULL;
	for (curr = front; curr != NULL; curr = curr->next) {
		if (strcmp (curr->plate, platep) == 0){ // check if plates match
			if (prev == NULL) {
				front = curr->next; // removing the first in the list
			}
			else { // adding to the middle of the list
				prev->next = curr->next;
			}

			curr->next = NULL; // end loop / delete car
			return curr; 
		}
		prev = curr; // move to next in list
	}
	return NULL; 
}



