#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  Display* display;
  Window main;
  Window red_square;
  Window black_square;
  Window current_visible_square;
  Time last_pressed;
} Application;

static const long black_pixel = 0x000000;
static const long white_pixel = 0xFFFFFF;
static const long red_pixel = 0xFF0000;
static const long square_size = 50;

void events_loop(Application* app);
Application make_app(char* serveur);
Window init_sub_window(Application* app, int offset, long background_color);
void on_button_pressed(Application* app, XButtonEvent *event);
void on_button_released(Application* app, XButtonEvent* event);
void exit_application(Application* app);

int main(int argc, char* argv[])
{
  Application app = make_app(NULL);
  events_loop(&app);
  exit_application(&app);
}

void events_loop(Application* app)
{
  XEvent event;
  while(1) {
    XNextEvent(app->display, &event);
    switch (event.type) {
      case ButtonPress:
        on_button_pressed(app, (XButtonEvent*)(&event));
        break;
      case ButtonRelease:
        on_button_released(app, (XButtonEvent*)(&event));
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
    0, 0, 115, 60,
    0,
    black_pixel, white_pixel);
  XSelectInput(app.display, app.main, ButtonPressMask);

  app.red_square = init_sub_window(&app, 5, red_pixel);
  app.black_square = init_sub_window(&app, 55, black_pixel);

  app.current_visible_square = app.red_square;
  app.last_pressed = 0;
  XStoreName(app.display, app.main, "Exercise 1");
  XMapWindow(app.display, app.main);
  XMapWindow(app.display, app.red_square);
  return app;
}

Window init_sub_window(Application* app, int x, long background_color)
{
  Window square = XCreateSimpleWindow(app->display,
    app->main,
    x, 5, square_size, square_size,
    0,
    black_pixel, background_color);
  XSelectInput(app->display, square, ButtonPressMask|ButtonReleaseMask);
  return square;
}

void exit_application(Application* app)
{
  XCloseDisplay(app->display);
  exit(0);
}

void on_button_pressed(Application* app, XButtonEvent *event)
{
  if (event->window == app->main) {
    exit_application(app);
  }
  if(event->window == app->current_visible_square) {
    app->last_pressed = event->time;
  }
}

int is_inside_square(XButtonEvent* event)
{
  return event->x >= 0 && event->x <= square_size
      && event->y >= 0 && event->y <= square_size;
}

int less_than_3_seconds(Application* app, Time released)
{
  return released - app->last_pressed < 3000;
}

void switch_windows(Application* app, Window current, Window new)
{
  XUnmapWindow(app->display, current);
  XMapWindow(app->display, new);
  app->current_visible_square = new;
}

void on_button_released(Application* app, XButtonEvent* event)
{
  if(event->window == app->current_visible_square &&
     is_inside_square(event) &&
     less_than_3_seconds(app, event->time))
  {
    if(app->current_visible_square == app->red_square) {
      switch_windows(app, app->red_square, app->black_square);
    } else {
      switch_windows(app, app->black_square, app->red_square);
    }
  }
}
