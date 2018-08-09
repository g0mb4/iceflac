#ifndef __FLAC_HANDLER_H__
#define __DLAC_HANDLER_H__

#define FLAC__NO_DLL

#include <stdint.h>

#include "ice_protocol.h"

/* FALC includes */
#include "FLAC/metadata.h"
#include "FLAC/stream_decoder.h"
#include "FLAC/stream_encoder.h"
#include "share/compat.h"

#define FREE_POINTER( x )	{ if( x ) { free( x ); } }

typedef struct S_FLAC_HANDLER {
	FLAC__StreamDecoder *decoder;
	FLAC__StreamEncoder *encoder;

	uint32_t **buffer;
	uint32_t buf_position;

	FLAC__uint64 streamed_bytes;

	FLAC__uint32 total_size;
	FLAC__uint64 total_samples;
	uint32_t bits_per_sampe;

	ice_clinet_t *ice;
}flac_handler_t;

flac_handler_t * fh_init(ice_clinet_t *ice);
void fh_destroy(flac_handler_t * fh);

int fh_decode_file(flac_handler_t * fh, const char *fname);
int fh_encode_stream(flac_handler_t * fh);

void strtoupper(char *up, char *str);

#endif