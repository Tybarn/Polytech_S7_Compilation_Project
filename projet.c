#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "projet.h"
#include "genCode.h"
#include "projet_y.h"


extern int yyparse();
extern int yylineno;

extern char *strdup(const char *s);
void printExpr(TreeP tree);

/* Niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Evaluation ou pas. Par defaut, on evalue les expressions */
bool noEval = FALSE;

/* code d'erreur a retourner: liste dans tp.h */
int errorCode = NO_ERROR;

/* yyerror:  fonction importee par Bison et a fournir explicitement.
 * Elle est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message minimal.
 */
void yyerror(char *ignore) {
  fprintf(stderr, "Syntax error on line: %d\n", yylineno);
}


/* Appel:
 *   tp [-option]* programme.txt donnees.dat
 * Le fichier de donnees est obligatoire si le programme execute la
 * construction GET (pas de lecture au clavier), facultatif sinon.
 * Les options doivent apparaitre avant le nom du fichier du programme.
 * Options: -[eE] -[vV] -[hH?]
 */
int main(int argc, char **argv) {
  int fi;
  int i, res;

  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'v': case 'V':
	verbose = TRUE; continue;
      case 'e': case 'E':
	noEval = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Syntax: tp -e -v program.txt\n");
	exit(USAGE_ERROR);
      default:
	fprintf(stderr, "Error: Unknown Option: %c\n", argv[i][1]);
	exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Error: Program file is missing\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "Error: Cannot open %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  if (i < argc) { /* fichier dans lequel lire les valeurs pour get() */
    fprintf(stderr, "Error: extra argument: %s\n", argv[i]);
    exit(USAGE_ERROR);
  }

  /* Lance l'analyse syntaxique de tout le source, en appelant yylex au fur
   * et a mesure. Execute les actions semantiques en parallele avec les
   * reductions.
   * yyparse renvoie 0 si le source est syntaxiquement correct, une valeur
   * differente de 0 en cas d'erreur syntaxique (eventuellement dues a des
   * erreurs lexicales).
   * Comme l'interpretation globale est automatiquement lancee par les actions
   * associees aux reductions, une fois que yyparse a termine il n'y
   * a plus rien a faire (sauf fermer les fichiers)
   * Si le code du programme contient une erreur, on bloque l'evaluation.
   * S'il n'y a que des erreurs contextuelles on essaye de ne pas s'arreter
   * a la premiere mais de continuer l'analyse pour en trovuer d'autres, quand
   * c'est possible.
   */
  res = yyparse();
  if (res == 0 && errorCode == NO_ERROR) return 0;
  else {
    return res ? SYNTAX_ERROR : errorCode;
  }
}



/* mémorise le code d'erreur et s'arrange pour bloquer l'évaluation */
void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) { noEval = TRUE; }
}


/* Fonction auxiliaire pour la construction d'arbre : renvoie un squelette
 * d'arbre avec 'nbChildre'n fils et d'etiquette 'op' donnee. Les appelants
 * doivent eux-mêmes stocker les fils une fois que MakeNode a renvoye son
 * resultat
 */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->op = op; tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}


/* Construction d'un arbre a nbChildren branches, passees en parametres
 * 'op' est une etiquette symbolique qui permet de memoriser la construction
 * dans le programme source qui est representee par cet arbre.
 * Une liste preliminaire d'etiquettes est dans tp.h; il faut l'enrichir selon
 * vos besoins.
 * Cette fonction prend un nombre variable d'arguments: au moins deux.
 * Les eventuels arguments supplementaires doivent etre de type TreeP
 * (defini dans tp.h)
 */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  int i;
  TreeP tree = makeNode(nbChildren, op); 
  va_start(args, nbChildren);
  for (i = 0; i < nbChildren; i++) { 
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  return(tree);
}


/* Retourne le i-ieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int i) {
  return tree->u.children[i];
}


/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op); 
  tree->u.val = val;
  return(tree);
}


/* Verifie que nouv n'apparait pas deja dans list. l'ajoute en tete et
 * renvoie la nouvelle liste
 */
