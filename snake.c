
#include <stdbool.h>
#include <assert.h>
#include "miniCurses.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////
// Partie définition et gestion des pixels seuls
//////////////////////////////////////////////////////////////////////////

// Un pixel de l'image
typedef struct {
	int x;
	int y;
} Pixel;

// Crée un nouveau pixel à partir de 2 coordonnées
Pixel pix_new(int x, int y) {
	Pixel p;
	p.x = x;
	p.y = y;
	return p;
}

// Vrai si p est dans l'espace de jeux
bool is_pix_in_game(Pixel p) {
	return p.x >= 0 && p.x < COLS && p.y >= 0 && p.y < LINES;
}

// Definition d'un pixel qui sert de valeur sentinelle
Pixel pix_end() {
	return pix_new(-1,-1);
}

// Predicat de comparaison entre deux pixel
// Vrai si les deux pixels p1 et p2 sont égaux
bool pix_equal(Pixel p1,Pixel p2) {
	return (p1.x==p2.x)&&(p1.y==p2.y);
}

// Vrai si p est le pixel sentinelle
bool is_pix_end(Pixel p) {
	return pix_equal(p,pix_end());
}


//////////////////////////////////////////////////////////////////////////
// Partie définition et gestion d'une chaîne de pixels
//////////////////////////////////////////////////////////////////////////

// Note : dans une chaine terminé par une sentinelle,
// si len est la longeur de la chaine, alors la sentinelle
// se trouve à l'indice len du tableau donc le dernier elements
// si len != 0 est à la position len - 1

// Vide la chaine de pixels
// t: un tableau de pixel qui contient une chaine terminé par la sentinelle
// En sortie: t[0] est la sentinelle
void pix_clear(Pixel t[]) {
	t[0] = pix_end();
}

// Calcule la longueur de la chaîne de pixels
// t: un tableau de pixel qui contient une chaine terminé par la sentinelle
size_t pix_len(Pixel t[]) {
	size_t i = 0;
	Pixel p_end = pix_end();
	while (!pix_equal(t[i],p_end))
		i++;
	//t[i] == pix_end()
	return i;
}

// Ajoute le pixel p à la fin de la chaîne de pixels de t
// t: un tableau de pixel terminé par la sentinelle
// size: le nombre de cases de t
// p: le pixel à ajouter à la fin de la chaine
void pix_push_back(Pixel t[],size_t size, Pixel p) {
	// Le tableau doit avoir au moins 2 cases
	assert(size >= 2);
	// Calcule la longueur de la chaîne
	size_t len = pix_len(t);
	// Vérifie qu'il est possible d'ajouter un élément
	assert(len + 1 < size);
	// Ajoute l'élément à la fin
	t[len] = p;
	t[len+1] = pix_end();
}

// Enlève le pixel de la fin de la chaîne de pixels de t
// La longueur est réduite de 1.
// t: un tableau de pixel terminé par la sentinelle
// return:  le pixel enlevé de la size_t pix_find(Pixel t[],Pixel p) {fin de la chaine
Pixel pix_pop_back(Pixel t[]) {
	// Calcule la longueur de la chaîne
  	size_t len = pix_len(t);
	// Vérifie qu'il est possible d'enlever un élément
	assert(len >= 1);
	Pixel p_return = t[len-1];
	t[len-1] = pix_end();
	return p_return;
}

// Ajoute le pixel p au début de la chaîne de pixels de t
// t: un tableau de pixel terminé par la sentinelle
// size: le nombre de cases de t
// p: le pixel à ajouter au début de la chaine
void pix_push(Pixel t[],size_t size, Pixel p) {
	// Le tableau doit avoir au moins 2 cases
	assert(size >= 2);
	// Calcule la longueur de la chaîne
	size_t len = pix_len(t);
	// Vérifie qu'il est possible d'ajouter un élément
	assert(len + 1 < size);
	// Décale tous les elements d'une case vers la droite
	// laisse la première case libre, puis ajoute l'élément au début
	size_t i = 0;
	while (i <= len){
		t[len-i+1] = t[len-i];
		i++;
	}//i=len i.e. tous les pixels bien décalés d'un indice, sentinelle comprise
	t[0] = p;
}

