/* Exo2.c */

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

#define min(x, y)	((x) < (y) ? (x) : (y))
#define max(x, y)	((x) > (y) ? (x) : (y))
#define abs(x)		((x) > 0 ? (x) : -(x))

/*****

      xc, yc, lc, hc du curseur
      L, H de la principale

      (xc / (L - lc)) == (indice_curseur / max_indice)
      yc              == 0

      avec comme choix lc = hc = H

Pour dessiner un rectangle plein :

XFillRectangle(serveur,
               fenetre,
	       contexte_graphique,
	       x, y,
	       largeur, hauteur);

avec par exemple :

     - dpy			comme serveur
     - reglettre->fenetre	comme fenetre
     - DefaultGC(dpy, ecran)	comme contexte graphique (c'est l'outil pour
       		      		dessiner avec comme couleur par defaut le noir)

*****/

typedef struct _REGLETTE {
  Window fenetre;
  int    largeur, hauteur;
  int    indice_curseur;
  int    max_indice;
  int    couleur_curseur;
  int	 bouton;
  int    x_bouton, y_bouton;
  int    nb;			/* pour enregistrer le nombre de bouton deja
				   enfonce */
} *REGLETTE;


Display	      *dpy;
int	       ecran;
Window	       racine;
unsigned long  noir, blanc, rouge, vert, bleu;
XEvent	       evmt;
Window	       principale;
Window	       affichage;
REGLETTE       reglette_rouge, reglette_verte, reglette_bleue;
int            largeur, hauteur;


/* Les prototypes de fonctions */
void Installer	         (void);
REGLETTE CreerReglette	 (Display *dpy,
			  Window mere,
			  int x, int y,
			  int largeur, int hauteur,
			  int indice_curseur,
			  int max_indice,
			  int couleur);
void RaffraichirReglette (REGLETTE reglette);
void PourButtonPressReglette (REGLETTE reglette, XButtonEvent *evmt);
void PourMotionNotifyReglette (REGLETTE reglette, XPointerMovedEvent *evmt);
void PourButtonReleaseReglette (REGLETTE reglette, XButtonEvent *evmt);
void PourConfigureNotifyReglette (REGLETTE reglette, XConfigureEvent *evmt);
void PourExposeReglette (REGLETTE reglette, XExposeEvent *evmt);
void PourButtonPress     (XButtonEvent *evmt);
void PourMotionNotify    (XPointerMovedEvent *evmt);
void PourButtonRelease   (XButtonEvent *evmt);
void PourConfigureNotify (XConfigureEvent *evmt);
void PourExpose		 (XExposeEvent *evmt);
void RaffraichirAffichage ();


int main (int argc, char *argv[]) {	/* la procedue main()                */
  Installer();
  for (;;) {				/* la boucle d'evenements            */
      XNextEvent(dpy, &evmt);
      switch (evmt.type) {
        case ButtonPress :
	  PourButtonPress((XButtonEvent *)(&evmt));
	  break;
        case MotionNotify :
	  PourMotionNotify((XPointerMovedEvent *)(&evmt));
	  break;
        case ButtonRelease :
	  PourButtonRelease((XButtonEvent *)(&evmt));
	  break;
        case ConfigureNotify :
	  PourConfigureNotify((XConfigureEvent *)(&evmt));
	  break;
        case Expose :
	  PourExpose((XExposeEvent *)(&evmt));
        default :;
      }
  }
}


void Installer (void) {
  dpy    = XOpenDisplay(NULL);
  racine = DefaultRootWindow(dpy);
  ecran  = DefaultScreen(dpy);
  noir   = 0x0;
  blanc  = 0xFFFFFF;
  rouge  = 0xFF0000;
  vert   = 0x00FF00;
  bleu   = 0x0000FF;

  largeur = hauteur = 300;
  principale = XCreateSimpleWindow(dpy, racine, 0, 0, largeur, hauteur, 1,
				   noir, blanc);

  reglette_rouge = CreerReglette(dpy,
		     principale, 2, 2, largeur - 6, 20, 0, 255, rouge);
  reglette_verte = CreerReglette(dpy,
		     principale, 2, 24, largeur - 6, 20, 0, 255, vert);
  reglette_bleue = CreerReglette(dpy,
		     principale, 2, 46, largeur - 6, 20, 0, 255, bleu);
  XSelectInput(dpy, principale, StructureNotifyMask);

  affichage = XCreateSimpleWindow(dpy,
		principale, 2, 68, largeur - 6, hauteur - 70, 1, noir, noir);

  XStoreName(dpy, principale, "Exercice 3");

  XMapWindow(dpy, principale);
  XMapSubwindows(dpy, principale);
}