/*
VarDeclP addToScope(VarDeclP list, VarDeclP nouv) {
  VarDeclP p; int i;
  for(p=list, i = 0; p != NIL(VarDecl); p = p->next, i = i+1) {
    if (! strcmp(p->name, nouv->name)) {
      fprintf(stderr, "Error: Multiple declaration in the same scope of %s\n",
	      p->name);
      setError(CONTEXT_ERROR);
      break;
    }
  }
  nouv->rank = i; nouv->next=list;
  return nouv;
}
*/

/* Construit le squelette d'un element de description d'une variable */
VarDeclP makeVar(char *name) {
  VarDeclP res = NEW(1, VarDecl);
  res->name = name; res->next = NIL(VarDecl);
  return(res);
}


/**
 * 	A partir d'ici les fonctions ont besoin d'etre modifiees/completees
 **/

char *getLabel() {
  static char buf[5];
  static int cpt = 1;
  sprintf(buf, "Eti%d", cpt++);
  return strdup(buf);
}


/* Avant evaluation, verifie si tout identificateur qui apparait dans tree a
 * bien ete declare (dans ce cas il doit etre dans la liste lvar).
 * On impose que ca soit le cas y compris si on n'a pas besoin de cet
 * identificateur pour l'evaluation, comme par exemple x dans
 * begin if 1 = 1 then 1 else x end
 * Le champ 'val' de la structure VarDecl n'est pas significatif
 * puisqu'on n'a encore rien evalue.
 */
bool checkScope(TreeP tree, VarDeclP lvar) {
  VarDeclP p; char *name;
  if (tree == NIL(Tree)) { return TRUE; }
  switch (tree->op) {
  case EIDVAR :
    name = tree->u.str;
    for(p=lvar; p != NIL(VarDecl); p = p->next) {
      if (! strcmp(p->name, name)) { return TRUE; }
    }
    fprintf(stderr, "\nError: undeclared variable %s\n", name);
    setError(CONTEXT_ERROR);
    return FALSE;
  case ECST:
    return TRUE;
  case ITE:
    return checkScope(getChild(tree, 0), lvar)
      && checkScope(getChild(tree, 1), lvar)
      && checkScope(getChild(tree, 2), lvar);
  case EQ:
  case NE:
  case GT:
  case GE:
  case LT:
  case LE:
  case Eadd:
  case Eminus:
  case Emult:
  case Ediv:
    return checkScope(getChild(tree, 0), lvar)
      && checkScope(getChild(tree, 1), lvar); 
  default: 
    fprintf(stderr, "Erreur! etiquette indefinie: %d\n", tree->op);
    exit(UNEXPECTED);
  }
}

/*
VarDeclP declVar(char *name, TreeP tree, VarDeclP decls) {
  VarDeclP pvar = NEW(1, VarDecl);
  pvar->name = name; pvar->next = NIL(VarDecl);
  checkScope(tree, decls);
  genCode(tree, decls);
  return addToScope(decls, pvar);
}
*/

/* Constructeur de feuille dont la valeur est une chaine de caracteres
 * Si check vaut Vrai, Verifie si cet identificateur a bien ete declare.
 */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op); 
  tree->u.str = str;
  return(tree);
}


/*-------------- AJOUTS PROJET COMPILATION ----------------*/

ClassP declClass(char* name, VarDeclP listParam, ClassP classeMere, VarDeclP listChamp, MethodP listMethod){
  ClassP cla = NEW(1, Class);
  cla->name = name;
  cla->headParam = listParam;
  cla->baseClass = classeMere;
  cla->fields = listChamp;
  cla->listMethods = listMethod;
  cla->next = NIL(Class);
  cla->isClass = TRUE;
  return(cla);
}

ClassP addClass(ClassP cl, ClassP nouv){
  ClassP c;
  int i;
  /*Si il y a une classe mere et que le nom de la nouvelle classe est le meme, alors erreur*/
  if((! strcmp(nouv->baseClass->name, nouv->name) && (nouv->baseClass != NIL(Class)))){
    fprintf(stderr, "Error: Multiple CLASS declaration in the same scope of %s\n", nouv->baseClass->name);
    setError(CONTEXT_ERROR);
  }
  for(c= cl, i = 0; c != NIL(Class); c = c->next, i = i+1) {
    if (! strcmp(c->name, nouv->name)) {
        fprintf(stderr, "Error: Multiple CLASS declaration in the same scope of %s\n", c->name);
        setError(CONTEXT_ERROR);
        break;
    }
  }
  nouv->next=cl;
  return nouv;
}