// Valeur qui indique une valeur non trouvée
const size_t NOT_FOUND = (size_t)-1;

// Recherche la position d'un pixel p dans la chaine
// t: un tableau de pixel terminé par la sentinelle
// p: le pixel à rechercher
// return: la position du pixel ou NOT_FOUND s'il n'est pas trouvé
size_t pix_find(Pixel t[],Pixel p) {
	size_t len = pix_len(t);
	size_t i = 0;
	while ((i <= len)&&!(pix_equal(t[i],p)))
		i++;
	// t[i]==p (pixel trouvé) ou i == len (arrivé à la fin du tableau)
	if(!pix_equal(t[i],p))
		return NOT_FOUND;
	return i;
}

//////////////////////////////////////////////////////////////////////////
// Partie définition et gestion des obstacles
//////////////////////////////////////////////////////////////////////////

// Initialise un ensemble d'obstacles
// obst : le tableau des obstacles
// size : la taille du tableau obst
// len : le nombre d'obstacles à créer dans obst (lenght)
void obst_init(Pixel obst[], size_t size, size_t len) {
	// Initialise la fonction hasard (random)
	srand (time(NULL));
	// on ne peut pas créer plus d'obstacles que la taille du tableau
	assert(len < size);
	// clear le tableau obstacle
	pix_clear(obst);
	size_t i = 0;
	Pixel p_rand;
	// Génère un pixel de position aléatoire et le stock s'il n'existe pas déjà dans le tableau
	while (i < len) {
		p_rand = pix_new(rand()%(COLS), rand()%(LINES));
		if ((pix_find(obst, p_rand) == NOT_FOUND)){
			pix_push_back(obst, size, p_rand);
			i++;
		}
	}// i == len ie tous les obstacles voulus ont été créer
}

// Affiche les obstacles en rouge
void obst_draw(Pixel obst[]) {
	for(size_t i = 0; ! is_pix_end(obst[i]); i++)
		drawChar(obst[i].x, obst[i].y,'#',red);
}


//////////////////////////////////////////////////////////////////////////
// Partie définition et gestion du serpent (snake)
//////////////////////////////////////////////////////////////////////////

// direction de mouvement du serpent
typedef enum {versLeHaut,versLeBas,versLaGauche,versLaDroite} Direction;

// Initialise le serpent au centre de la fenêtre avec une longeur len=5
void snake_init(Pixel snake[], size_t size, size_t len, int numJoueur) {
	// Vérification que la taille du tableau est suffisante
	assert(len < size);
	// Le serpent doit avoir une taille de 2 minimum
	assert(len >= 2);
	// initialisation du corps du serpent au milieu du jeux
	pix_clear(snake);
	size_t i = 0;
	for(i=0; i< len; i++)
		pix_push_back(snake,size,pix_new(COLS/2+i,LINES/2+numJoueur));
}

// Affiche le serpent
void snake_draw(Pixel snake[]) {
	// Dessine la tête
	drawChar(snake[0].x, snake[0].y,ACS_DIAMOND|A_ALTCHARSET,green);
	// Dessine le reste
	for(size_t i=1; ! is_pix_end(snake[i]); i++)
		drawChar(snake[i].x, snake[i].y,ACS_CKBOARD|A_ALTCHARSET,green);
}

// Etat du jeux
typedef enum {
	GAME_RUNING,  // Le jeux se poursuit normalement
	GAME_STOPPED, // Le jeux est arrété à la demande de l'utilisateur (touche 's')
	EXIT_SPACE,   // Le serpent est sortis de l'aire du jeux
	TOUCH_OBST,   // Le serpent a touché un obstacle
	TOUCH_SNAKE,  // Le serpent a mordu sa queue
	LEN_MAX       // Le serpent a atteind sa taille adulte (maximale)
} GameStatus;

