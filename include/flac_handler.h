#ifndef __FLAC_HANDLER_H__
#define __DLAC_HANDLER_H__

#define FLAC__NO_DLL
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>

#include "ice_protocol.h"

#include <ogg/ogg.h>

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
	uint32_t bitrate;

	uint32_t write_count;

	double duration;
	double timestamp;
	double timestamp_new;

	ice_clinet_t *ice;
}flac_handler_t;

flac_handler_t * fh_init(ice_clinet_t *ice);
void fh_destroy(flac_handler_t * fh);

int fh_decode_file(flac_handler_t * fh, const char *fname);
int fh_encode_stream(flac_handler_t * fh);

static FLAC__StreamDecoderWriteStatus _decode_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
static FLAC__StreamDecoderMetadataCallback _decode_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
static FLAC__StreamDecoderErrorCallback _decode_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);

static FLAC__StreamEncoderWriteStatus _encode_write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data);

void strtoupper(char *up, char *str);

void printprogress(double time, double duration);

#endif