ClassP declObject(char* name, VarDeclP listParam, ClassP classeMere, VarDeclP listChamp, MethodP listMethod){
  ClassP obj = NEW(1, Class);
  obj->name = name;
  obj->headParam = listParam;
  obj->baseClass = classeMere;
  obj->fields = listChamp;
  obj->listMethods = listMethod;
  obj->next = NIL(Class);
  obj->isClass = FALSE;
  return(obj);
}


VarDeclP declChamp(char *name, ClassP type, bool isVar, bool isAttribute){
  VarDeclP decl = NEW(1, VarDecl);
  decl->name = name;
  decl->type = type;
  decl->next = NIL(VarDecl);
  decl->isVar = isVar;
  decl->isAttribute = isAttribute;
  return(decl);
}

VarDeclP addChamp(VarDeclP ch, VarDeclP nouv){
  VarDeclP v; 
  int i;
  for(v=ch, i = 0; v != NIL(VarDecl); v = v->next, i = i+1) {
    if (! strcmp(v->name, nouv->name)) {
      fprintf(stderr, "Error: Multiple FIELD declaration in the same scope of %s\n", v->name);
      setError(CONTEXT_ERROR);
      break;
    }
  }
  nouv->next=ch;
  return nouv;
}

MethodP addMethod(MethodP m, MethodP nouv){
  MethodP meth; 
  int i;
  for(meth=m, i = 0; meth != NIL(Method); meth = meth->next, i = i+1) {
    if (! strcmp(meth->name, nouv->name)) {
      fprintf(stderr, "Error: Multiple FIELD declaration in the same scope of %s\n", meth->name);
      setError(CONTEXT_ERROR);
      break;
    }
  }
  nouv->next=m;
  return nouv;
}



/********* FONCITON UTILES POUR LA DECLARATION D'UNE METHODE ******/
/**
   Verifie si la methode est contenue dans list 
*/
MethodP containsMethod(MethodP m, MethodP list)
{
  if(list == NIL(Method)) return NIL(Method);
  MethodP tmp = list;
  while(tmp != NIL(Method)){
    if(strcmp(m->name, tmp->name) == 0){
      return tmp;
    }
    tmp = tmp->next;
  }
  return NIL(Method);
}

/**
   renvoie la taille d'une liste de VarDecl
*/
int size(VarDeclP list){
  if(list == NIL(VarDecl))
    return 0;
  return 1 + size(list->next);
}


/**
   compare deux liste de parametre pour voir s'il ont les memes types
   aide a la verification de la surcharge ou redefinition
*/
bool compareParameterType(VarDeclP param1, VarDeclP param2){
  if(param1 == NIL(VarDecl) && param2 == NIL(VarDecl))
    return TRUE; 		/* les deux sont vides */
  if(param1 == NIL(VarDecl) && param2 != NIL(VarDecl)){
    setError(CONTEXT_ERROR);
    return FALSE;		/* l'un est vide et pas l'autre */
  }
  
  if(param2 == NIL(VarDecl) && param1 != NIL(VarDecl)){
    setError(CONTEXT_ERROR);
    return FALSE;		/* l'un est vide et pas l'autre */
  }
  if(size(param1) != size(param2)){
    setError(CONTEXT_ERROR);
    return FALSE;
  }
  
  VarDeclP tmp1 = param1;
  VarDeclP tmp2 = param2;
  /* les  types doivent etre les memes*/ 
  while(tmp1 != NIL(VarDecl)){
    if(strcmp(tmp1->type->name, tmp1->type->name) != 0){
      return FALSE;
    }
    tmp1 = tmp1->next;
    tmp2 = tmp2->next;
  }
  return FALSE;
}


