#ifndef RENDERER_H
#define RENDERER_H

#include "config.h"
#include <directfb.h>
#include <map>
#include "Event.h"

#define IMAGE_CACHE_SIZE 100
#ifdef NMT
#define INPUT_DEVICE DIDID_REMOTE
#else
#define INPUT_DEVICE DIDID_KEYBOARD
#endif

#define FONT_NORMAL 0
#define FONT_BOLD 1

using namespace std;

typedef map<const char *, IDirectFBSurface *> image_map;
typedef map<unsigned, IDirectFBFont *> font_map;

class Renderer
{
 private:
  bool m_initialized;
  bool m_exit;
  IDirectFB *m_dfb;
  IDirectFBSurface *m_surface;
  IDirectFBEventBuffer *m_eventBuffer;
  IDirectFBInputDevice *m_input;
  int m_width;
  int m_height;
  image_map m_image_cache;
  font_map m_font_cache;

 private:
  void init();
  void destroy();

 public:
  Renderer(int argc, char **argv);
  ~Renderer();
  bool initialized() { return m_initialized; }
  void exit() { m_exit = true; }
  int width() { return m_width; }
  int height() { return m_height; }
  void loop(EventListener *listener);
  void color(unsigned char r, unsigned char g, unsigned char b, unsigned char alpha);
  void rect(int x, int y, int w, int h);
  void line(int x1, int y1, int x2, int y2, bool blend = false);
  void image(int x, int y, const char *path, bool blend = false);
  void font(const char *path, int size = 32);
  void text(int x, int y, const char *str, int max_width = 0);
  void flip();
  void play(const char *file);
};

#endif