REGLETTE CreerReglette(Display *dpy,
		       Window mere,
		       int x, int y,
		       int largeur, int hauteur,
		       int indice_curseur,
		       int max_indice,
		       int couleur) {

  REGLETTE aux = (REGLETTE)(malloc(sizeof(struct _REGLETTE)));

  aux->fenetre = XCreateSimpleWindow(dpy, mere, x, y, largeur, hauteur, 1,
				     noir, couleur);
  aux->largeur         = largeur;
  aux->hauteur         = hauteur;
  aux->indice_curseur  = min(indice_curseur, max_indice);
  aux->max_indice      = max_indice;
  aux->couleur_curseur = couleur;
  aux->bouton          = aux->x_bouton = aux->y_bouton = -1;
  aux->nb              = 0;

  XSelectInput(dpy,
	       aux->fenetre,
	       ButtonPressMask|ButtonMotionMask|ButtonReleaseMask
	         |ExposureMask|StructureNotifyMask);
  return aux;
}


void RaffraichirReglette (REGLETTE reglette) {
  int xc, yc, lc, hc;			/* du curseur */

  lc = hc = reglette->hauteur;
  xc = reglette->indice_curseur * (reglette->largeur - lc)
         / reglette->max_indice;
  yc = 0;

  XClearWindow(dpy, reglette->fenetre);
  XFillRectangle(dpy, reglette->fenetre, DefaultGC(dpy, ecran),
		 xc, yc, lc, hc);
}


void PourButtonPressReglette (REGLETTE reglette, XButtonEvent *evmt) {
  if (!reglette->nb) {
    int xc, yc, lc, hc;			/* du curseur */

    lc = hc = reglette->hauteur;
    xc = reglette->indice_curseur * (reglette->largeur - lc)
           / reglette->max_indice;
    yc = 0;

    reglette->bouton = evmt->button;
    if ((evmt->x < xc) || (evmt->x > xc + lc) ||
	(evmt->y < yc) || (evmt->y > yc + hc)) {
      xc = max(0, evmt->x - lc / 2);
      xc = min(xc, reglette->largeur - lc);
      reglette->indice_curseur = xc * reglette->max_indice
	                           / (reglette->largeur - lc);
    }
    reglette->x_bouton = evmt->x - xc;
    reglette->y_bouton = evmt->y - yc;
  }
  reglette->nb++;
  RaffraichirReglette(reglette);
}


void PourMotionNotifyReglette (REGLETTE reglette, XPointerMovedEvent *evmt) {
  int xc, yc, lc, hc;

  if (reglette->x_bouton >= 0) {
    lc = hc = reglette->hauteur;
    xc = max(0, evmt->x - reglette->x_bouton);
    xc = min(xc, reglette->largeur - lc);
    yc = max(0, evmt->y - reglette->y_bouton);
    yc = min(yc, reglette->hauteur - hc);
    reglette->indice_curseur = xc * reglette->max_indice
	                         / (reglette->largeur - lc);
    RaffraichirReglette(reglette);
  }
}


