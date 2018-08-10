#include "flac_handler.h"

flac_handler_t * fh_init(ice_clinet_t *ice) {
	flac_handler_t * fh = (flac_handler_t*)malloc(sizeof(flac_handler_t));
	memset(fh, 0, sizeof(flac_handler_t));

	if (!fh) {
		fprintf(stderr, "flac: malloc() failed\n");
		return NULL;
	}

	fh->decoder = FLAC__stream_decoder_new();

	fh->ice = ice;

	if (!fh->decoder) {
		fprintf(stderr, "flac: FLAC__stream_decoder_new() failed\n");
		return NULL;
	}

	(void)FLAC__stream_decoder_set_md5_checking(fh->decoder, true);

	fh->encoder = FLAC__stream_encoder_new();

	if (!fh->encoder) {
		fprintf(stderr, "flac: FLAC__stream_encoder_new() failed\n");
		return NULL;
	}

	return fh;
}

void fh_destroy(flac_handler_t * fh) {
	if (fh->decoder) {
		FLAC__stream_decoder_delete(fh->decoder);
	}

	int i;
	for (i = 0; i < fh->ice->channels; i++) {
		FREE_POINTER( fh->buffer[i] );
	}

	FREE_POINTER( fh->buffer );
}

void strtoupper(char *up, char *str) {
	while (*str) {
		*up++ = toupper(*str++);
	}
	*up = '\0';
}

int fh_decode_file(flac_handler_t * fh, const char *fname) {
	FLAC__StreamDecoderInitStatus init_status;
	FLAC__bool ok = true;
	FLAC__StreamMetadata *tags = (FLAC__StreamMetadata *)malloc(sizeof(FLAC__StreamMetadata));
	int i;

	init_status = FLAC__stream_decoder_init_file(fh->decoder, fname, _decode_write_callback, _decode_metadata_callback, _decode_error_callback, (void*)fh);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		fprintf(stderr, "decoder: FLAC__stream_decoder_init_file() failed with error: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
		return -1;
	}

	char *name = NULL, *value = NULL, *name_up = NULL;
	if (FLAC__metadata_get_tags(fname, &tags)) {
		for (i = 0; i < tags->data.vorbis_comment.num_comments; i++) {
			FLAC__StreamMetadata_VorbisComment_Entry entry = tags->data.vorbis_comment.comments[i];
			
			if (FLAC__metadata_object_vorbiscomment_entry_to_name_value_pair(entry, &name, &value)) {
				name_up = (char *)malloc(strlen(name) + 1);
				strtoupper(name_up, name);

				if (!strcmp(name_up, "ARTIST")) {
					ice_set_artist(fh->ice, value);
				}
				else if (!strcmp(name_up, "TITLE")) {
					ice_set_title(fh->ice, value);
				}

				free(name_up);
			}
		}
	}

	FLAC__metadata_object_delete(tags);


	for (i = 0; i < fh->ice->channels; i++) {
		FREE_POINTER( fh->buffer[i] );
	}

	FREE_POINTER( fh->buffer );

	ok = FLAC__stream_decoder_process_until_end_of_stream(fh->decoder);
	if (!fh->ice->silent && fh->ice->verbose) {
		fprintf(stderr, "decoding: %s\n", ok ? "succeeded" : "FAILED");
	}
	//fprintf(stderr, "   state: %s\n", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(fh->decoder)]);

	FLAC__stream_decoder_finish(fh->decoder);

	return ok ? 0 : -2;
}

