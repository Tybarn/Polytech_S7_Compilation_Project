Chiffre [0-9]
Entier {chiffre}+
LettreMin [a-z]
LettreMaj [A-Z]
Lettre {LettreMin}|{LettreMaj}
LC {Lettre}|{Chiffre}
Symbol [():,{};.]
Star \*
NotStar [^*]
NotStarSlash [^*/]


NomClasse {LettreMajLC*}
Delim [ \n\r\t]

%{
#include "projet.h"
#include "projet_y.h"
#include <string.h>

extern YYSTYPE yylval;
extern int yylineno;
extern char *strdup(const char *s);

int keywordOrId(){
    if (! strcmp(yytext, "if"))    return(IF);
    if (! strcmp(yytext, "then"))  return(THEN);
    if (! strcmp(yytext, "else"))  return(ELSE);
    if (! strcmp(yytext, "this"))  return(THIS);
    if (! strcmp(yytext, "class"))  return(CLASS);
    if (! strcmp(yytext, "def"))  return(DEF);
    if (! strcmp(yytext, "extends"))  return(EXTENDS);
    if (! strcmp(yytext, "is"))  return(IS);
    if (! strcmp(yytext, "new"))  return(NEW);
    if (! strcmp(yytext, "object"))  return(OBJECT);
    if (! strcmp(yytext, "this"))  return(THIS);
    if (! strcmp(yytext, "var"))  return(VAR);
    if (! strcmp(yytext, "override"))  return(OVERRIDE);
    if (! strcmp(yytext, "super"))  return(SUPER);
    if (! strcmp(yytext, "result"))  return(RESULT);
    if (! strcmp(yytext, "return"))  return(RETURN); 
    yylval.S = strdup(yytext);
    return(IDVAR);
}

void lexError(char c) {
  fprintf(stderr, "Error: Skipping invalid character '%c' on line %d\n",
          c, yylineno);
  setError(LEXICAL_ERROR);
}

%}

%%
{Delim}+ {/* comportement de l'analyseur en fonction de ce qu'il rencontre */ }
{Chiffre}* {yylval.I = strtol(yytext, NULL, 0); return(CST);}
{LettreMaj}{LC}*    { yylval.S = strdup(yytext); return(IDCLASS);}
{LettreMin}{LC}*    { return keywordOrId();}
":="    {return(AFF);}
"+" {return (ADD);}
"-"     {return (SUB);}
"*"     {return (MUL);}
"/"     {return (DIV);}
"="     { yylval.C = EQ; return(RELOP); }
"<>"    { yylval.C = NE; return(RELOP); }
">"     { yylval.C = GT; return(RELOP); }
">="    { yylval.C = GE; return(RELOP); }
"<"     { yylval.C = LT; return(RELOP); }
"<="    { yylval.C = LE; return(RELOP); }
"&"     { return(EAND); }
{Symbol} { return(yytext[0]); }
"/*"({NotStar}|{Star}+{NotStarSlash})*{Star}*"*/" { }
.       { lexError(yytext[0]); /* raler, l'ignorer et continuer */ }