/**
   tester si m est défini dans les super classe de classe
*/
bool isRedefinition(MethodP m, ClassP classe){
  /* si pas de classe mere */
  if(classe->baseClass == NIL(Class)){
    printf("%s doesn't have to be redefined\n", m->name);
    setError(CONTEXT_ERROR);
    return TRUE;
  }
 
  ClassP mere = classe->baseClass;
  while(mere != NIL(Class)){
    MethodP mp = containsMethod(m, classe->baseClass->listMethods);
    if(mp == NIL(Method)){  /* si la mere n'a pas la methode m */
      printf("%s doesn't have to be redefined\n", m->name);
      setError(CONTEXT_ERROR);
      return FALSE;
    } 

    /* meme parametre et type de retour */
    if(compareParameterType(m->parameters, mp->parameters) == TRUE){
      if(strcmp(m->returnType->name, mp->returnType->name) != 0){
	printf("%s doesn't exist in super-class\n", m->name);
        setError(CONTEXT_ERROR);
	return FALSE;
      }
    }
    else{
      setError(CONTEXT_ERROR);
      return FALSE;
    } 
    mere = mere->baseClass;
  }
  return TRUE;
}

/**
   tester si deux listde parametres sont egales
 */
bool equalsVarDecl(VarDeclP param1, VarDeclP param2){
  if(param1 == NIL(VarDecl) && param2 == NIL(VarDecl))
    return TRUE; 		/* les deux sont vides */
  if(param1 == NIL(VarDecl) && param2 != NIL(VarDecl)){
    setError(CONTEXT_ERROR);
    return FALSE;		/* l'un est vide et pas l'autre */
  }
  
  if(param2 == NIL(VarDecl) && param1 != NIL(VarDecl)){
    setError(CONTEXT_ERROR);
    return FALSE;		/* l'un est vide et pas l'autre */
  }
  if(size(param1) != size(param2)){
    setError(CONTEXT_ERROR);
    return FALSE;
  }

  VarDeclP tmp1 = param1;
  VarDeclP tmp2 = param2;
  /* les  types doivent etre les memes*/ 
  while(tmp1 != NIL(VarDecl)){
    if((strcmp(tmp1->type->name, tmp2->type->name) != 0) ||
       (strcmp(tmp1->name, tmp2->name) != 0) ||
       (tmp1->isVar != tmp2->isVar)
       ){
      return FALSE;
    }
    tmp1 = tmp1->next;
    tmp2 = tmp2->next;
  }
  return TRUE;
}

/** Verifier la surcharge de toutes les methodes dans la classe ou l'objet
    avant la generation du code
    Dans ce cas, la methode ne doit pas etre dans la liste des methodes de la classe 
    ou dans celle de la super class
*/
bool checkMethod(MethodP method, ClassP class)
{
  MethodP cont = containsMethod(method, class->listMethods);

  /* si la methode n'existe pas dans la classe */
  if(cont == NIL(Method)){
    //Verifier si C un constructeur
    if(method->isConstructor == TRUE){
      if(equalsVarDecl(method->parameters, cont->parameters) == FALSE){ /* memes parametres */
	setError(CONTEXT_ERROR);
	return FALSE;
      }
    }
   
    return TRUE;
  }
  
  /* si elle y existe, verifier si C une surcharge ou redefinition */
  if(method->isOverride == FALSE){
    /* surcharge */
    if(compareParameterType(cont->parameters, method->parameters) == TRUE){
      printf("%s can't surcharge\n", method->name);
      setError(CONTEXT_ERROR);
      return FALSE;
    }
    else{
      return TRUE;  /* méthode valide */
    }
  }
  else{
    /* si redefinition */
    if(compareParameterType(cont->parameters, method->parameters) == TRUE){
      return isRedefinition(method, class);
    }
  }
  setError(CONTEXT_ERROR);
  return FALSE;
}



/**
   initialisation d'une Methode 
*/
MethodP makeMethod(char *name, ClassP returnType, VarDeclP parameters, TreeP body, ClassP home, bool isOverride, bool isConstructor)
{
  MethodP pmethod = NEW (1, Method);  pmethod->next = NIL(Method);
  pmethod->name = name;              pmethod->returnType = returnType;
  pmethod->parameters = parameters;   pmethod->body = body;
  pmethod->home = home;               pmethod->isOverride = isOverride;
  pmethod->isConstructor = isConstructor;

  return pmethod;
}


