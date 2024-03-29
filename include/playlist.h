#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifndef FREE_POINTER
	#define FREE_POINTER( x )	{ if( x ) { free( x ); x = NULL; } }
#endif

typedef struct S_PLAYLIST {
	char **list;

	uint32_t len;
	uint32_t cap;

	uint32_t current;

	bool loop;
	bool random;

	bool verbose;
	bool silent;
} playlist_t;

playlist_t * pl_init(const char *fname);
void pl_destroy(playlist_t * pl);

void pl_set_loop(playlist_t * pl, bool l);
void pl_set_random(playlist_t * pl, bool r);

void pl_set_verbose(playlist_t * pl, bool v);
void pl_set_silent(playlist_t * pl, bool s);

const char* pl_get_next(playlist_t * pl);

void pl_print(playlist_t * pl);

#endif
