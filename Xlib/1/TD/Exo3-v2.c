#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  int is_pressed;
  int button_used;
  int x;
  int y;
} MouseClick;

typedef struct {
  Display* display;
  Window main;
  Window inner_window;
  MouseClick click_on_inner;
} Application;

static const long black_pixel = 0x000000;
static const long white_pixel = 0xFFFFFF;

int min(int a, int b) {
  return (a > b) ? b : a;
}

int max(int a, int b) {
  return (a > b) ? a : b;
}

void events_loop(Application*);
Application make_app(char* serveur);
void exit_application(Application* app);

void on_screen_change(Application* app, XConfigureEvent* event);
void move_inner_window(Application* app, int x, int y);
int bounded_shift(int current, int delta, int max);
void on_button_pressed(Application* app, XButtonEvent* event);
void on_button_released(Application* app, XButtonEvent* event);
void on_mouse_moved(Application* app, XPointerMovedEvent* event);

int main(int argc, char* argv[])
{
  Application app = make_app(NULL);
  events_loop(&app);
  exit_application(&app);
}

void exit_application(Application* app)
{
  XCloseDisplay(app->display);
  exit(0);
}

void events_loop(Application* app)
{
  XEvent event;
  while(1)
  {
    XNextEvent(app->display, &event);
    switch (event.type) {
      case ButtonPress:
        on_button_pressed(app, (XButtonEvent*)&event);
        break;
      case MotionNotify:
        on_mouse_moved(app, (XPointerMovedEvent*)&event);
        break;
      case ButtonRelease:
        on_button_released(app, (XButtonEvent*)&event);
        break;
      case ConfigureNotify:
        on_screen_change(app, (XConfigureEvent*)&event);
        break;
    }
  }
}

Application make_app(char* serveur)
{
  Application app;
  app.display = XOpenDisplay(serveur);
  if(app.display == NULL) {
    printf("Error: couldn't connect to the server %s.\n", serveur);
    exit(0);
  }

  app.main = XCreateSimpleWindow(app.display,
    DefaultRootWindow(app.display),
    0, 0, 200, 200,
    0,
    black_pixel, white_pixel);
  XSelectInput(app.display, app.main, StructureNotifyMask);

  app.inner_window =  XCreateSimpleWindow(app.display,
    app.main,
    90, 90, 20, 20,
    0,
    black_pixel, black_pixel);
  XSelectInput(app.display, app.inner_window,
    ButtonPressMask|ButtonReleaseMask|ButtonMotionMask);

  app.click_on_inner.is_pressed = 0;

  XStoreName(app.display, app.main, "Exercise 3");
  XMapSubwindows(app.display, app.main);
  XMapWindow(app.display, app.main);

  return app;
}

void on_button_pressed(Application* app, XButtonEvent* event)
{
  if (event->window == app->inner_window &&
      app->click_on_inner.is_pressed == 0) {
    app->click_on_inner.is_pressed = 1;
    app->click_on_inner.button_used = event->button;
    app->click_on_inner.x = event->x;
    app->click_on_inner.y = event->y;
  }
}

void on_button_released(Application* app, XButtonEvent* event)
{
  if (app->click_on_inner.is_pressed == 1 &&
      app->click_on_inner.button_used == event->button) {
    app->click_on_inner.is_pressed = 0;
  }
}

void on_mouse_moved(Application* app, XPointerMovedEvent* event)
{
  if (app->click_on_inner.is_pressed == 1) {
    move_inner_window(app, event->x, event->y);
  }
}

void move_inner_window(Application* app, int x, int y)
{
  XWindowAttributes main_attr;
  XWindowAttributes inner;
  XGetWindowAttributes(app->display, app->main, &main_attr);
  XGetWindowAttributes(app->display, app->inner_window, &inner);

  int delta_x = x - app->click_on_inner.x;
  int delta_y = y - app->click_on_inner.y;

  int inner_x = bounded_shift(inner.x, delta_x, main_attr.width-inner.width);
  int inner_y = bounded_shift(inner.y, delta_y, main_attr.height-inner.height);

  XMoveWindow(app->display, app->inner_window,
    inner_x, inner_y);
}

// Shift current by delta while keeping the solution inside [0, upper_bound].
int bounded_shift(int current, int delta, int upper_bound)
{
  current = max(0, current + delta);
  return min(upper_bound, current);
}

void on_screen_change(Application* app, XConfigureEvent* event)
{
  XWindowAttributes main_attr;
  XWindowAttributes inner;
  XGetWindowAttributes(app->display, app->main, &main_attr);
  XGetWindowAttributes(app->display, app->inner_window, &inner);

  XWindowChanges attributes;

  attributes.width = event->width / 10.0;
  attributes.height = event->height / 10.0;
  attributes.x = inner.x * event->width / main_attr.width;
  attributes.y = inner.y * event->height / main_attr.height;

  XConfigureWindow(app->display,
       app->inner_window,
       CWX|CWY|CWWidth|CWHeight,
       &attributes);
}
