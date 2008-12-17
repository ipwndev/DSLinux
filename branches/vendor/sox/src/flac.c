/* libSoX file format: FLAC   (c) 2006-7 robs@users.sourceforge.net
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "sox_i.h"

#include <string.h>
/* Next line for systems that don't define off_t when you #include
   stdio.h; apparently OS/2 has this bug */
#include <sys/types.h>

#include <FLAC/all.h>

/* Workaround for flac versions < 1.1.2 */
#define FLAC__metadata_object_vorbiscomment_append_comment(object, entry, copy)\
  FLAC__metadata_object_vorbiscomment_insert_comment(object, object->data.vorbis_comment.num_comments, entry, copy)

#if !defined(FLAC_API_VERSION_CURRENT)
#define FLAC_API_VERSION_CURRENT 7
#define FLAC__StreamDecoder FLAC__FileDecoder
#define FLAC__stream_decoder_new FLAC__file_decoder_new
#define FLAC__stream_decoder_set_metadata_respond_all FLAC__file_decoder_set_metadata_respond_all
#define FLAC__stream_decoder_set_md5_checking FLAC__file_decoder_set_md5_checking
#define FLAC__stream_decoder_process_until_end_of_metadata FLAC__file_decoder_process_until_end_of_metadata
#define FLAC__stream_decoder_process_single FLAC__file_decoder_process_single
#define FLAC__stream_decoder_finish FLAC__file_decoder_finish
#define FLAC__stream_decoder_delete FLAC__file_decoder_delete
#define FLAC__stream_decoder_seek_absolute FLAC__file_decoder_seek_absolute
#endif

#define MAX_COMPRESSION 8


typedef struct {
  /* Info: */
  unsigned bits_per_sample;
  unsigned channels;
  unsigned sample_rate;
  unsigned total_samples;

  /* Decode buffer: */
  FLAC__int32 const * const * decoded_wide_samples;
  unsigned number_of_wide_samples;
  unsigned wide_sample_number;

  FLAC__StreamDecoder * decoder;
  FLAC__bool eof;

  /* Encode buffer: */
  FLAC__int32 * decoded_samples;
  unsigned number_of_samples;

  FLAC__StreamEncoder * encoder;
  FLAC__StreamMetadata * metadata[2];
  unsigned num_metadata;
} priv_t;



static void FLAC__decoder_metadata_callback(FLAC__StreamDecoder const * const flac, FLAC__StreamMetadata const * const metadata, void * const client_data)
{
  sox_format_t * ft = (sox_format_t *) client_data;
  priv_t * p = (priv_t *)ft->priv;

  (void) flac;

  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    p->bits_per_sample = metadata->data.stream_info.bits_per_sample;
    p->channels = metadata->data.stream_info.channels;
    p->sample_rate = metadata->data.stream_info.sample_rate;
    p->total_samples = metadata->data.stream_info.total_samples;
  }
  else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
    size_t i;

    if (metadata->data.vorbis_comment.num_comments == 0)
      return;

    if (ft->oob.comments != NULL) {
      lsx_warn("multiple Vorbis comment block ignored");
      return;
    }

    for (i = 0; i < metadata->data.vorbis_comment.num_comments; ++i)
      sox_append_comment(&ft->oob.comments, (char const *) metadata->data.vorbis_comment.comments[i].entry);
  }
}



static void FLAC__decoder_error_callback(FLAC__StreamDecoder const * const flac, FLAC__StreamDecoderErrorStatus const status, void * const client_data)
{
  sox_format_t * ft = (sox_format_t *) client_data;

  (void) flac;

  lsx_fail_errno(ft, SOX_EINVAL, "%s", FLAC__StreamDecoderErrorStatusString[status]);
}



static FLAC__StreamDecoderWriteStatus FLAC__frame_decode_callback(FLAC__StreamDecoder const * const flac, FLAC__Frame const * const frame, FLAC__int32 const * const buffer[], void * const client_data)
{
  sox_format_t * ft = (sox_format_t *) client_data;
  priv_t * p = (priv_t *)ft->priv;

  (void) flac;

  if (frame->header.bits_per_sample != p->bits_per_sample || frame->header.channels != p->channels || frame->header.sample_rate != p->sample_rate) {
    lsx_fail_errno(ft, SOX_EINVAL, "FLAC ERROR: parameters differ between frame and header");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }

  p->decoded_wide_samples = buffer;
  p->number_of_wide_samples = frame->header.blocksize;
  p->wide_sample_number = 0;
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}