void PourButtonReleaseReglette (REGLETTE reglette, XButtonEvent *evmt) {
  if (evmt->button == reglette->bouton) {
    int xc, yc, lc, hc;			/* du curseur */

    lc = hc = reglette->hauteur;
    xc = reglette->indice_curseur * (reglette->largeur - lc)
           / reglette->max_indice;
    yc = 0;

    reglette->bouton = evmt->button;
    if ((evmt->x < xc) || (evmt->x > xc + lc) ||
	(evmt->y < yc) || (evmt->y > yc + hc)) {
      xc = max(0, evmt->x - lc / 2);
      xc = min(xc, reglette->largeur - lc);
      reglette->indice_curseur = xc * reglette->max_indice
	                           / (reglette->largeur - lc);
    }
    reglette->x_bouton = evmt->x - xc;
    reglette->y_bouton = evmt->y - yc;
  }
  reglette->x_bouton = reglette->y_bouton = reglette->bouton = -1;
  reglette->nb--;
  RaffraichirReglette(reglette);
}


void PourConfigureNotifyReglette (REGLETTE reglette, XConfigureEvent *evmt) {
  reglette->largeur = evmt->width;
  reglette->hauteur = evmt->height;
  RaffraichirReglette(reglette);
}


void PourExposeReglette (REGLETTE reglette, XExposeEvent *evmt) {
  RaffraichirReglette(reglette);
}


void PourButtonPress (XButtonEvent *evmt) {
  if (evmt->window == reglette_rouge->fenetre) {
    PourButtonPressReglette(reglette_rouge, evmt);
  } else if (evmt->window == reglette_verte->fenetre) {
    PourButtonPressReglette(reglette_verte, evmt);
  } else if (evmt->window == reglette_bleue->fenetre) {
    PourButtonPressReglette(reglette_bleue, evmt);
  }
  RaffraichirAffichage();
}


void PourMotionNotify (XPointerMovedEvent *evmt) {
  if (evmt->window == reglette_rouge->fenetre) {
    PourMotionNotifyReglette(reglette_rouge, evmt);
  } else if (evmt->window == reglette_verte->fenetre) {
    PourMotionNotifyReglette(reglette_verte, evmt);
  } else if (evmt->window == reglette_bleue->fenetre) {
    PourMotionNotifyReglette(reglette_bleue, evmt);
  }
  RaffraichirAffichage();
}


void PourButtonRelease (XButtonEvent *evmt) {
  if (evmt->window == reglette_rouge->fenetre) {
    PourButtonReleaseReglette(reglette_rouge, evmt);
  } else if (evmt->window == reglette_verte->fenetre) {
    PourButtonReleaseReglette(reglette_verte, evmt);
  } else if (evmt->window == reglette_bleue->fenetre) {
    PourButtonReleaseReglette(reglette_bleue, evmt);
  }
  RaffraichirAffichage();
}


void PourConfigureNotify (XConfigureEvent *evmt) {
  largeur = evmt->width;
  hauteur = evmt->height;

  if (evmt->window == principale) {
    XResizeWindow(dpy, reglette_rouge->fenetre, largeur - 6, 20);
    XResizeWindow(dpy, reglette_verte->fenetre, largeur - 6, 20);
    XResizeWindow(dpy, reglette_bleue->fenetre, largeur - 6, 20);
    XResizeWindow(dpy, affichage, largeur - 6, hauteur - 70);
  } else if (evmt->window == reglette_rouge->fenetre) {
    PourConfigureNotifyReglette(reglette_rouge, evmt);
  } else if (evmt->window == reglette_verte->fenetre) {
    PourConfigureNotifyReglette(reglette_verte, evmt);
  } else if (evmt->window == reglette_bleue->fenetre) {
    PourConfigureNotifyReglette(reglette_bleue, evmt);
  }
  RaffraichirAffichage();
}


void PourExpose (XExposeEvent *evmt) {
  if (evmt->window == reglette_rouge->fenetre) {
    PourExposeReglette(reglette_rouge, evmt);
  } else if (evmt->window == reglette_verte->fenetre) {
    PourExposeReglette(reglette_verte, evmt);
  } else if (evmt->window == reglette_bleue->fenetre) {
    PourExposeReglette(reglette_bleue, evmt);
  }
  RaffraichirAffichage();
}


void RaffraichirAffichage () {
  XSetWindowBackground(dpy,
		       affichage,
		       (reglette_rouge->indice_curseur << 16) +
		         (reglette_verte->indice_curseur << 8) +
		           reglette_bleue->indice_curseur);
  XClearWindow(dpy, affichage);
}
