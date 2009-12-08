#include "Application.h"

Application::Application(Renderer *renderer, Audio *audio, Database *database)
  : m_renderer(renderer),
    m_audio(audio),
    m_db(database)
{
  Renderer *r = renderer;
  r->color(0, 0, 0, 0xff);
  r->rect(0, 0, r->width(), r->height());
  r->color(0xff, 0xff, 0xff, 0xff);
  r->font("data/bold_small.ttf", 42);
  r->text(640, 378, APP_NAME, 0, JUSTIFY_CENTER);
  r->color(0xcc, 0xcc, 0xcc, 0xff);
  r->font("data/regular_small.ttf", 18);
  r->text(640, 420, "loading...", 0, JUSTIFY_CENTER);
  r->flip();  
  if (r->getScale() == 1.5) {
    r->loadImage("data/menuitem_bg.png", 1, "data/menuitem_bg_1.5.png");
    r->loadImage("data/menuitem_bg_fade.png", 1, "data/menuitem_bg_fade_1.5.png");
  }
  else {
    r->loadImage("data/menuitem_bg.png");
    r->loadImage("data/menuitem_bg_fade.png");
  }
  r->loadImage("data/arrow.png");
  r->loadImage("data/arrow_on.png");
  r->loadImage("data/fade_bot.png");
  r->loadImage("data/fade_top.png");
  r->loadImage("data/position_bar.png");
  r->loadImage("data/position_knob.png");
  r->loadImage("data/movies.png", 2.0);
  r->loadImage("data/tvshows.png");
  r->loadImage("data/music.png");
  r->loadImage("data/settings.png");
  r->loadImage("data/unknown_movie.png");
  r->loadImage("data/unknown_album.png");
  r->color(0, 0, 0, 0xff);
  r->rect(0, 0, r->width(), r->height());
  r->color(0xff, 0xff, 0xff, 0xff);
  r->font("data/bold_small.ttf", 42);
  r->text(640, 378, APP_NAME, 0, JUSTIFY_CENTER);
  r->color(0xcc, 0xcc, 0xcc, 0xff);
  r->font("data/regular_small.ttf", 18);
  r->text(640, 420, "initializing...", 0, JUSTIFY_CENTER);
  r->flip();  
  r->font(BOLD_FONT, 18);
  r->font(BOLD_FONT, 23);
  r->font(BOLD_FONT, 29);
  r->font(BOLD_FONT, 37);
  r->font(REGULAR_FONT, 18);
  r->font(REGULAR_FONT, 23);
  r->font(REGULAR_FONT, 29);
  // m_audio->open("data/start.mp3", "", "", "", "");

  r->color(0x0, 0x0, 0x0, 0xff);
  r->rect(0, 0, r->width(), r->height());
  r->flip();  
  r->rect(0, 0, r->width(), r->height());
  r->flip();    
}

void Application::setScreen(Screen *screen) 
{
  while (m_stack.top()) {
    m_stack.pop();
    m_stack.cleanUp();
  }
  m_stack.push(screen);
  if (m_renderer->initialized()) {
    screen->setDirty();
    screen->paint();
    m_renderer->flip();
    screen->paint();
    m_renderer->flip();
  }
}

void Application::run()
{
  if (m_renderer->initialized()) {
    m_renderer->loop(this);
  }
}

void Application::exit()
{
  if (m_renderer->initialized()) {
    m_renderer->exit();
  }
}

void Application::go(Screen *screen) 
{ 
  m_stack.push(screen); 
  if (m_renderer->initialized()) {
    screen->setDirty();
    screen->paint();
    m_renderer->flip();
    screen->paint();
    m_renderer->flip();
  }
}

void Application::back() 
{ 
  if (m_stack.size() > 1) {
    m_stack.pop(); 
    if (m_renderer->initialized()) {
      Screen *screen = m_stack.top();
      screen->setDirty();
      screen->paint();
      m_renderer->flip();
      screen->paint();
      m_renderer->flip();
    }
  }
}

bool Application::handleEvent(Event &event)
{
  m_stack.cleanUp();
  
  switch (event.key) {
  case KEY_BACK:
    m_audio->playSound("data/exit.pcm"); 
    back();
    break;
  }

  Screen *screen = m_stack.top();

  if (screen) {
    if (!screen->handleEvent(event)) back();
    screen = m_stack.top();
    screen->paint();
    m_renderer->flip();
  }

  return true;
}

bool Application::handleIdle()
{
  m_stack.cleanUp();

  Screen *screen = m_stack.top();

  if (screen) {
    if (screen->handleIdle()) {
      screen = m_stack.top();
      screen->paint();
      m_renderer->flip();
    }
  }

  return true;
}

Stack::Stack()
{
  m_top = -1;

  for (int i=0; i<MAX_STACK_SIZE; i++) {
    m_screens[i] = 0;
  }
}

Stack::~Stack()
{
  m_top = -1;
  cleanUp();
}

void Stack::cleanUp()
{
  for (int i=m_top+1; i<MAX_STACK_SIZE; i++) {
    if (m_screens[i]) {
      delete m_screens[i];
      m_screens[i] = 0;
    }
  }
}

bool Stack::push(Screen *screen)
{
  screen->setDirty();
  if (m_top < MAX_STACK_SIZE-1) {
    if (m_top > -1) m_screens[m_top]->setDirty();
    m_screens[++m_top] = screen;
    return true;
  }
  return false;
}

Screen *Stack::pop() 
{
  if (m_top >= 0) {
    return m_screens[m_top--];
  }
  return NULL;
}

Screen *Stack::top() {
  if (m_top > -1) {
    return m_screens[m_top];
  }
  return NULL;
}

