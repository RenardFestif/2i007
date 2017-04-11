#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  Display* display;
  Window root;
  int buffer_read_count;
} Application;

void events_loop(Application*);
Application make_app(char* serveur);
void exit_application(Application* app);
void write_koukou_if_10_reads(Application* app);
void read_xcut_buffer0(Application* app, XPropertyEvent* event);

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
      case PropertyNotify:
        read_xcut_buffer0(app, (XPropertyEvent*)&event);
        write_koukou_if_10_reads(app);
        break;
    }
  }
}

void read_xcut_buffer0(Application* app, XPropertyEvent* event)
{
  if(event->atom == XA_CUT_BUFFER0) {
    XTextProperty cut_text;
    XGetTextProperty(app->display, app->root,
      &cut_text, XA_CUT_BUFFER0);
    printf("%.*s\n", cut_text.nitems, cut_text.value);
    app->buffer_read_count += 1;
  }
}

void write_koukou_if_10_reads(Application* app)
{
  if(app->buffer_read_count == 10) {
    app->buffer_read_count = 0;
    XChangeProperty(app->display, app->root,
      XA_CUT_BUFFER0, XA_STRING, 8,
      PropModeReplace,
      "KOUKOU", 6);
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

  app.buffer_read_count = 0;
  app.root = DefaultRootWindow(app.display);
  XSelectInput(app.display, app.root, PropertyChangeMask);
  return app;
}