// Fait avancer le serpent d'une case et affiche le changement
// Retourne l'état du jeux apres le mouvement
GameStatus snake_move(Pixel snake1[], Pixel snake2[], size_t size,Pixel obst[],Direction direction1,Direction direction2,bool grow, char nbr_joueur) {
	// Test si la nouvelle position est valide
	Pixel new_snake_head = snake1[0];
	switch (direction1)
	{
		case versLeHaut: new_snake_head.y--; break;
		case versLeBas: new_snake_head.y++; break;
		case versLaGauche: new_snake_head.x--; break;
		case versLaDroite: new_snake_head.x++; break;
	}
	if(!is_pix_in_game(new_snake_head))
		return EXIT_SPACE;
	else if(pix_find(obst, new_snake_head) != NOT_FOUND)
		return TOUCH_OBST;
	if(pix_find(snake1, new_snake_head) != NOT_FOUND || pix_find(snake2, new_snake_head) != NOT_FOUND )
		return TOUCH_SNAKE;
	if((grow)&&(pix_len(snake1) + 1 == size))
		return LEN_MAX;
	else if (grow)// augmente la len de snake
		snake1[pix_len(snake1)] = pix_end();
	if(!grow) {// efface la queue de snake
		Pixel pix_a_effacer = pix_pop_back(snake1);
		drawPoint(pix_a_effacer.x, pix_a_effacer.y, black);
	}
	// Actualisation de la chaine de pixel snake2
	pix_push(snake1, size, new_snake_head);
	snake_draw(snake1);
	new_snake_head = snake2[0];
	if(nbr_joueur == '2') {
		// Test si la nouvelle position est valide
		switch (direction2)
		{
			case versLeHaut: new_snake_head.y--; break;
			case versLeBas: new_snake_head.y++; break;
			case versLaGauche: new_snake_head.x--; break;
			case versLaDroite: new_snake_head.x++; break;
		}
		if(!is_pix_in_game(new_snake_head))
			return EXIT_SPACE;
		else if(pix_find(obst, new_snake_head) != NOT_FOUND)
			return TOUCH_OBST;
		if(pix_find(snake1, new_snake_head) != NOT_FOUND || pix_find(snake2, new_snake_head) != NOT_FOUND )
			return TOUCH_SNAKE;
		if((grow)&&(pix_len(snake1) + 1 == size))
			return LEN_MAX;
		else if (grow)// augmente la len de snake
			snake2[pix_len(snake1)] = pix_end();
		if(!grow) {// efface la queue de snake
			Pixel pix_a_effacer = pix_pop_back(snake2);
			drawPoint(pix_a_effacer.x, pix_a_effacer.y, black);
		}
		// Actualisation de la chaine de pixel snake
		pix_push(snake2, size, new_snake_head);
		snake_draw(snake2);
	}
	return GAME_RUNING;
}

// Calcule la nouvelle direction à suivre si on tourne à droite
// à partir de la valeur direction
Direction turnRight(Direction direction) {
	// Calcule la nouvelle direction en fonction de celle en paramètre
	switch (direction) {
		case versLeHaut: return versLaDroite;
		case versLeBas: return versLaGauche;
		case versLaGauche: return versLeHaut;
		case versLaDroite: return versLeBas;
		}
	return direction;
}

// Calcule la nouvelle direction à suivre si on tourne à gauche
// à partir de la valeur direction
Direction turnLeft(Direction direction) {
	// Calcule la nouvelle direction en fonction de celle en paramètre
	switch (direction) {
		case versLeHaut: return versLaGauche;
		case versLeBas: return versLaDroite;
		case versLaGauche: return versLeBas;
		case versLaDroite: return versLeHaut;
	}
	return direction;
}


//////////////////////////////////////////////////////////////////////////
// Partie test
//////////////////////////////////////////////////////////////////////////

