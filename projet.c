#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "projet.h"
#include "projet_y.h"

extern int yyparse();
extern int yylineno;

extern char *strdup(const char *s);

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


/* mémorise le code d'erreur et s'arrange pour bloquer l'évaluation */
void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) { noEval = TRUE; }
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
  /* On continue meme en cas de double declaration, pour pouvoir eventuellement
   * detecter plus d'une erreur
   */
  nouv->rank = i; nouv->next=list;
  return nouv;
}


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
  case IDVAR :
    name = tree->u.str;
    for(p=lvar; p != NIL(VarDecl); p = p->next) {
      if (! strcmp(p->name, name)) { return TRUE; }
    }
    fprintf(stderr, "\nError: undeclared variable %s\n", name);
    setError(CONTEXT_ERROR);
    return FALSE;
  case CONST:
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


VarDeclP declVar(char *name, TreeP tree, VarDeclP decls) {
  VarDeclP pvar = NEW(1, VarDecl);
  pvar->name = name; pvar->next = NIL(VarDecl);
  checkScope(tree, decls);
  genCode(tree, decls);
  return addToScope(decls, pvar);
}


/* Constructeur de feuille dont la valeur est une chaine de caracteres
 * Si check vaut Vrai, Verifie si cet identificateur a bien ete declare.
 */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op); 
  tree->u.str = str;
  return(tree);
}


VarDeclP genCodeAff (TreeP tree, VarDeclP decls) {
  if (tree == NIL(Tree) || tree->op != DECL) {
    exit(UNEXPECTED);
  } else {
    /* Va laisser la valeur de la nouvelle variable en sommet de pile
     * donc rien de special a faire, puisqu'on va lui associer comme rang
     * cette adresse dans la pile
     */
    TreeP fils = getChild(tree, 1);
    return declVar(getChild(tree, 0)->u.str, fils, decls);
  }
}


VarDeclP genCodeDecls (TreeP tree) {
  if (tree == NIL(Tree)) {
    printf("START\n");    /* initialisation des registres internes */
    return NIL(VarDecl);
  }
  else {
    TreeP g, d; VarDeclP res;
    g = getChild(tree, 0);
    d = getChild(tree, 1);
    res = genCodeDecls(g);
    res = genCodeAff(d, res);
    return res;
  } 
}


/* generation de code pour un if then else 
 * le premier fils represente la condition,
 * les deux autres fils correspondent respectivement aux parties then et else.
 */
int genCodeIf(TreeP tree, VarDeclP decls) {
  char * etiFalse = getLabel();
  char * etiFin = getLabel();
  genCode(getChild(tree, 0), decls);
  printf("JZ %s\n", etiFalse);
  genCode(getChild(tree, 1), decls);
  printf("JUMP %s\n", etiFin);
  printf("%s: ", etiFalse);
  genCode(getChild(tree, 2), decls);
  printf("%s: NOP\n", etiFin);
  return 0;  
}


int getLocVar(char *name, VarDeclP decls) {
  VarDeclP l = decls;
  while (l != NIL(VarDecl)) {
    if (! strcmp(name, l->name)) { return l->rank; }
    else { l = l->next; }
  }
  if (errorCode == NO_ERROR) {
    	fprintf(stderr, "Unexpected error: undeclared variable %s\n", name);
	exit(UNEXPECTED);
  } else { return -1; /* code generation will not complete anyway */ }
}


/* generation de code pour une expression.
 * tree: l'AST de l'expression
 * decls: l'environnement, c'est à dire la liste des variables que l'expression
 * a le droit de referencer.
 */
int genCode(TreeP tree, VarDeclP decls) {
  if (tree == NIL(Tree)) { exit(UNEXPECTED); }
  switch (tree->op) {
 case IDVAR:
   printf("PUSHG %d \t-- %s\n", getLocVar(tree->u.str, decls), tree->u.str);
   break;
  case CONST:
    printf("PUSHI %d\n", tree->u.val);
    break;
  case EQ:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("EQUAL\n");
    break;
  case NE:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("EQUAL\nNOT\n");
    break;
  case GT:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("SUP\n");
    break;
  case GE:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("SUPEQ\n");
    break;
  case LT:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("INF\n");
    break;
  case LE:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("INFEQ\n");
    break;
  case Eadd:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("ADD\n");
    break;
  case Eminus:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("SUB\n");
    break;
  case Emult:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("MUL\n");
    break;
  case Ediv:
    genCode(getChild(tree, 0), decls); genCode(getChild(tree, 1), decls);
    printf("DUPN 1\nJZ DIV0\n");
    break;
  case ITE:
    genCodeIf(tree, decls);
    break;
  default: 
    fprintf(stderr, "Erreur! etiquette indefinie: %d\n", tree->op);
    exit(UNEXPECTED);
  }
  return 0;
}

