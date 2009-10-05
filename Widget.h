#ifndef WIDGET_H
#define WIDGET_H

#include "Event.h"
#include "Box.h"

#define MAX_LABEL_LENGTH 256

class Widget : public EventListener
{
 protected:
  Box m_box;
  int m_screen_x;
  int m_screen_y;
  char m_label[MAX_LABEL_LENGTH];
  Widget *m_parent;
  class Application *m_app;
  bool m_dirty;

 private:
  void update_screen_xy();
  
 public:
  Widget(Widget *parent);
  void setDirty(bool flag) { m_dirty = flag; }
  bool dirty() { return m_dirty; }
  Application *application() { return m_app; }
  virtual bool handleEvent(Event &event);
  virtual bool handleIdle();
  virtual void paint();
  virtual void move(int x, int y);
  virtual void resize(int w, int h);
  virtual const Box &box();
  virtual void setBox(const Box &box);
  virtual const char *label();
  virtual void setLabel(const char *label);
};

#endif 
