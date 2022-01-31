#include "playlist.h"

static const char * _pl_get(playlist_t * pl);
static const char * _pl_get_random(playlist_t * pl);
static bool _pl_add(playlist_t * pl, char *line);

playlist_t * pl_init(const char *fname) {
	FILE *fp = NULL;
	char line[1024];

	playlist_t * pl = (playlist_t *)malloc(sizeof(playlist_t));
	memset(pl, 0, sizeof(playlist_t));

	pl->list = (char **)malloc(20 * sizeof(char*));
	if (!pl->list) {
		fprintf(stderr, "playlist: malloc() failed\n");
		return NULL;
	}
	pl->len = 0;
	pl->cap = 20;
	pl->current = 0;

	pl->loop = false;
	pl->random = false;

	fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "playlist: fopen() failed at '%s'\n", fname);
		pl_destroy(pl);
		return NULL;
	}

	while (fgets(line, sizeof(line), fp)) {
		//if (utf8_encode(line, &line_uft8) >= 0) {
			if (!_pl_add(pl, line)) {
				pl_destroy(pl);
				fprintf(stderr, "playlist: _pl_add() failed\n");
				return NULL;
			}
		//}
		//else {
		//	fprintf(stderr, "playlist: utf8_encode() failed\n");
		//}

	}

	fclose(fp);

	srand(time(NULL));

	return pl;
}

void pl_destroy(playlist_t * pl) {
	uint32_t i;

	for (i = 0; i < pl->len; i++) {
		FREE_POINTER( pl->list[i] );
	}

	FREE_POINTER( pl->list );
}

void pl_set_loop(playlist_t * pl, bool l) {
	pl->loop = l;
}

void pl_set_random(playlist_t * pl, bool r) {
	pl->random = r;
}

void pl_set_verbose(playlist_t * pl, bool v) {
	pl->verbose = v;
}

void pl_set_silent(playlist_t * pl, bool s) {
	pl->silent = s;
}

static bool _pl_add(playlist_t * pl, char *file) {
	if (pl->len < pl->cap) {
		uint32_t ind = strlen(file);
		if (file[ind - 1] == '\n') {
			file[ind - 1] = '\0';
		}

		pl->list[pl->len] = (char *)malloc(strlen(file) + 1);
		strcpy(pl->list[pl->len], file);
		pl->len++;
	}
	else {
		char **new_list = (char **)malloc(pl->cap * 2 * sizeof(char*));
		if (!new_list) {
			return false;
		}

		pl->cap *= 2;
		uint32_t i;

		for (i = 0; i < pl->len; i++) {
			new_list[i] = (char *)malloc(strlen(pl->list[i]) + 1);
			strcpy(new_list[i], pl->list[i]);
		}

		for (i = 0; i < pl->len; i++) {
			FREE_POINTER(pl->list[i]);
		}

		FREE_POINTER(pl->list);

		pl->list = new_list;

		pl->list[(pl->len + 1)] = (char *)malloc(strlen(file) + 1);
		strcpy(pl->list[(pl->len + 1)], file);
		pl->len++;
	}

	return true;
}

const char* pl_get_next(playlist_t * pl) {
	if (pl->random) {
		return _pl_get_random(pl);
	} else {
		return _pl_get(pl);
	}
}

static const char*  _pl_get(playlist_t * pl) {
	if (pl->current == pl->len) {
		if (pl->loop) {
			pl->current = 0;
		} else {
			return NULL;
		}
	}

	return pl->list[pl->current++];
}

static const char*  _pl_get_random(playlist_t * pl) {
	int r = rand() % pl->len;

	return pl->list[r];
}

void pl_print(playlist_t * pl) {
	if (!pl->silent) {
		uint32_t i;

		if (pl->verbose) {
			printf("loop   : %s\n", pl->loop ? "true" : "false");
			printf("random : %s\n", pl->random ? "true" : "false");
		}

		for (i = 0; i < pl->len; i++) {
			printf("%s\n", pl->list[i]);
		}
		printf("\n");
	}
}