FLAC__StreamDecoderWriteStatus _decode_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	flac_handler_t * fh = (flac_handler_t *)client_data;
	size_t i;

	(void)decoder;

	if (!fh->buffer) {
		fprintf(stderr, "decoder: unable to allocate buffer\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	if (fh->ice->channels != 2 || fh->bits_per_sampe != 16) {
		fprintf(stderr, "decoder: decoder only supports 16bit stereo streams\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (frame->header.channels < 2) {
		fprintf(stderr, "decoder: frame contains %d channels (should be minimum 2)\n", frame->header.channels);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (buffer[0] == NULL) {
		fprintf(stderr, "decoder: buffer [0] is NULL\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	if (buffer[1] == NULL) {
		fprintf(stderr, "decoder: buffer [1] is NULL\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	/* write decoded PCM samples */
	for (i = 0; i < frame->header.blocksize; i++) {
		fh->buffer[0][fh->buf_position] = (FLAC__int16)buffer[0][i];		/* left channel */
		fh->buffer[1][fh->buf_position] = (FLAC__int16)buffer[1][i];		/* right channel */
		fh->buf_position++;
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderMetadataCallback _decode_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	uint32_t i;
	flac_handler_t * fh = (flac_handler_t *)client_data;

	(void)decoder;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		fh->total_samples = metadata->data.stream_info.total_samples;
		fh->ice->ice_samplerate = metadata->data.stream_info.sample_rate;
		fh->ice->channels = metadata->data.stream_info.channels;
		fh->bits_per_sampe = metadata->data.stream_info.bits_per_sample;
		fh->total_size = (FLAC__uint32)(fh->total_samples * fh->ice->channels * (fh->bits_per_sampe / 8));	// size in bytes
		fh->duration = (double)(fh->total_samples) / (double)(fh->ice->ice_samplerate);
		fh->ice->ice_bitrate_raw = (uint32_t)((fh->total_size * 8) / (fh->duration));
		fh->bitrate = 0.00085034 * fh->ice->channels * fh->bits_per_sampe * fh->ice->ice_samplerate; // ?

		if (!fh->ice->silent && fh->ice->verbose) {
			fprintf(stderr, "sample rate    : %u Hz\n", fh->ice->ice_samplerate);
			fprintf(stderr, "channels       : %u\n", fh->ice->channels);
			fprintf(stderr, "bits per sample: %u\n", fh->bits_per_sampe);
			fprintf(stderr, "total samples  : %llu\n", fh->total_samples);
			fprintf(stderr, "total size     : %u B\n", fh->total_size);
			fprintf(stderr, "duration       : %.4f s\n", fh->duration);
			fprintf(stderr, "bitrate (FALC) : %u kb/s\n", fh->bitrate);
			fprintf(stderr, "bitrate (raw)  : %u kb/s\n", (fh->ice->ice_bitrate_raw / 1000));
		}

		fh->buf_position = 0;
		fh->buffer = (uint32_t**)malloc(fh->ice->channels * sizeof(uint32_t*));

		if (!fh->buffer) {
			fprintf(stderr, "decoder: unable to allocate buffer\n");
			fh->buffer = NULL;
		}

		for(i = 0; i < fh->ice->channels; i++) {
			fh->buffer[i] = (uint32_t*)malloc((fh->total_samples * (fh->bits_per_sampe / 8)) * sizeof(uint32_t));
			if (!fh->buffer[i]) {
				fprintf(stderr, "decoder: unable to allocate buffer for channels\n");
			}
		}
	}
}

FLAC__StreamDecoderErrorCallback _decode_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	(void)decoder, (void)client_data;

	fprintf(stderr, "decoder: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

int fh_encode_stream(flac_handler_t * fh) {
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__bool ok = true;

	FLAC__stream_encoder_set_channels(fh->encoder, fh->ice->channels);
	FLAC__stream_encoder_set_ogg_serial_number(fh->encoder, rand());
	FLAC__stream_encoder_set_bits_per_sample(fh->encoder, fh->bits_per_sampe);
	FLAC__stream_encoder_set_sample_rate(fh->encoder, fh->ice->ice_samplerate);

	/* do NOT check the result, it will fail, but works .... */
	FLAC__stream_encoder_init_ogg_stream(fh->encoder, NULL, _encode_write_callback, NULL, NULL, NULL, (void*)fh);
	
	fh->streamed_bytes = 0;
	fh->write_count = 0;

	fh->timestamp = 0;
	fh->timestamp_new = 0;
	ok = FLAC__stream_encoder_process(fh->encoder, fh->buffer, fh->total_samples);
	//fprintf(stderr, "encoding: %s\n", ok ? "succeeded" : "FAILED");

	FLAC__stream_encoder_finish(fh->encoder);

	return ok ? 0 : -2;
}

static FLAC__StreamEncoderWriteStatus _encode_write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame, void *client_data) {
	flac_handler_t * fh = (flac_handler_t *)client_data;
	ogg_page og;
	int granulepos;
	char time[128];

	(void)encoder;

	if ((fh->write_count & 0x1) == 0) {
		/* ogg header */
		og.header = (unsigned char *)buffer;
		og.header_len = bytes;
		og.body = NULL;
		og.body_len = 0;

		switch ((granulepos = ogg_page_granulepos(&og)))
		{
		case -1:
			break;
		case 0:
			break;
		default:
			fh->timestamp_new = (double)granulepos / (double)fh->ice->ice_samplerate;
		}
	} else {
		/* ogg body */
		fh->streamed_bytes += bytes;

		if (!fh->ice->silent) {
			printprogress(fh->timestamp_new, fh->duration);
		}

		double sleepms = (int)((fh->timestamp_new - fh->timestamp) * 1000.0);

		if (sleepms > 40) {
			sleepms -= 10;	// give some time to the server to process
		}

		Sleep(sleepms);

		fh->timestamp = fh->timestamp_new;
	}

	if (ice_send_data(fh->ice, buffer, bytes) < 0) {
		fprintf(stderr, "encoder: ice_send_data() failed\n");
		return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
	}

	fh->write_count++;
	
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

void printprogress(double time, double duration) {
	int hour = (int)time / 3600;
	int min = ((int)time / 60) % 60;
	int sec = (int)time % 60;
	int i;

	double progress = ((time / duration) * 100.0);

	printf("\r [");
	for (i = 0; i < 21; i++) {
		if ((i * 5) <= (progress)) {
			printf("#");
		} else {
			printf("_");
		}
	}
	printf("]  ");

	if (hour == 0) {
		printf("%02d:%02d", min, sec);
	}
	else {
		printf("%d:%02d:%02d", hour, min, sec);
	}
}
