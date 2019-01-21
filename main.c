#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "projet.h"
#include "projet_y.h"
#include "projet.h"
#include "genCode.h"

/* Niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Evaluation ou pas. Par defaut, on evalue les expressions */
bool noEval = FALSE;

/* code d'erreur a retourner: liste dans tp.h */
int errorCode = NO_ERROR;

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
