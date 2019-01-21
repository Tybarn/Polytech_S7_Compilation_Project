#ifndef PROJET_H_
#define PROJET_H_

/* C fichier contient toutes les structures nécessaires */
#include <stdlib.h>

/* deux macros pratiques, utilisees dans les allocations de structure
 * Pour NEW on donne le nombre et le type de la stucture a allouer (pas le type
 * du pointeur) et on recupere un pointeur sur cette structure.
 * Pour NIL: on donne de meme le type de la structure (pas le type du pointeur)
 */
#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

#define TRUE 1
#define FALSE 0


/* Codes d'erreurs */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	4
#define EVAL_ERROR	5
#define UNEXPECTED	10


/* Etiquettes pour les arbres de syntaxe abstraite */
#define Eadd	1
#define Eminus	2
#define Emult	3
#define Ediv	4
#define ITE	5
#define ECST	6
#define EIDVAR	7
#define NE 	8
#define EQ 	9
#define LT 	10
#define LE 	11
#define GT 	12
#define GE 	13
#define ELIST	14
#define EDECL	15
#define EIDCLASS	16
#define AFFECT 17
#define EAND 18
#define Ecast 19
#define Einstan 20
#define Einstruct 21
#define Eselect 22
#define Emessage 23
#define Earg 24
#define Ebloc 25
#define Estring 26
#define Einteger 27
#define Eobjet 28
#define EBlocObjet 29
#define EDeclObjet 30
#define Eparam 31
#define Emeth 32
#define Econstructor 33
#define Echamp 34
#define Eclass 35
#define Eaxiome 36
#define Ereturn 37

typedef int bool;

typedef struct _class;		/* structure de la classe */

/* la structure d'un arbre (noeud ou feuille) */
typedef struct _Tree {
  short op;         /* etiquette de l'operateur courant */
  short nbChildren; /* nombre de sous-arbres */
  union {
    char *str;      /* valeur de la feuille si op = ID */
    int val;        /* valeur de la feuille si op = CST */
    struct _Tree **children; /* tableau des sous-arbres d'un noeud interne */
  } u;
} Tree, *TreeP;


/**
   STRUCTURE DES ATTRIBUTS
 */
typedef struct _Decl
{
  char *name;			/* nom de l'attribut */
  struct _class *type;		/* son type qui est forcement une classe */
  struct _Decl *next;
  bool isVar;               /*pour savoir si c'est une variable*/
  bool isAttribute;	    /* si c'est un attribut d'une classe */
} VarDecl, *VarDeclP;

/**
   stucture d'une METHODE
 */
typedef struct _method
{
  char *name;
  struct _class *returnType; 		/* type de retour ou char* nom du type de retour */
  VarDeclP parameters;		/* liste des parametres */
  TreeP body;			/* corps de la fonction */
  struct _class *home; 		/* la classe proprietaire de cette methode */
  bool isOverride;		/* si cette methode est une redefinition */
  bool isConstructor;		/* si la methode est constructeur */
  struct _method *next ;        /*les methodes suivantes*/
}Method, *MethodP;

/**
   structure pour la CLASSE et l'OBJET ISOLE
 */
typedef struct _class
{
  char *name ; 			/* nom de la classe */
  VarDeclP headParam;		/* les parametres de l'entete */
  struct _class *baseClass;	/* la classe mere */
  VarDeclP fields;		/* liste des attributs */ 
  MethodP listMethods;		/* liste des methodes de la classe */
  struct _class *next;      /*les classes suivantes*/
  bool isClass;       /*si c'est une classe ou un objet*/
} Class, *ClassP;


/* Type pour la valeur de retour de Flex et les actions de Bison
 * le premier champ est necessaire pour Flex.
 * les autres correspondent aux variantes utilisees dans les actions
 * associees aux productions de la grammaire.
*/
typedef union
{ char C;	/* RELOP = caractere isole */
  char *S;	/* IDVAR = chaine de caractere */
  char *V; /*IDCLASS = chaine de caractere*/
  int I;	/* CST = valeur entiere */
  VarDeclP D;	/* liste de paires (variable, valeur) */
  TreeP T;	/* AST */
} YYSTYPE;



/*-------------- METHODES ----------------*/

/* construction pour les AST */
TreeP makeLeafStr(short op, char *str); 	    /* feuille (string) */
TreeP makeLeafInt(short op, int val);	            /* feuille (int) */
TreeP makeTree(short op, int nbChildren, ...);	    /* noeud interne */

/* Impression des AST */
void printAST(TreeP decls, TreeP main);
#endif