static int start_read(sox_format_t * const ft)
{
  priv_t * p = (priv_t *)ft->priv;
  lsx_debug("API version %u", FLAC_API_VERSION_CURRENT);
  p->decoder = FLAC__stream_decoder_new();
  if (p->decoder == NULL) {
    lsx_fail_errno(ft, SOX_ENOMEM, "FLAC ERROR creating the decoder instance");
    return SOX_EOF;
  }

  FLAC__stream_decoder_set_md5_checking(p->decoder, sox_true);
  FLAC__stream_decoder_set_metadata_respond_all(p->decoder);
#if FLAC_API_VERSION_CURRENT <= 7
  /* This is wrong: not using SoX IO, and there will be 2 FILEs open;
   * however, it's an old FLAC API, so not worth fixing now. */
  FLAC__file_decoder_set_filename(p->decoder, ft->filename);
  FLAC__file_decoder_set_write_callback(p->decoder, FLAC__frame_decode_callback);
  FLAC__file_decoder_set_metadata_callback(p->decoder, FLAC__decoder_metadata_callback);
  FLAC__file_decoder_set_error_callback(p->decoder, FLAC__decoder_error_callback);
  FLAC__file_decoder_set_client_data(p->decoder, ft);
  if (FLAC__file_decoder_init(p->decoder) != FLAC__FILE_DECODER_OK) {
    lsx_fail_errno(ft, SOX_EHDR, "FLAC ERROR initialising decoder");
    return SOX_EOF;
  }
#else
  if (FLAC__stream_decoder_init_FILE(p->decoder, ft->fp, /* Not using SoX IO */
      FLAC__frame_decode_callback, FLAC__decoder_metadata_callback,
      FLAC__decoder_error_callback, ft) != FLAC__STREAM_DECODER_INIT_STATUS_OK){
    lsx_fail_errno(ft, SOX_EHDR, "FLAC ERROR initialising decoder");
    return SOX_EOF;
  }
  ft->fp = NULL; /* Transfer ownership of fp to FLAC */
#endif

  if (!FLAC__stream_decoder_process_until_end_of_metadata(p->decoder)) {
    lsx_fail_errno(ft, SOX_EHDR, "FLAC ERROR whilst decoding metadata");
    return SOX_EOF;
  }

#if FLAC_API_VERSION_CURRENT <= 7
  if (FLAC__file_decoder_get_state(p->decoder) != FLAC__FILE_DECODER_OK && FLAC__file_decoder_get_state(p->decoder) != FLAC__FILE_DECODER_END_OF_FILE) {
#else
  if (FLAC__stream_decoder_get_state(p->decoder) > FLAC__STREAM_DECODER_END_OF_STREAM) {
#endif
    lsx_fail_errno(ft, SOX_EHDR, "FLAC ERROR during metadata decoding");
    return SOX_EOF;
  }

  ft->encoding.encoding = SOX_ENCODING_FLAC;
  ft->signal.rate = p->sample_rate;
  ft->encoding.bits_per_sample = p->bits_per_sample;
  ft->signal.channels = p->channels;
  ft->signal.length = p->total_samples * p->channels;
  return SOX_SUCCESS;
}


static size_t read_samples(sox_format_t * const ft, sox_sample_t * sampleBuffer, size_t const requested)
{
  priv_t * p = (priv_t *)ft->priv;
  size_t actual = 0;

  while (!p->eof && actual < requested) {
    if (p->wide_sample_number >= p->number_of_wide_samples)
      FLAC__stream_decoder_process_single(p->decoder);
    if (p->wide_sample_number >= p->number_of_wide_samples)
      p->eof = sox_true;
    else {
      unsigned channel;

      for (channel = 0; channel < p->channels; channel++, actual++) {
        FLAC__int32 d = p->decoded_wide_samples[channel][p->wide_sample_number];
        switch (p->bits_per_sample) {
        case  8: *sampleBuffer++ = SOX_SIGNED_8BIT_TO_SAMPLE(d,); break;
        case 16: *sampleBuffer++ = SOX_SIGNED_16BIT_TO_SAMPLE(d,); break;
        case 24: *sampleBuffer++ = SOX_SIGNED_24BIT_TO_SAMPLE(d,); break;
        case 32: *sampleBuffer++ = SOX_SIGNED_32BIT_TO_SAMPLE(d,); break;
        }
      }
      ++p->wide_sample_number;
    }
  }
  return actual;
}



static int stop_read(sox_format_t * const ft)
{
  priv_t * p = (priv_t *)ft->priv;
  if (!FLAC__stream_decoder_finish(p->decoder) && p->eof)
    lsx_warn("decoder MD5 checksum mismatch.");
  FLAC__stream_decoder_delete(p->decoder);
  return SOX_SUCCESS;
}



static FLAC__StreamEncoderWriteStatus flac_stream_encoder_write_callback(FLAC__StreamEncoder const * const flac, const FLAC__byte buffer[], size_t const bytes, unsigned const samples, unsigned const current_frame, void * const client_data)
{
  sox_format_t * const ft = (sox_format_t *) client_data;
  (void) flac, (void) samples, (void) current_frame;

  return lsx_writebuf(ft, buffer, bytes) == bytes ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}



static void flac_stream_encoder_metadata_callback(FLAC__StreamEncoder const * encoder, FLAC__StreamMetadata const * metadata, void * client_data)
{
  (void) encoder, (void) metadata, (void) client_data;
}



#if FLAC_API_VERSION_CURRENT >= 8
static FLAC__StreamEncoderSeekStatus flac_stream_encoder_seek_callback(FLAC__StreamEncoder const * encoder, FLAC__uint64 absolute_byte_offset, void * client_data)
{
  sox_format_t * const ft = (sox_format_t *) client_data;
  (void) encoder;
  if (!ft->seekable)
    return FLAC__STREAM_ENCODER_SEEK_STATUS_UNSUPPORTED;
  else if (lsx_seeki(ft, (off_t)absolute_byte_offset, SEEK_SET) != SOX_SUCCESS)
    return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
  else
    return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;
}



static FLAC__StreamEncoderTellStatus flac_stream_encoder_tell_callback(FLAC__StreamEncoder const * encoder, FLAC__uint64 * absolute_byte_offset, void * client_data)
{
  sox_format_t * const ft = (sox_format_t *) client_data;
  off_t pos;
  (void) encoder;
  if (!ft->seekable)
    return FLAC__STREAM_ENCODER_TELL_STATUS_UNSUPPORTED;
  else if ((pos = ftello(ft->fp)) < 0)
    return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
  else {
    *absolute_byte_offset = (FLAC__uint64)pos;
    return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
  }
}
#endif



static int start_write(sox_format_t * const ft)
{
  priv_t * p = (priv_t *)ft->priv;
  FLAC__StreamEncoderState status;
  unsigned compression_level = MAX_COMPRESSION; /* Default to "best" */

  if (ft->encoding.compression != HUGE_VAL) {
    compression_level = ft->encoding.compression;
    if (compression_level != ft->encoding.compression ||
        compression_level > MAX_COMPRESSION) {
      lsx_fail_errno(ft, SOX_EINVAL,
                 "FLAC compression level must be a whole number from 0 to %i",
                 MAX_COMPRESSION);
      return SOX_EOF;
    }
  }

  p->encoder = FLAC__stream_encoder_new();
  if (p->encoder == NULL) {
    lsx_fail_errno(ft, SOX_ENOMEM, "FLAC ERROR creating the encoder instance");
    return SOX_EOF;
  }
  p->decoded_samples = lsx_malloc(sox_globals.bufsiz * sizeof(FLAC__int32));

  p->bits_per_sample = ft->encoding.bits_per_sample;

  lsx_report("encoding at %i bits per sample", p->bits_per_sample);

  FLAC__stream_encoder_set_channels(p->encoder, ft->signal.channels);
  FLAC__stream_encoder_set_bits_per_sample(p->encoder, p->bits_per_sample);
  FLAC__stream_encoder_set_sample_rate(p->encoder, (unsigned)(ft->signal.rate + .5));

  { /* Check if rate is streamable: */
    static const unsigned streamable_rates[] =
      {8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    size_t i;
    sox_bool streamable = sox_false;
    for (i = 0; !streamable && i < array_length(streamable_rates); ++i)
       streamable = (streamable_rates[i] == ft->signal.rate);
    if (!streamable) {
      lsx_report("non-standard rate; output may not be streamable");
      FLAC__stream_encoder_set_streamable_subset(p->encoder, sox_false);
    }
  }

#if FLAC_API_VERSION_CURRENT >= 10
  FLAC__stream_encoder_set_compression_level(p->encoder, compression_level);
#else
  {
    static struct {
      unsigned blocksize;
      FLAC__bool do_exhaustive_model_search;
      FLAC__bool do_mid_side_stereo;
      FLAC__bool loose_mid_side_stereo;
      unsigned max_lpc_order;
      unsigned max_residual_partition_order;
      unsigned min_residual_partition_order;
    } const options[MAX_COMPRESSION + 1] = {
      {1152, sox_false, sox_false, sox_false, 0, 2, 2},
      {1152, sox_false, sox_true, sox_true, 0, 2, 2},
      {1152, sox_false, sox_true, sox_false, 0, 3, 0},
      {4608, sox_false, sox_false, sox_false, 6, 3, 3},
      {4608, sox_false, sox_true, sox_true, 8, 3, 3},
      {4608, sox_false, sox_true, sox_false, 8, 3, 3},
      {4608, sox_false, sox_true, sox_false, 8, 4, 0},
      {4608, sox_true, sox_true, sox_false, 8, 6, 0},
      {4608, sox_true, sox_true, sox_false, 12, 6, 0},
    };
#define SET_OPTION(x) do {\
  lsx_report(#x" = %i", options[compression_level].x); \
  FLAC__stream_encoder_set_##x(p->encoder, options[compression_level].x);\
} while (0)
    SET_OPTION(blocksize);
    SET_OPTION(do_exhaustive_model_search);
    SET_OPTION(max_lpc_order);
    SET_OPTION(max_residual_partition_order);
    SET_OPTION(min_residual_partition_order);
    if (ft->signal.channels == 2) {
      SET_OPTION(do_mid_side_stereo);
      SET_OPTION(loose_mid_side_stereo);
    }
#undef SET_OPTION
  }
#endif

  if (ft->signal.length != 0) {
    FLAC__stream_encoder_set_total_samples_estimate(p->encoder, (FLAC__uint64)(ft->signal.length / ft->signal.channels));

    p->metadata[p->num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_SEEKTABLE);
    if (p->metadata[p->num_metadata] == NULL) {
      lsx_fail_errno(ft, SOX_ENOMEM, "FLAC ERROR creating the encoder seek table template");
      return SOX_EOF;
    }
    {
#if FLAC_API_VERSION_CURRENT >= 8
      if (!FLAC__metadata_object_seektable_template_append_spaced_points_by_samples(p->metadata[p->num_metadata], (unsigned)(10 * ft->signal.rate + .5), (FLAC__uint64)(ft->signal.length/ft->signal.channels))) {
#else
      size_t samples = 10 * ft->signal.rate;
      size_t total_samples = ft->signal.length/ft->signal.channels;
      if (!FLAC__metadata_object_seektable_template_append_spaced_points(p->metadata[p->num_metadata], total_samples / samples + (total_samples % samples != 0), (FLAC__uint64)total_samples)) {
#endif
        lsx_fail_errno(ft, SOX_ENOMEM, "FLAC ERROR creating the encoder seek table points");
        return SOX_EOF;
      }
    }
    p->metadata[p->num_metadata]->is_last = sox_false; /* the encoder will set this for us */
    ++p->num_metadata;
  }

  if (ft->oob.comments) {     /* Make the comment structure */
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    int i;

    p->metadata[p->num_metadata] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    for (i = 0; ft->oob.comments[i]; ++i) {
      static const char prepend[] = "Comment=";
      char * text = lsx_calloc(strlen(prepend) + strlen(ft->oob.comments[i]) + 1, sizeof(*text));
      /* Prepend `Comment=' if no field-name already in the comment */
      if (!strchr(ft->oob.comments[i], '='))
        strcpy(text, prepend);
      entry.entry = (FLAC__byte *) strcat(text, ft->oob.comments[i]);
      entry.length = strlen(text);
      FLAC__metadata_object_vorbiscomment_append_comment(p->metadata[p->num_metadata], entry, /*copy= */ sox_true);
      free(text);
    }
    ++p->num_metadata;
  }

  if (p->num_metadata)
    FLAC__stream_encoder_set_metadata(p->encoder, p->metadata, p->num_metadata);

#if FLAC_API_VERSION_CURRENT <= 7
  FLAC__stream_encoder_set_write_callback(p->encoder, flac_stream_encoder_write_callback);
  FLAC__stream_encoder_set_metadata_callback(p->encoder, flac_stream_encoder_metadata_callback);
  FLAC__stream_encoder_set_client_data(p->encoder, ft);
  status = FLAC__stream_encoder_init(p->encoder);
#else
  status = FLAC__stream_encoder_init_stream(p->encoder, flac_stream_encoder_write_callback,
      flac_stream_encoder_seek_callback, flac_stream_encoder_tell_callback, flac_stream_encoder_metadata_callback, ft);
#endif

  if (status != FLAC__STREAM_ENCODER_OK) {
    lsx_fail_errno(ft, SOX_EINVAL, "%s", FLAC__StreamEncoderStateString[status]);
    return SOX_EOF;
  }
  return SOX_SUCCESS;
}



static size_t write_samples(sox_format_t * const ft, sox_sample_t const * const sampleBuffer, size_t const len)
{
  priv_t * p = (priv_t *)ft->priv;
  unsigned i;

  for (i = 0; i < len; ++i) {
    long pcm = SOX_SAMPLE_TO_SIGNED_32BIT(sampleBuffer[i], ft->clips);
    p->decoded_samples[i] = pcm >> (32 - p->bits_per_sample);
    switch (p->bits_per_sample) {
      case  8: p->decoded_samples[i] =
          SOX_SAMPLE_TO_SIGNED_8BIT(sampleBuffer[i], ft->clips);
        break;
      case 16: p->decoded_samples[i] =
          SOX_SAMPLE_TO_SIGNED_16BIT(sampleBuffer[i], ft->clips);
        break;
      case 24: p->decoded_samples[i] = /* sign extension: */
          SOX_SAMPLE_TO_SIGNED_24BIT(sampleBuffer[i],ft->clips) << 8;
        p->decoded_samples[i] >>= 8;
        break;
      case 32: p->decoded_samples[i] =
          SOX_SAMPLE_TO_SIGNED_32BIT(sampleBuffer[i],ft->clips);
        break;
    }
  }
  FLAC__stream_encoder_process_interleaved(p->encoder, p->decoded_samples, (unsigned) len / ft->signal.channels);
  return FLAC__stream_encoder_get_state(p->encoder) == FLAC__STREAM_ENCODER_OK ? len : 0;
}



static int stop_write(sox_format_t * const ft)
{
  priv_t * p = (priv_t *)ft->priv;
  FLAC__StreamEncoderState state = FLAC__stream_encoder_get_state(p->encoder);
  unsigned i;

  FLAC__stream_encoder_finish(p->encoder);
  FLAC__stream_encoder_delete(p->encoder);
  for (i = 0; i < p->num_metadata; ++i)
    FLAC__metadata_object_delete(p->metadata[i]);
  free(p->decoded_samples);
  if (state != FLAC__STREAM_ENCODER_OK) {
    lsx_fail_errno(ft, SOX_EINVAL, "FLAC ERROR: failed to encode to end of stream");
    return SOX_EOF;
  }
  return SOX_SUCCESS;
}



/*
 * N.B.  Do not call this function with offset=0 when file-pointer
 * is already 0 or a block of decoded FLAC data will be discarded.
 */
static int seek(sox_format_t * ft, uint64_t offset)
{
  priv_t * p = (priv_t *)ft->priv;
  int result = ft->mode == 'r' && FLAC__stream_decoder_seek_absolute(p->decoder, (FLAC__uint64)(offset / ft->signal.channels)) ?  SOX_SUCCESS : SOX_EOF;
  p->wide_sample_number = p->number_of_wide_samples = 0;
  return result;
}


SOX_FORMAT_HANDLER(flac)
{
  static char const * const names[] = {"flac", NULL};
  static unsigned const encodings[] = {SOX_ENCODING_FLAC, 8, 16, 24, 0, 0};
  static sox_format_handler_t const handler = {SOX_LIB_VERSION_CODE,
    "Free Lossless Audio CODEC compressed audio", names, 0,
    start_read, read_samples, stop_read,
    start_write, write_samples, stop_write,
    seek, encodings, NULL, sizeof(priv_t)
  };
  return &handler;
}
