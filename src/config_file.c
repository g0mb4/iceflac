#include "config_file.h"

static cf_pair_t *head = NULL;

int cf_load(const char *fname) {
	FILE *fp = NULL;

	int p;
	char *paths[] = { "settings", "server-settings", "ice-settings", "playlist-settings" };

	if ((fp = fopen(fname, "r")) == NULL) {
		fprintf(stderr, "config: fopen() failed\n");
		return -1;
	}

	mxml_node_t * tree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);
	mxml_node_t * node = NULL;
	if (!tree) {
		fprintf(stderr, "config: mxmlLoadFile() failed\n");
		fclose(fp);
		return -2;
	}

	for (p = 0; p < 4; p++) {
		node = mxmlFindPath(tree, paths[p]);
		if (!node) {
			fprintf(stderr, "config: mxmlFindPath(%s) failed, wrong config format\n", paths[p]);
			fclose(fp);
			return -3;
		}

		node = mxmlGetNextSibling(node);
		while (node) {
			if (mxmlGetElement(node) && mxmlGetOpaque(node)) {
				//fprintf(stdout, "Element = %s\n", mxmlGetElement(node));
				//fprintf(stdout, "  Value = %s\n", mxmlGetOpaque(node));

				_cf_add_pair(mxmlGetElement(node), mxmlGetOpaque(node));
			}

			node = mxmlGetNextSibling(node);
		}
	}

	fclose(fp);

	return 0;
}

void _cf_add_pair(const char *key, const char *value) {
	if (!head) {
		head = (cf_pair_t*)malloc(sizeof(cf_pair_t));

		head->key = (char*)malloc(strlen(key) + 1);
		strcpy(head->key, key);

		head->value = (char*)malloc(strlen(value) + 1);
		strcpy(head->value, value);

		head->next = NULL;
	} else {
		cf_pair_t * current = head;

		while(current->next) {
			current = current->next;
		}

		current->next = (cf_pair_t*)malloc(sizeof(cf_pair_t));

		current->next->key = (char*)malloc(strlen(key) + 1);
		strcpy(current->next->key, key);

		current->next->value = (char*)malloc(strlen(value) + 1);
		strcpy(current->next->value, value);

		current->next->next = NULL;
	}
}

void cf_print(void) {
	if (!head) {
		return;
	}

	cf_pair_t * current = head;
	while (current) {
		printf("  %s = %s\n", current->key, current->value);
		current = current->next;
	}
}

bool cf_get_value(const char *key, char *value) {
	if (!head) {
		return false;
	}

	cf_pair_t * current = head;
	while (current) {
		if (!strcmp(key, current->key)) {
			strcpy(value, current->value);

			return true;
		}

		current = current->next;
	}

	return false;
}

void cf_destroy(void) {
	cf_pair_t * current;
	while (head) {
		current = head;
		head = head->next;
		free(current);
	}
}