// Test de la gestion des Pixels
void test_Pixel() {
	// La sentinelle est égale à elle même
	assert(pix_equal(pix_end(),pix_end()));

	// Le point (0,0) est égal à lui même
	assert(pix_equal(pix_new(0,0),pix_new(0,0)));

	// La sentinelle est différente du point (0,0)
	assert(!pix_equal(pix_end(),pix_new(0,0)));

	// La sentinelle est identifiée par is_pix_end
	assert(is_pix_end(pix_end()));

	// Le point (0,0) n'est pas une sentinelle
	assert(! is_pix_end(pix_new(0,0)));

	printf("*** Le test de Pixel est passé sans erreurs\n");
}

// Test de la gestion d'une chaine de pixels
void test_Pixel_String() {
	// Création d'un tableau de len pixels max
	const size_t len = 4;
	// Le tableau doit avoir une case de plus pour stocker la sentinelle
	const size_t size = len + 1;
	Pixel tab[size];

	// Vide la chaine de pixel
	pix_clear(tab);

	// Vérifie que la chaine est vide
	// C'est à dire que sa première case contient la sentinelle
	assert(pix_equal(tab[0],pix_end()));

	// Verifie qu'une chaine vide a une taille nulle
	assert(pix_len(tab) == 0);

	// Définit des pixels
	Pixel p1 = pix_new(45,67);
	Pixel p2 = pix_new(12,23);
	Pixel p3 = pix_new(134,86);
	Pixel p4 = pix_new(0,567);
	Pixel p5 = pix_new(98,218);

	// Ajoute p1  au début de la chaine
	pix_push(tab,size,p1);

  // Vérifie que la longeur de la chaine vaut maitenant 1
	assert(pix_len(tab) == 1);

	// Vérifie que le premier pixel est bien celui que l'on a ajouté
	assert(pix_equal(tab[0],p1));

	// ajoute un autre pixel au début
	pix_push(tab,size,p2);
	// Vérifie que le premier pixel est bien celui que l'on a ajouté
	assert(pix_equal(tab[0],p2));
	// et que p1 est dans la case suivante
	assert(pix_equal(tab[1],p1));
	// et que la longeur est maintenant de 2
	assert(pix_len(tab) == 2);

	// ajoute un autre pixel à la fin
	pix_push_back(tab,size,p3);
	// Vérifie la disposition des pixels dans la liste
	assert(pix_equal(tab[0],p2));
	assert(pix_equal(tab[1],p1));
	assert(pix_equal(tab[2],p3));
	// et que la longeur est maintenant 3
	assert(pix_len(tab) == 3);

	// ajoute un autre pixel au début
	pix_push(tab,size,p4);
	// Vérifie la disposition des pixels dans la liste
	assert(pix_equal(tab[0],p4));
	assert(pix_equal(tab[1],p2));
	assert(pix_equal(tab[2],p1));
	assert(pix_equal(tab[3],p3));
	// et que la longeur est maintenant 4
	assert(pix_len(tab) == 4);

	// Recherche des pixels
	assert(pix_find(tab,p1) == 2);
	assert(pix_find(tab,p2) == 1);
	assert(pix_find(tab,p3) == 3);
	assert(pix_find(tab,p4) == 0);
	// Important : il faut caster le -1 vers le type size_t
	assert(pix_find(tab,p5) == NOT_FOUND);

	// Enlève les pixels de la fin
	assert(pix_equal(pix_pop_back(tab),p3));
	assert(pix_len(tab) == 3);
	assert(pix_equal(pix_pop_back(tab),p1));
	assert(pix_len(tab) == 2);
	assert(pix_equal(pix_pop_back(tab),p2));
	assert(pix_len(tab) == 1);
	assert(pix_equal(pix_pop_back(tab),p4));
	assert(pix_len(tab) == 0);

	printf("*** Le test des chaînes de Pixels est passé sans erreurs\n");
}