/**
   Declaration d'une methode
*/
MethodP declMethode(char *name, ClassP returnType, VarDeclP parameters, TreeP body, ClassP home, bool isOverride, bool isConstructor)
{
  MethodP pmethod = makeMethod(name, returnType, parameters, body, home, isOverride, isConstructor);
  if(checkMethod(pmethod, home) == FALSE){
    exit(EXIT_FAILURE);
  }
  pmethod = addMethod(pmethod, home->listMethods);
  //TODO
  //Appel la fonction qui genere le code pour la methode
  
  return pmethod;
}

VarDeclP declParam(char *name, ClassP type, bool isVar, bool isAttribute){
  VarDeclP decl = NEW(1, VarDecl);
  decl->name = name;
  decl->type = type;
  decl->next = NIL(VarDecl);
  decl->isVar = isVar;
  decl->isAttribute = isAttribute;
  return(decl);
}

VarDeclP addParam(VarDeclP ch, VarDeclP nouv){
  VarDeclP v; 
  int i;
  for(v=ch, i = 0; v != NIL(VarDecl); v = v->next, i = i+1) {
    if (! strcmp(v->name, nouv->name)) {
      fprintf(stderr, "Error: Multiple FIELD declaration in the same scope of %s\n", v->name);
      setError(CONTEXT_ERROR);
      break;
    }
  }
  nouv->next=ch;
  return nouv;
}

/*
TreeP initOpt(ClassP type, bool isAff, ...){
  va_list  args;
  if(isAff){
    return evalDef(type, isExpr, args);
  }
  else{
    return NIL(Tree);    
        
  }
  return eval()
    }
TreeP evalDef(ClassP type, bool isExpr, ...){
  if(isExpr)
    return makeTree(op, nbChil, ...);
  else
    return NIL(Tree);
}
VarDeclP declVar(char *name, ClassP type)
{
      
}
*/

/***********************    AFFICHAGE DE L'ARBRE   ********************************/

void printOpBinaire(char op) {
  switch(op) {
  case EQ:    printf("="); break;
  case NE:    printf("<>"); break;
  case GT:    printf("<"); break;
  case GE:    printf(">="); break;
  case LT:    printf("<"); break;
  case LE:    printf("<="); break;
  case Eadd:  printf("+"); break;
  case Eminus:printf("-"); break;
  case Emult: printf("*"); break;
  case Ediv:  printf("/"); break;
  default:
    fprintf(stderr, "Unexpected binary operator of code: %d\n", op);
    exit(UNEXPECTED);
  }
}



void printDecls(TreeP decls) {
  TreeP gauche, droite;
  if (decls == NIL(Tree)) { return; }
  if (decls->op != ELIST) {
    fprintf(stderr, "Mauvais format dans les declarations\n");
    exit(UNEXPECTED);
  }
  gauche = getChild(decls, 0); droite = getChild(decls, 1);
  printDecls(gauche);
  printf("%s := ", getChild(droite, 0)->u.str);
  printExpr(getChild(droite, 1));
  printf("\n");
}


void printAST(TreeP decls, TreeP main) {
  printDecls(decls);
  printExpr(main);
}


