#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "share/utf8.h"
#include "share/win_utf8_io.h"
#include "share/windows_unicode_filenames.h"

#define FREE_POINTER( x )	{ if( x ) { free( x ); } }

typedef struct S_PLAYLIST {
	char **list;

	uint32_t len;
	uint32_t cap;

	uint32_t current;

	bool loop;
	bool random;
} playlist_t;

playlist_t * pl_init(char *fname);
void pl_destroy(playlist_t * pl);

void pl_set_loop(playlist_t * pl, bool l);
void pl_set_random(playlist_t * pl, bool r);

bool _pl_add(playlist_t * pl, char *line);

bool pl_get_next(playlist_t * pl, char *file);

bool _pl_get(playlist_t * pl, char *file);
bool _pl_get_random(playlist_t * pl, char *file);

void pl_print(playlist_t * pl);

#endif