// Test les obstacles
void test_obstacle() {
	// Définit un nombre d'obstacle à produire
	const size_t len = 25;
	// En déduit la taille du tableau : il faut compter la sentinelle
	const size_t size = len+1;
	// Définit le tableau qui contient la chaîne des pixels
	Pixel obst[size];

	// Verifie que le nombre d' obstacles est plus petit que la taille
	// du tableau car il faut compter la sentinelle
	assert(len < size);

	// On a besoin de curse pour définir LINES et COLS
	startCurses();

	// Initialise les obstacles
	obst_init(obst,size,len);

	// Vérifie tous les obstacles
	size_t i;
	for (i = 0; ! is_pix_end(obst[i]); i++)
		// Verifie que chaque obstacle est dans la fenêtre de jeux
		assert(is_pix_in_game(obst[i]));
	// Vérifie qu'on a bien créé le bon nombre d'obstacles
	if (i != len) {
		printf("Erreur : %lu obstacles crées alors qu'il en faut %lu\n",i,len);
		exit(1);
	}

	stopCurses();

	printf("*** Le test des obstacles est passé sans erreurs\n");
}

	// Paramétrage de la partie :
	// Choisi une vitesse de jeux
	// Choisi un temps de grossissement du serpent
	// Compteur de temps pour le grossisment
	// Il est décrémenté à chaque mouvement
	// Initialise les obstacles
	// Longueur de la chaine des obstacles
	// Taille du tableau qui contient les obstacles
	// On ajoute 1 pour stocker la sentinelle
	// Longueur maximum que Snake doit atteindre pour être adulte
	// Taille du tableau qui doit contenir la chaîne des Pixels
typedef struct{
	int gameSpeed;
	int growTimeMax;
	size_t len_obst;
	size_t size_obst;
	size_t len_max_snake;
	size_t size_snake;
} Partie;

Partie param_partie(char difficulte){
	Partie param_partie;
	switch(difficulte)
	{
		case 'f': param_partie.gameSpeed = 100; param_partie.growTimeMax = 10; param_partie.len_obst = 60; param_partie.size_obst = param_partie.len_obst + 1; param_partie.len_max_snake = 90; param_partie.size_snake = param_partie.len_max_snake + 1; break;
		case 'm': param_partie.gameSpeed = 85; param_partie.growTimeMax = 8; param_partie.len_obst = 90; param_partie.size_obst = param_partie.len_obst + 1; param_partie.len_max_snake = 100; param_partie.size_snake = param_partie.len_max_snake + 1; break;
		case 'd': param_partie.gameSpeed = 70; param_partie.growTimeMax = 6; param_partie.len_obst = 120; param_partie.size_obst = param_partie.len_obst + 1; param_partie.len_max_snake = 110; param_partie.size_snake = param_partie.len_max_snake + 1; break;
		default :  param_partie.gameSpeed = 100; param_partie.growTimeMax = 10; param_partie.len_obst = 30; param_partie.size_obst = param_partie.len_obst + 1; param_partie.len_max_snake = 100; param_partie.size_snake = param_partie.len_max_snake + 1; break;
	}

	return param_partie;
}

//////////////////////////////////////////////////////////////////////////
// Partie principale : l'initalisation, la boucle du jeux, et la fin
//////////////////////////////////////////////////////////////////////////

