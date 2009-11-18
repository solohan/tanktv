#include "Decoder.h"

MP3Decoder::MP3Decoder()
  : Decoder(), 
    m_mpg123(0),
    m_total_frames(0)
{
  int errno;

  if ((errno=mpg123_init()) != MPG123_OK || (m_mpg123=mpg123_new(NULL, &errno)) == NULL) {
    debug("couldn't initialize mpg123: %s\n", mpg123_plain_strerror(errno));
    mpg123_exit();
    m_mpg123 = 0;
    return;    
  }  
}

MP3Decoder::~MP3Decoder()
{
  if (m_mpg123) {
    mpg123_delete(m_mpg123);
    mpg123_exit();
  }
}

bool MP3Decoder::valid()
{
  return (m_mpg123 != 0);
}

bool MP3Decoder::open(const char *file)
{
  int encoding;
  int meta;

  if (mpg123_open(m_mpg123, (char *)file) != MPG123_OK) {
    debug("couldn't open mp3 file: %s\n", mpg123_strerror(m_mpg123));
    return false;
  }
  
  if (mpg123_getformat(m_mpg123, &m_rate, &m_channels, &encoding) != MPG123_OK) {
    debug("couldn't get mp3 format: %s\n", mpg123_strerror(m_mpg123));
    return false;
  }
  
  if (encoding != MPG123_ENC_SIGNED_16) {
    debug("only signed 16-bit encoding supported!\n");
    return false;
  }

  m_buffer_size = mpg123_outblock(m_mpg123);
  m_buffer = (unsigned char *)malloc(m_buffer_size);

  m_bits = 16;
  
  mpg123_format_none(m_mpg123);
  mpg123_format(m_mpg123, m_rate, m_channels, encoding);

  m_v1 = 0;
  m_v2 = 0;
  if ((meta=mpg123_meta_check(m_mpg123))) {
    if (meta & MPG123_ID3) {
      mpg123_id3(m_mpg123, &m_v1, &m_v2);
    }
  }

  mpg123_position(m_mpg123, 0, 0, NULL, &m_total_frames, NULL, NULL);  
  m_position = 0;
}

void MP3Decoder::close()
{
  if (m_mpg123) {
    mpg123_close(m_mpg123);
  }
}

bool MP3Decoder::read(unsigned char **buffer, size_t *bytes_read)
{
  off_t frames, frames_left;

  *buffer = m_buffer;

  if (mpg123_read(m_mpg123, m_buffer, m_buffer_size, bytes_read) != MPG123_OK)
    return false;

  mpg123_position(m_mpg123, 0, *bytes_read, &frames, &frames_left, NULL, NULL);  
  m_total_frames = frames + frames_left;
  m_position = frames * 1.0 / m_total_frames;

  return true;
}

void MP3Decoder::seek(float percent) 
{
  if (m_mpg123 && percent >= 0 && percent <= 1) {
    mpg123_seek_frame(m_mpg123, (int)(percent * m_total_frames + 0.5), SEEK_SET);
  }  
}

const char *MP3Decoder::title()
{
  if (m_v2) return m_v2->title->p;
  if (m_v1) {
    m_v1->title[sizeof(m_v1->title)-1] = 0;
    return m_v1->title;
  }
  return NULL;
}

const char *MP3Decoder::album()
{
  if (m_v2) return m_v2->album->p;
  if (m_v1) {
    m_v1->album[sizeof(m_v1->album)-1] = 0;
    return m_v1->album;
  }
  return NULL;
}

const char *MP3Decoder::artist()
{
  if (m_v2) return m_v2->artist->p;
  if (m_v1) {
    m_v1->artist[sizeof(m_v1->artist)-1] = 0;
    return m_v1->artist;
  }
  return NULL;
}

const char *MP3Decoder::genre()
{
  if (m_v2) return m_v2->genre->p;
  if (m_v1) return NULL;
  return NULL;
}