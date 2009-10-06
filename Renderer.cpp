#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Renderer.h"
#include "Utils.h"

Renderer::Renderer(int argc, char **argv)
  : m_initialized(false),
    m_exit(false)
{
  if (DirectFBInit(&argc, &argv) != DFB_OK) {
    fprintf(stderr, "Error in DirectFBInit!\n");
  }
  else {
    init();
  }
}

Renderer::~Renderer()
{
  if (m_initialized) destroy();
}

void Renderer::init()
{
  if (m_initialized) return;

  DFBSurfaceDescription dsc;
  
  if (DirectFBCreate (&m_dfb) != DFB_OK) {
    fprintf(stderr, "Error in DirectFBCreate!\n"); return;
  }

  if (m_dfb->SetCooperativeLevel(m_dfb, DFSCL_EXCLUSIVE) != DFB_OK) {
    fprintf(stderr, "Error in SetCooperativeLevel!\n"); return;
  }
  
  dsc.flags = DSDESC_CAPS;
  dsc.caps  = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY | DSCAPS_FLIPPING);
  
  if (m_dfb->CreateSurface( m_dfb, &dsc, &m_surface ) != DFB_OK) {
    fprintf(stderr, "Error in CreateSurface!\n"); return;
  }
  
  if (m_surface->GetSize (m_surface, &m_width, &m_height) != DFB_OK) {
    fprintf(stderr, "Error in GetSize!\n"); return;
  }

  if (m_surface->Flip (m_surface, NULL, DSFLIP_NONE) != DFB_OK) {
    fprintf(stderr, "Error in Flip!\n"); return;
  }
  if (m_dfb->GetInputDevice (m_dfb, INPUT_DEVICE, &m_input) != DFB_OK) {
    fprintf(stderr, "Error in GetInputDevice!\n"); return;    
  }
  
  if (m_input->CreateEventBuffer (m_input, &m_eventBuffer) != DFB_OK) {
    fprintf(stderr, "Error in CreateEventBuffer!\n"); return;    
  }

  color(0,0,0,0xff);
  rect(0,0,m_width, m_height);

  m_exit = false;
  m_initialized = true;
}

void Renderer::destroy()
{
  if (!m_initialized) return;

  for (image_map::const_iterator i=m_image_cache.begin(); i != m_image_cache.end(); i++) {
    if (i->second) i->second->Release(i->second);    
  }
  m_image_cache.clear();

  for (font_map::const_iterator i=m_font_cache.begin(); i != m_font_cache.end(); i++) {
    if (i->second) i->second->Release(i->second);    
  }
  m_font_cache.clear();

  m_input->Release(m_input);
  m_surface->Release(m_surface);
  m_dfb->Release (m_dfb);

  m_initialized = false;
}

void Renderer::loop(EventListener *listener)
{
  DFBInputEvent dfb_event;
  Event event;

  while (!m_exit) {
    while (!m_exit && m_eventBuffer->GetEvent (m_eventBuffer, DFB_EVENT(&dfb_event)) == DFB_OK) {
      if (dfb_event.type == DIET_KEYPRESS) {
	event.type = EVENT_KEYPRESS;
	event.key = (Key)dfb_event.key_symbol;
	debug("got key: 0x%x 0x%x\n", (int)(event.key & 0xFF00), (int)(event.key & 0xFF));
	if (!listener->handleEvent(event)) m_exit = true;
      }
    }
    if (!listener->handleIdle()) m_exit = true;
  }    
}
 
void Renderer::color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  m_surface->SetColor(m_surface, r & 0xff, g & 0xff, b & 0xff, a & 0xff);
}

void Renderer::rect(int x, int y, int w, int h)
{
  m_surface->FillRectangle(m_surface, x, y, w, h);
}

void Renderer::line(int x1, int y1, int x2, int y2, bool blend)
{
  if (blend) m_surface->SetDrawingFlags(m_surface, DSDRAW_BLEND);
  m_surface->DrawLine(m_surface, x1, y1, x2, y2);
  if (blend) m_surface->SetDrawingFlags(m_surface, DSDRAW_NOFX);
}

void Renderer::image(int x, int y, const char *path, bool blend) 
{
  debug("drawing: %s\n", path);

  IDirectFBSurface *image = NULL;
  DFBSurfaceDescription sdsc;
  DFBImageDescription idsc;

  if (m_image_cache.find(path) == m_image_cache.end()) {
    IDirectFBImageProvider *provider = NULL;
    if (m_dfb->CreateImageProvider (m_dfb, path, &provider) == DFB_OK) {
      if (provider->GetSurfaceDescription(provider, &sdsc) == DFB_OK &&
	  provider->GetImageDescription(provider, &idsc) == DFB_OK) {
	if (m_dfb->CreateSurface(m_dfb, &sdsc, &image) == DFB_OK) {
	  provider->RenderTo(provider, image, NULL);
	}
	else {
	  debug("CreateSurface failed\n");
	}
      }
      else {
	debug("GetSurfaceDescription failed\n");
      }
      if (provider) provider->Release(provider);
    }
    else {
      debug("CreateImageProvider failed\n");
    }
    m_image_cache[path] = image;
  }
  image = m_image_cache[path];
  if (image) {
    if (blend) m_surface->SetBlittingFlags(m_surface, DSBLIT_BLEND_ALPHACHANNEL);
    m_surface->Blit(m_surface, image, NULL, x, y);
    if (blend) m_surface->SetBlittingFlags(m_surface, DSBLIT_NOFX);
  }
}

void Renderer::font(const char *path, int size)
{
  if (!path || !path[0]) return;

  DFBFontDescription dsc;
  IDirectFBFont *font = NULL;
  unsigned key = hash(path) + size;

  if (m_font_cache.find(key) == m_font_cache.end()) {
    dsc.flags = DFDESC_HEIGHT;
    dsc.height = size;
    m_dfb->CreateFont(m_dfb, path, &dsc, &font);
    m_font_cache[key] = font;    
  }
  font = m_font_cache[key];
  if (font)
    m_surface->SetFont(m_surface, font);
}

void Renderer::text(int x, int y, const char *str, int max_width)
{
  DFBRegion clip, textclip;

  if (max_width) {
    m_surface->GetClip(m_surface, &clip);
    textclip.x1 = clip.x1;
    textclip.y1 = clip.y1;
    textclip.x2 = x + max_width;
    textclip.y2 = clip.y2;
    m_surface->SetClip(m_surface, &textclip);
  }
  m_surface->DrawString(m_surface, str, -1, (int)x, (int)y, DSTF_LEFT);
  if (max_width) {
    m_surface->SetClip(m_surface, &clip);
  }
}

void Renderer::flip()
{
  m_surface->Flip(m_surface, NULL, DSFLIP_WAITFORSYNC);
}

void Renderer::play(const char *file)
{
  if (!file || !file[0]) return;

  int status;
  pid_t pid;

  destroy();
  //m_dfb->Suspend(m_dfb);
  if ((pid = fork()) == -1)
    perror("couldn't fork");
  else if (pid == 0)
    execl("/bin/mono", "/bin/mono", "-single", "-nogui", "-dram", "1", file, NULL);
  else if ((pid = wait(&status)) == -1)
    perror("wait error");
  init();
  //m_dfb->Resume(m_dfb);
}
