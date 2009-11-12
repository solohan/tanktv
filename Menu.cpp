#include <string.h>
#include "Menu.h"
#include "Utils.h"

Menu::Menu(Application *application, const char *title)
  : Screen(application),
    m_size(0),
    m_current(-1),
    m_top(180),
    m_refreshed(false)
{  
  setLabel(title);
}

Menu::~Menu()
{
  for (int i; i<m_size; i++) {
    delete m_menuitems[i];
  }
}

void Menu::add(MenuItem *menuItem)
{
  if (m_size < MAX_MENU_ITEMS) {
    menuItem->m_index = m_size;
    m_menuitems[m_size++] = menuItem;
  }
  if (m_size == 1) m_current = 0;
}

bool Menu::handleEvent(Event &event)
{
  debug("in Menu::handleEvent: %d\n", event.key);
  switch (event.key) {
  case KEY_UP:
    if (m_current > 0) {
      m_current--; 
      setDirty(true);
    }
    break;
  case KEY_DOWN:
    if (m_current < m_size - 1) {
      m_current++;
      setDirty(true);
    }
    break;
  case KEY_ENTER:
    if (m_current > -1) {
      m_menuitems[m_current]->select();
      setDirty(true);
    }
    break;
  }
  m_refreshed = false;
  return true;
}

bool Menu::handleIdle()
{
  Renderer *r = m_app->renderer();
  if (m_current > -1) {
    MenuItem *mi = m_menuitems[m_current];
    if (mi->dirty()) {
      if (!m_refreshed) {
	paint();
	m_refreshed = true;
      }
      paintBackground();
      mi->paint();
      r->flip();
    }
  }
  return true;
}

void Menu::paintBackground()
{
  if (m_current > -1) {
    int start = 0;
    int x = MENU_X, height = m_menuitems[0]->box().h;
    Renderer *r = m_app->renderer();

    if (m_current > m_size - 5) {
      start = max(0, m_size - 9);
    }
    else if (m_current > 4) {
      start = m_current - 4;
    }    
    r->image(x - 32, m_top + (m_current - start) * height - 8, "images/menuitem_bg.png");  
    r->font(BOLD_FONT, 29);
  }
}

void Menu::paint()
{
  if (m_current > -1) {
    int start = 0, end = 0;
    int x = MENU_X, y, height = m_menuitems[0]->box().h;
    Renderer *r = m_app->renderer();

    r->color(0, 0, 0, 0xff);
    r->rect(0, 0, r->width(), r->height());

    if (m_current > m_size - 5) {
      start = max(0, m_size - 9);
    }
    else if (m_current > 4) {
      start = m_current - 4;
    }
    end = min(m_size-1, start + 9);
    
    r->font(BOLD_FONT, 37);
    r->color(0xff, 0xff, 0xff, 0xff);
    r->text(x + 222, m_top - 40, m_label, 445, JUSTIFY_CENTER);
    
    paintBackground();

    for (int i=start; i<=end; i++) {
      MenuItem *mi = m_menuitems[i];
      y = (i - start) * height;
      mi->move(x, m_top + y);
      mi->paint();
      if (i-start == 0 && start > 0) {
	r->image(x - 32, m_top - 18, "images/fade_top.png", true);  
      }
      if (i-start == 9) {
	r->image(x - 32, m_top + y - 18, "images/fade_bot.png", true);  
      }
    }

    r->flip();
  }
}