/* generation de code pour l'expression finale du programme.
 * tree : expression comprise entre le BEGIN et le END
 * decls : l'environnement forme par les variables declarees
 */
int genCodeMain(TreeP tree, VarDeclP decls) {
  /* verifie les ident utilises dans l'expression finale */
  checkScope(tree, decls);
  if (noEval) {
    fprintf(stderr, "Skipping evaluation step.\n");
  } else {
    if (errorCode == NO_ERROR) {
      /* if errorCode != NO_ERROR then noEval shoud be equals to true */
      genCode(tree, decls);
      printf("PUSHS \"Resultat global: \"\nWRITES\nWRITEI\n");
      printf("PUSHS \"\\n\"\nWRITES\nSTOP\n");
      printf("DIV0: ERR \"Tentative de division par 0\"\n");
    } else { fprintf(stderr, "Code generation stopped because of errors\n"); }
  }
  return errorCode;
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
  if((! strcmp(nouv->baseClass->name, nouv->name) && (nouv->baseClass != NIL(Class)){
    fprintf(stderr, "Error: Multiple CLASS declaration in the same scope of %s\n", nouv->baseClass->name);
    setError(CONTEXT_ERROR);
    break;
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
  MethodeP meth; 
  int i;
  for(meth=m, i = 0; meth != NIL(Methode); meth = meth->next, i = i+1) {
    if (! strcmp(meth->name, nouv->name)) {
        fprintf(stderr, "Error: Multiple FIELD declaration in the same scope of %s\n", meth->name);
        setError(CONTEXT_ERROR);
        break;
    }
  }
  nouv->next=m;
  return nouv;
}

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
  if(param1 == NIL(VarDecl) && param2 != NIL(VarDecl))
    return FALSE;		/* l'un est vide et pas l'autre */
  if(param2 == NIL(VarDecl) && param1 != NIL(VarDecl))
    return FALSE;
  if(size(param1) != size(param2))
    return FALSE;		/* tester le nombre de parametre */

  VarDeclP tmp1;
  VarDeclP tmp2;
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
    return TRUE;
  }
 
  ClassP mere = classe->baseClass;
  while(mere != NIL(Class)){
    MethodP mp = containsMethod(m, classe->baseClass->listMethods);
    if(mp == NIL(Method)){  /* si la mere n'a pas la methode m */
      printf("%s doesn't have to be redefined\n", m->name);
      return FALSE;
    } 

    /* meme parametre et type de retour */
    if(compareParameterType(m->parameters, mp->parameters) == TRUE){
      if(strcmp(m->returnType->name, mp->returnType->name) != 0){
	printf("%s doesn't exist in super-class\n", m->name);
	return FALSE;
      }
    }
    else{
      return FALSE;
    } 
    mere = mere->baseClass;
  }
  return TRUE;
}

/* Verifier la surcharge de toutes les methodes dans la classe ou l'objet
   avant la generation du code
   Dans ce cas, la methode ne doit pas etre dans la liste des methodes de la classe 
   ou dans celle de la super class
*/
bool checkMethod(MethodP method, ClassP class)
{
  MethodP cont = containsMethod(method, class->listMethods);

  /* si la methode n'existe pas dans la classe */
  if(cont == NIL(Method)){
    return TRUE;
  }
  
  /* si elle y existe, verifier si C une surcharge ou redefinition */
  if(method->isOverride == FALSE){
    /* surcharge */
    if(compareParameterType(cont->parameters, method->parameters) == TRUE){
       printf("%s can't surcharge\n", method->name);
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
   ajout entete de method dans listMethod
 */
MethodP addMethod(MethodP method, MethodP listMethod)
{
  if(method == NIL(Method)) return listMethod;
  method->next = listMethod;
  return method;
}


/**
   Declaration d'une methode
*/
MethodP declMethode(char *name, ClassP returnType, VarDeclP parameters, TreeP body, ClassP home, bool isOverride, bool isConstructor, ClassP classe)
{
  MethodP pmethod = makeMethod(name, returnType, parameters, body, home, isOverride, isConstructor);
  if(checkMethod(pmethod, classe) == FALSE){
    exit(EXIT_FAILURE);
  }
  pmethod = addMethod(pmethod, classe->listMethods);
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




