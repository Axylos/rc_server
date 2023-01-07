#include "include/store.h"
#include <stdlib.h>
#include <string.h>

struct Node {
	char *key;
	char *val;
	struct Node *next;
};

static struct Node *head = NULL;

int put(char *key, char *val)
{
	if (head == NULL) {
		struct Node *node = malloc(sizeof(struct Node));
		node->key = strdup(key);
		node->val = strdup(val);
		node->next = NULL;
		head = node;
	} else {
		struct Node *prev = head;
		struct Node *next = head;
		do {
			if (strcmp(next->key, key) == 0) {
				free(next->val);
				next->val = strdup(val);
				return 2;
			}
			prev = next;
			next = prev->next;
		} while (next != NULL);

		struct Node *node = malloc(sizeof(struct Node));
		node->key = strdup(key);
		node->val = strdup(val);
		node->next = NULL;
		prev->next = next;
		return 1;
	}

	return 0;
}

char *get(char *key)
{
	struct Node *cur = head;
	while (cur != NULL) {
		if (strcmp(key, cur->key) == 0) {
			return cur->val;
		} else {
			cur = cur->next;
		}
	}
	return NULL;
}
