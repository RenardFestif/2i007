/*
  Exo3.c
  A compiler avec	gcc -o Exo3 Exo3.c -lX11
*/

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>   
#include <unistd.h>


Display              *dpy;
Window                racine, principale;
int                   ecran;
GC                    ctx;
XEvent                evmt;
XSetWindowAttributes  attr;
int                   compteur = 0;


void PourExpose (XExposeEvent *evmt) {
}


void PourButtonPress (XButtonPressedEvent *evmt) {
  XClearWindow(dpy, principale);
}


void PourButtonRelease (XButtonReleasedEvent *evmt) {
  XClearWindow(dpy, principale);
}


void PourButtonRelease4 (XButtonReleasedEvent *evmt) {
  int intensite;

  compteur++;
  intensite = compteur * 20;
  intensite = intensite > 255 ? 255 : intensite;
  XSetForeground(dpy, ctx, intensite);
  XFillRectangle(dpy, principale, ctx, 0, 0, 300, 300);
}


void PourButtonRelease5 (XButtonReleasedEvent *evmt) {
  int intensite;

  compteur++;
  intensite = compteur * 20;
  intensite = (intensite > 255 ? 255 : intensite) << 16;
  XSetForeground(dpy, ctx, intensite);
  XFillRectangle(dpy, principale, ctx, 0, 0, 300, 300);
}


void Installer (char *serveur) {
  dpy = XOpenDisplay(serveur);          /* demande une connexion au serveur  */
  if (dpy == NULL) {
     printf("Connection impossoble avec le serveur %s.\n", serveur);
     exit(0);
  }

  racine     = DefaultRootWindow(dpy);  /* on recupere la racine             */
  ecran      = DefaultScreen(dpy);      /* ecran par defaut qui doit etre 0  */
  ctx        = XCreateGC(dpy, racine, 0, NULL);

  principale = XCreateSimpleWindow(dpy, /* demande de creation d'une fenetre */
		 racine,                 /*   comme mere racine              */
		 0, 0, 200, 200,         /* la geometrie                     */
		 0,                      /* epaisseur pour la bordure        */
		 BlackPixel(dpy, ecran), /* pixel (couleur pour la bordure)  */
		 BlackPixel(dpy, ecran));/* pixel (pour le fond)             */

  XSelectInput(dpy, principale,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask);


  XStoreName(dpy, principale, "Exemple 1");
  XMapWindow(dpy, principale);
}

int main (int argc, char *argv[]) {	/* la procedue main()                */
  Installer(NULL);
  for (;;) {				/* la boucle d'evenements            */
    if (!XCheckWindowEvent(dpy, principale, ButtonPressMask, (XEvent *)&evmt)) {
      compteur = 0;
      usleep(100000);
      XClearWindow(dpy, principale);
    }
    XNextEvent(dpy, &evmt);
    switch (evmt.type) {
    case Expose :
      PourExpose((XExposeEvent *)&evmt);
      break;
    case ButtonPress :
      PourButtonPress((XButtonPressedEvent *)&evmt);
      break;
    case ButtonRelease :
      switch (evmt.xbutton.button) {
      case 4 :
	PourButtonRelease4((XButtonReleasedEvent *)&evmt);
	break;
      case 5 :
	PourButtonRelease5((XButtonReleasedEvent *)&evmt);
	break;
      default :
	PourButtonRelease((XButtonReleasedEvent *)&evmt);
	break;
      }
      break;
    default :;
    }
  }
}


/*

1. le ButtonPressMask pour principale est obligatoire car sinon elle ne
   recevra jamais le ButtonRelease :

   Explication :
     - si on ne selectionne pas un ButtonPressMask dans principale,
     - si on enfonce un bouton dans principale, un evenement ButtonPress est
       emis.
     - comme principale n'est pas interessee, alors le serveur cherche la
       mere, puis la grande mere, etc, ... en remontant dans l'arbre des
       fenetres jusqu'a la racine pour trouver une fenetre interessee.
       En general, il y a toujours le gestionnaire de fenetre qui
       selectionne un tel masque sur ses fenetres de decoration et sur la
       fenetre racine.
     - le serveur trouve une fenetre interessee (qui n'est pas
       principale), il y a alors capture.
     - par la capture, cette fenetre sera alors source de tous les
       evenements concernant la souris et le clavier,
     - donc principale ne sera jamais source de ButtonRelease.

     - Dans notre situation, il est donc important de selectionner un
       ButtonPressMask sur principale pour que cette derniere puisse
       recevoir des ButtonRelease.


2. La fin de la roulette ou un arret intentionnel est indique par l'absence de
   l'evenemenr ButtonRelease dans la file des evenements.
   L'utilisation de XCheckWindowEvent() pour tester cela est important car
   elle n'est pas bloquant, contrairement a XNextEvent().

   L'utilisation de usleep(100000) pour permettre le temps du dernier affichage
   de couleur d'être vu. Sinon, c'est trop rapide, on ne verra que le dernier
   affichage, c'est à dire du noir.
*/
