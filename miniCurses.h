// Pour utiliser au minimum le mode curses d'une console shell
// 2017 : J.P. Chevallet
#ifndef miniCurses_h
#define miniCurses_h
#include <stdlib.h> // Pour la commande exit
#include <curses.h>

// Liste des couleurs de base : fond et caracteres de couleur identique.
typedef enum {none,black,red,green,yellow,blue,magenta,cyan,white} Color;

// Initialise le mode curses
void startCurses() {
    initscr(); // Debute le mode curses
    // Verifie que le terminal accepte la couleur
    if(has_colors() == false)
    {	endwin();
        printf("Votre terminal n'accepte pas la couleur ...\n");
        exit(1);
    }
    start_color(); // Démmare le mode avec couleur
    curs_set(0);   // rend le curseur d'affichage invisible
    noecho(); // N'affiche aucun echo dees caractères lus par getchar
    keypad(stdscr, TRUE); // Permet d'interpréter les caractères spéciaux comme un seul caractère
    // Voir ici la liste des caractères
    // https://www.gnu.org/software/guile-ncurses/manual/html_node/Getting-characters-from-the-keyboard.html

    // Construit la liste de 8 couleurs de base avec le nom de la couleur du fond
    // Le fond (background) est le nom de la couleur
    // les caractères ont la couleur complementaire
    init_pair(black,COLOR_WHITE,COLOR_BLACK);
    init_pair(red,COLOR_CYAN,COLOR_RED);
    init_pair(green,COLOR_MAGENTA,COLOR_GREEN);
    init_pair(yellow,COLOR_BLUE,COLOR_YELLOW);
    init_pair(blue,COLOR_YELLOW,COLOR_BLUE);
    init_pair(magenta,COLOR_GREEN,COLOR_MAGENTA);
    init_pair(cyan,COLOR_RED,COLOR_CYAN);
    init_pair(white,COLOR_BLACK,COLOR_WHITE);
    // Definit le fond
    bkgd(' '| COLOR_PAIR(black));
}

// Termine le mode curses
void stopCurses() {
    endwin();
}

// Dessine un caractère à la position x, et y et
void drawChar(int x,int y,int c,Color color) {
    // Vérifie que x et y sont dans les bornes de la fenêtre
    if (x < 0 || y < 0 || x >= COLS || y >= LINES) {
        endwin();
        printf("Erreur draw(%d,%d,%c,%d)",x,y,c,color);
        printf(": les coordonnées doivent être >= (0,0) et < (%d,%d)\n",COLS,LINES);
        exit(1);
    }
    // Deplace le curseur
    move(y,x);
    // Affiche un espace
    addch(c| COLOR_PAIR(color));
    // Annule l'usage de cet attribut pour ne pas interferer avec printw
    attroff(COLOR_PAIR(color));
}


// Dessine un point de couleur à la position x,y
void drawPoint(int x,int y,Color color) {
    // Vérifie que x et y sont dans les bornes de la fenêtre
    if (x < 0 || y < 0 || x >= COLS || y >= LINES) {
        endwin();
        printf("Erreur drawPoint(%d,%d;%d)",x,y,color);
        printf(": les coordonnées doivent être >= (0,0) et < (%d,%d)\n",COLS,LINES);
        exit(1);
    }
    // Deplace le curseur
    move(y,x);
    // Affiche un espace
    addch(' ' | COLOR_PAIR(color));
    // Annule l'usage de cet attribut pour ne pas interferer avec printw
    attroff(COLOR_PAIR(color));
    // Met à jour l'affichage
    refresh();
}

#endif