int main() {

	// Lance les tests du Test Driven Developpement
	test_Pixel();
	test_Pixel_String();
	test_obstacle();

	// Récupérer les informations pour créer la partie
	char nbr_joueur = 0; // nombre de joueur (pouvant valoir 1 ou 2)
	printf("\n\nCombien de joueur êtes vous ? (la réponse doit être 1 ou 2) : ");
	scanf(" %c%*[^\n]", &nbr_joueur);
	while(nbr_joueur!='1' && nbr_joueur!='2') {
		printf("Veuillez saisir 1 pour un seul joueur ou 2 pour deux joueurs : ");
		scanf(" %c%*[^\n]", &nbr_joueur);
		printf("%c\n", nbr_joueur);
	}
	char difficulte_partie;
	printf("\nEn quelle difficulté voulez vous jouer ?\nSaissir f pour facile\n        m pour moyen\n     ou d pour difficile) : ");
	scanf(" %c%*[^\n]", &difficulte_partie);
	while(difficulte_partie != 'f' && difficulte_partie != 'm' && difficulte_partie != 'd'){
		printf("Saissir f pour facile\n     OU  m pour moyen\n     OU  d pour difficile : ");
		scanf(" %c%*[^\n]", &difficulte_partie);
	}
	const Partie parametre = param_partie(difficulte_partie);
	// Rappel des règles
	char regles=0x00;
	if(nbr_joueur == '1'){
		printf("\n\nUne partie :\n\n\tpour %c joueur\n\tde niveau %c va se lancer.\n\nAvant de commencer la partie voici les règles :\n\n\tutilisez les flèches gauche et droite pour vous déplacer\n\n\ttoucher les bords, un obstacle ou votre serpent vous fait perdre\n\n\tune fois atteint une certaine taille vous avez gagné\n\nCliquez sur une touche puis ENTRER pour commencer :", nbr_joueur, difficulte_partie);
		scanf(" %c*[^\n]", &regles);
	}else if(nbr_joueur == '2') {
		printf("\n\nUne partie : \n\n\tpour %c joueurs\n\t de niveau %c va se lancer.\n\nAvant de commencer la partie voici les règles :\n\n\tJoueur 1 utilise les flèches gauche et droite se déplacer\n\n\tJoueur 2 utilise les touches Q et S pour se déplacer\n\n\ttoucher les bords, un obstacle ou un serpent vous fait perdre\n\n\tune fois atteint une certaine taille vous avez gagné\n\nCliquez sur une touche puis ENTRER pour commencer : ", nbr_joueur, difficulte_partie);
		scanf(" %c*[^\n]", &regles);
	}
	printf("\n\n\n");

	// démarre le mode dessin curses sur le terminal
	startCurses();

	// C'est le temps d'attente en ms dans l'entrée d'une touche avec getch()
	timeout(parametre.gameSpeed);

	// Score : le nombre de mouvements réussis
	int score = 0;

	// Snake grandit quant cette valeur tombe à zéro
	int growTime = parametre.growTimeMax;

	// Définition du tableau qui contient la chaîne des obstacles
	Pixel obstacle[parametre.size_obst];
	// Initialisation des obstacles
	obst_init(obstacle, parametre.size_obst, parametre.len_obst);
	// Dessine les obstacles à l'écran
	obst_draw(obstacle);

	// Le serpent est definit par une chaîne de Pixels
	// Pour pouvoir agrandir la chaîne, elle est stockée
	// dans un tableau est géré en partie pleine / vide avec une sentinelle
	// La tête est à la position 0

	// Tableau pour stocker la chaîne des pixels de Snake
	Pixel snake1[parametre.size_snake];
	Pixel snake2[parametre.size_snake];
	pix_clear(snake2);
	// Direction actuelle du serpent
	Direction direction1 = versLaGauche;
	Direction direction2 = versLaGauche;
	// Initialise Snake avec une longeur de 5
	snake_init(snake1,parametre.size_snake,5,1);

	if(nbr_joueur == '2') {
		snake_init(snake2,parametre.size_snake,5,2);
		snake_draw(snake2);
	}
	// Affiche l'état de départ du sepent
	snake_draw(snake1);

	// Caractère lu au clavier
	int charRead;

	// Etat du jeux après chaque mouvement
	GameStatus gameStatus = GAME_RUNING;

	// On réalise l'annimation tant que le jeux tourne pour 1 JOUEUR
	while (gameStatus == GAME_RUNING && nbr_joueur == '1') {

		// Fait bouger le serpent et le fait grandir tout les growTimeMax mouvements
		if (growTime == 0) {
		// le serpent grandit
			gameStatus = snake_move(snake1,snake2,parametre.size_snake,obstacle,direction1,direction2,true,nbr_joueur);
			growTime = parametre.growTimeMax;
		} else {
		// le serpent ne grandit pas
			gameStatus = snake_move(snake1,snake2,parametre.size_snake,obstacle,direction1,direction2,false,nbr_joueur);
			growTime--;
		}
		// Augmente le score à chaque mouvement
		score++;

		//  Lecture du clavier et attente de gameSpeed ms si aucune touche n'est enfoncée
		// Ce temps est définit par la fonction timeout()
		charRead = getch();
		// Changement de l'état du jeux en fonction de l'entrée de l'utilisateur
		switch (charRead) {
		// Fin du jeux
			case 'n': gameStatus = GAME_STOPPED;  break;
			// Change la direction : demande à Sanke de tourner à droite
			case KEY_RIGHT: direction1 = turnRight(direction1); break;
			case 'q': direction2 = turnRight(direction2); break;
			// Change la direction : demande à Sanke de tourner à gauche
			case KEY_LEFT: direction1 = turnLeft(direction1); break;
			case 's': direction2 = turnLeft(direction2); break;
			// Met en pause
			case 'p': // Pause
			// Le jeux est bloqué sur le getch() jusqu'à l'appui sur n'importe quelle touche
			timeout(-1);
			getch();
			// Le jeux reprend à la même vitesse
			timeout(parametre.gameSpeed);
			default:
			// On ne fait rien pour toutes les autres touches du clavier
			break;
		}
	}


	// On réalise l'annimation tant que le jeux tourne pour 2 JOUEURS
	while (gameStatus == GAME_RUNING && nbr_joueur == '2') {
		// Fait bouger le serpent et le fait grandir tout les growTimeMax mouvements
		if (growTime == 0) {
			// le serpent grandit
			gameStatus = snake_move(snake1,snake2,parametre.size_snake,obstacle,direction1,direction2,true,nbr_joueur);
			growTime = parametre.growTimeMax;
		} else {
		// le serpent ne grandit pas
			gameStatus = snake_move(snake1,snake2,parametre.size_snake,obstacle,direction1,direction2,false,nbr_joueur);
			growTime--;
		}
		// Augmente le score à chaque mouvement
		score++;

		//  Lecture du clavier et attente de gameSpeed ms si aucune touche n'est enfoncée
		// Ce temps est définit par la fonction timeout()
		charRead = getch();
		// Changement de l'état du jeux en fonction de l'entrée de l'utilisateur
		switch (charRead) {
			// Fin du jeux
			case 'n': gameStatus = GAME_STOPPED;  break;
			// Change la direction : demande à Sanke de tourner à droite
			case KEY_RIGHT: direction1 = turnRight(direction1); break;
			case 'q': direction2 = turnLeft(direction2); break;
			// Change la direction : demande à Sanke de tourner à gauche
			case KEY_LEFT: direction1 = turnLeft(direction1); break;
			case 's': direction2 = turnRight(direction2); break;
      		// Met en pause
			case 'p': // Pause
			// Le jeux est bloqué sur le getch() jusqu'à l'appui sur n'importe quelle touche
				timeout(-1);
				getch();
			// Le jeux reprend à la même vitesse
				timeout(parametre.gameSpeed);
			default:
			// On ne fait rien pour toutes les autres touches du clavier
			break;
		}
	}


	//Jeux terminé, fin du mode curse
	stopCurses();

	printf("Le jeux est terminé : ");
	// Conclusion du jeux
	switch (gameStatus) {
		case GAME_STOPPED: printf("vous avez arreté le jeux\n"); break;
		case EXIT_SPACE: printf("vous êtes sortis de l'aire du jeux\n"); break;
		case TOUCH_OBST: printf("vous avez touché un obstacle\n"); break;
		case TOUCH_SNAKE: printf("vous avez mordu votre queue\n"); break;
		case LEN_MAX: printf("BRAVO, Snake a atteint sa taille adulte\nVous avez gagné !\n"); break;
		default: printf("ERREUR INTERNE : état du jeux incorrect\n"); break;
	}

	printf("Snake a atteind la taille de %lu\n",pix_len(snake1));
	printf("Votre score : %i\n",score);
	printf("Merci d'avoir joué ...\n");
}