void printExpr(TreeP tree) {
    if(tree == NIL(Tree))
        return;
  switch (tree->op) {
  case EIDVAR :
    printf("%s", tree->u.str); break;
  case ECST:
    printf("%d", tree->u.val); break;
  case ITE:
    printf("[ITE "); printExpr(getChild(tree, 0)); /* la condition */
    printf(", "); printExpr(getChild(tree, 1)); /* la partie 'then' */
    printf(", "); printExpr(getChild(tree, 2)); /* la partie 'else' */
    printf("]");
    break;
  case EQ:
    printExpr(getChild(tree, 0));
    printf("==");
    printExpr(getChild(tree, 1));
    break;
  case NE:
    printExpr(getChild(tree, 0));
    printf("!=");
    printExpr(getChild(tree, 1));
    break;
  case GT:
    printExpr(getChild(tree, 0));
    printf(">=");
    printExpr(getChild(tree, 1));
    break;
  case GE:
    printExpr(getChild(tree, 0));
    printf(">");
    printExpr(getChild(tree, 1));
    break;
  case LT:
    printExpr(getChild(tree, 0));
    printf("<=");
    printExpr(getChild(tree, 1));
    break;
  case LE:
    printExpr(getChild(tree, 0));
    printf("<");
    printExpr(getChild(tree, 1));
    break;
  case Eadd:
    if(tree->nbChildren<2){
        printf("+=");
        printExpr(getChild(tree, 0));
    }
    else{
        printExpr(getChild(tree, 0));
        printf("+");
        printExpr(getChild(tree, 1));
    }
    break;   
  case Eminus:
    if(tree->nbChildren<2){
        printf("-=");
        printExpr(getChild(tree, 0));
    }
    else{
        printExpr(getChild(tree, 0));
        printf("-");
        printExpr(getChild(tree, 1));
    }
    break;  
  case Emult:
    printExpr(getChild(tree, 0));
    printf("*");
    printExpr(getChild(tree, 1));
    break;
  case Ediv:
    printf("(");
    printExpr(getChild(tree, 0));
    printf("/"); //tree->op
    printExpr(getChild(tree, 1));
    printf(")"); 
    break;
  case EDECL :
    if(tree->nbChildren<3){
        printExpr(getChild(tree, 0));
        printExpr(getChild(tree, 1));
    }
    else{
        printf("%s", getChild(tree,0)->u.str);
        printf(":"); 
        printf("%s", getChild(tree,1)->u.str);
        printExpr(getChild(tree, 2));
    }
    break;
  case EIDCLASS :
    printf("%s", tree->u.str);
    break;
  case AFFECT: 
     if(tree->nbChildren<2){
        printf(":="); 
        printExpr(getChild(tree, 0));
    }
    else{
        printExpr(getChild(tree, 0));
        printf(":="); 
        printExpr(getChild(tree, 1));
    }
    break;  
  case Ecast: 
    printf("("); 
    printExpr(getChild(tree, 0));
    printExpr(getChild(tree, 1));
    printf(")"); 
    break;  
  case Einstan:
    printf("new"); 
    printExpr(getChild(tree, 0));
    printf("("); 
    printExpr(getChild(tree, 1));
    printf(")"); 
    break;
  case Einstruct: 
    printExpr(getChild(tree, 0));
    printExpr(getChild(tree, 1));
    break;
  case Eselect: 
    printExpr(getChild(tree, 0));
    printf("."); 
    printExpr(getChild(tree, 1));
    break;
  case Emessage1:
    if(tree->nbChildren == 3){
        printExpr(getChild(tree, 0));
        printf("."); 
        printf("%s", getChild(tree,1)->u.str);
        printf("("); 
        printExpr(getChild(tree, 2));
        printf(")"); 
        break;
   }
   else {
        printExpr(getChild(tree, 0));
        printf("."); 
        printf("%s", getChild(tree,1)->u.str);
        break;
   }
  case Emessage2: 
    printf("%s", getChild(tree,0)->u.str);
    printf("."); 
    printf("%s", getChild(tree,1)->u.str);
    printf("("); 
    printExpr(getChild(tree, 2));
    printf(")"); 
    break;
  case Earg: 
    printExpr(getChild(tree, 0));
    printf(","); 
    printExpr(getChild(tree, 1));
    break;
  case Ebloc: 
    if(tree->nbChildren<3){
        printExpr(getChild(tree, 0));
        printf(" is "); 
        printExpr(getChild(tree, 1));
    }
    else{
        printf("{"); 
        printExpr(getChild(tree, 0));
        printExpr(getChild(tree, 1));
        printExpr(getChild(tree, 2));
        printf("}"); 
    }
    break;
  case Estring: 
    printf("%s", tree->u.str);
    break;
  case Einteger: 
    printf("%d", tree->u.val); 
    break;
  case Eobjet: 
    printf("def "); 
    printExpr(getChild(tree, 0));
    printf(" is "); 
    printExpr(getChild(tree, 1));
    break;
  case EBlocObjet: 
    printf("{"); 
    printExpr(getChild(tree, 0));
    printExpr(getChild(tree, 1));
    printExpr(getChild(tree, 2));
    printf("}"); 
    break;
  case EDeclObjet: 
    printf("object "); 
    printExpr(getChild(tree, 0));
    printf(" is "); 
    printExpr(getChild(tree, 1));
    break;
  case Eparam1: 
    printExpr(getChild(tree, 0));
    printf(","); 
    printExpr(getChild(tree, 1));
    break;
  case Eparam2: 
    printf("var "); 
    printExpr(getChild(tree, 0));
    printf(":"); 
    printExpr(getChild(tree, 1));
    break;
 case Eparam3: 
    printExpr(getChild(tree, 0));
    printf(":"); 
    printExpr(getChild(tree, 1));
    break;
  case Emeth: 
    printExpr(getChild(tree, 0));
    printExpr(getChild(tree, 1));
    break;
  case Emeth1: 
    printf("override def ");
    printExpr(getChild(tree, 0));
    printf("(");
    printExpr(getChild(tree, 1));
    printf("):");
    printExpr(getChild(tree, 2));
    printf(":=");
    printExpr(getChild(tree, 3));
    break;
  case Emeth2: 
    printf("def ");
    printExpr(getChild(tree, 0));
    printf("(");
    printExpr(getChild(tree, 1));
    printf("):");
    printExpr(getChild(tree, 2));
    printf(":=");
    printExpr(getChild(tree, 3));
    break;
  case Emeth3: 
    printf("override def ");
    printExpr(getChild(tree, 0));
    printf("(");
    printExpr(getChild(tree, 1));
    printf(")");
    printExpr(getChild(tree, 2));
    printf(" is ");
    printExpr(getChild(tree, 3));
    break;
  case Emeth4: 
    printf("def ");
    printExpr(getChild(tree, 0));
    printf("(");
    printExpr(getChild(tree, 1));
    printf(")");
    printExpr(getChild(tree, 2));
    printf(" is ");
    printExpr(getChild(tree, 3));
    break;
  case Econstructor: 
    if(tree->nbChildren<5){
        printf("def ");
        printExpr(getChild(tree, 0));
        printf("("); 
        printExpr(getChild(tree, 1));
        printf(") is ");
        printExpr(getChild(tree, 2));
    }
    else{
        printf("def "); 
        printExpr(getChild(tree, 0));
        printf("("); 
        printExpr(getChild(tree, 1));
        printf("):");
        printExpr(getChild(tree, 2));
        printf("("); 
        printExpr(getChild(tree, 3));
        printf(") is ");
        printExpr(getChild(tree, 4));
    }
    break;
  case Echamp1: 
    printExpr(getChild(tree, 0));
    printExpr(getChild(tree, 1));
    break;
  case Echamp2: 
    printf("var ");
    printExpr(getChild(tree, 0));
    printf(":");
    printExpr(getChild(tree, 1));
    printf(";");
    break;
  case Eclass: 
    if(tree->nbChildren<4){
        printf("class ");
        printExpr(getChild(tree, 0));
        printf("("); 
        printExpr(getChild(tree, 1));
        printf(") is ");
        printExpr(getChild(tree, 2));
  case EAND:
    printExpr(getChild(tree, 0));
    printf(" AND "); 
    printExpr(getChild(tree, 1));

    break;
    }
    else{
        printf("class "); 
        printExpr(getChild(tree, 0));
        printf("("); 
        printExpr(getChild(tree, 1));
        printf(") extends ");
        printExpr(getChild(tree, 2));
        printf(" is "); 
        printExpr(getChild(tree, 3));
    }
    break;
  case ETHIS: printf("this"); break;
  case ESUPER: printf("super"); break;
  case ERESULT: printf("result"); break;
  case Ereturn : printf("return ; "); break;
  default:
    fprintf(stderr, "Erreur! etiquette indefinie: %d\n", tree->op);
    abort();
  }
}
