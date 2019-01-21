#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "genCode.h"


/* ************************ GENERATION DE CODE ************************* */

/*    
VarDeclP genCodeAff (TreeP tree, VarDeclP decls) {
  if (tree == NIL(Tree) || tree->op != EDECL) {
    exit(UNEXPECTED);
  } else {
    TreeP fils = getChild(tree, 1);
    return declVar(getChild(tree, 0)->u.str, fils, decls);
  }
}


VarDeclP genCodeDecls (TreeP tree) {
  if (tree == NIL(Tree)) {
    printf("START\n");   
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
*/

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


/*
int getLocVar(char *name, VarDeclP decls) {
  VarDeclP l = decls;
  while (l != NIL(VarDecl)) {
    if (! strcmp(name, l->name)) { return l->rank; }
    else { l = l->next; }
  }
  if (errorCode == NO_ERROR) {
    fprintf(stderr, "Unexpected error: undeclared variable %s\n", name);
    exit(UNEXPECTED);
  } else { return -1; }
}
*/

/* generation de code pour une expression.
 * tree: l'AST de l'expression
 * decls: l'environnement, c'est à dire la liste des variables que l'expression
 * a le droit de referencer.
 */
int genCode(TreeP tree, VarDeclP decls) {
  if (tree == NIL(Tree)) { exit(UNEXPECTED); }
  switch (tree->op) {
  case EIDVAR:
    //printf("PUSHG %d \t-- %s\n", getLocVar(tree->u.str, decls), tree->u.str);
    break;
  case ECST:
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
  case EDECL :
    break;
  case EIDCLASS :
    break;
  case AFFECT: 
    break;
  case EAND: 
    break;
  case Ecast: 
    break;
  case Einstan: 
    break;
  case Einstruct: 
    break;
  case Eselect: 
    break;
  case Emessage: 
    break;
  case Earg: 
    break;
  case Ebloc: 
    break;
  case Estring: 
    break;
  case Einteger: 
    break;
  case Eobjet: 
    break;
  case EBlocObjet: 
    break;
  case EDeclObjet: 
    break;
  case Eparam: 
    break;
  case Emeth: 
    break;
  case Econstructor: 
    break;
  case Echamp: 
    break;
  case Eclass: 
    break;
  case Eaxiome: 
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


