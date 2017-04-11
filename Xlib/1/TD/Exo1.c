/* Exo1.c */

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>


Display	      *dpy;			/* le serveur			     */
int	       ecran;			/* son ecran par defaut		     */
Window	       racine, principale;
Window	       fille_rouge, fille_noire, fille;
unsigned long  noire, rouge, blanche;	/* les differents pixels de couleur  */
XEvent	       evmt;
Time	       moment_buttonpress;	/* Time == unsigned long en mili-sec */
unsigned int   bouton_presse;



/* Les prototypes de fonctions */
void Installer	       (void);
void PourButtonPress   (XButtonEvent *evmt);
void PourButtonRelease (XButtonEvent *evmt);


int main (int argc, char *argv[]) {	/* la procedue main()                */
  Installer();
  for (;;) {				/* la boucle d'evenements            */
      XNextEvent(dpy, &evmt);
      switch (evmt.type) {
        case ButtonPress :
	  PourButtonPress((XButtonEvent *)(&evmt));
	  break;
        case ButtonRelease :
	  PourButtonRelease((XButtonEvent *)(&evmt));
	  break;
        default :;
      }
  }
}


void Installer (void) {
  dpy    = XOpenDisplay(NULL);
  racine = DefaultRootWindow(dpy);
  ecran  = DefaultScreen(dpy);

  rouge   = 0xFF0000;	/* ne fonctionne que si le serveur est en TrueColor  */
  noire   = 0x0;	/*    ou DirectColor. Sinon, il faut s'adresser a la */
  blanche = 0xFFFFFF;	/*    table de couleur Colormap du serveur           */

  moment_buttonpress = 0;
  bouton_presse	     = 0;

  principale  = XCreateSimpleWindow(dpy,
		  racine,
                  0, 0, 115, 60,
                  0,
                  noire, blanche);
  fille_rouge = XCreateSimpleWindow(dpy,
                  principale,
                  5, 5, 50, 50,
                  1,
                  noire, rouge);
  fille_noire = XCreateSimpleWindow(dpy,
                  principale,
                  55, 5, 50, 50,
                  1,
                  noire, noire);


  XSelectInput(dpy, principale, ButtonPressMask);
  XSelectInput(dpy, fille_rouge, ButtonPressMask|ButtonReleaseMask);
  XSelectInput(dpy, fille_noire, ButtonPressMask|ButtonReleaseMask);

  XStoreName(dpy, principale, "Exercice 1");
  XMapWindow(dpy, fille_rouge);
  XMapWindow(dpy, principale);
}


void PourButtonPress (XButtonEvent *evmt) {
  if (evmt->window == principale) {
     XCloseDisplay(dpy);
     exit(0);
  } if (!bouton_presse) {
    bouton_presse      = evmt->button;
    fille	       = evmt->window;
    moment_buttonpress = evmt->time;
  }
}


void PourButtonRelease (XButtonEvent *evmt) {

  if (evmt->button != bouton_presse || !bouton_presse) {
    return;
  } else {
    Time duree = evmt->time - moment_buttonpress;

    if (evmt->x < 0 || evmt->x > 50
	  || evmt->y < 0 || evmt->y > 50
	  || duree > 3000) {
    } else if (evmt->window == fille_rouge) {
      XUnmapWindow(dpy, fille_rouge);
      XMapWindow(dpy, fille_noire);
    } else {
      XUnmapWindow(dpy, fille_noire);
      XMapWindow(dpy, fille_rouge);
    }
    moment_buttonpress = 0;
    bouton_presse = 0;
  }
}
