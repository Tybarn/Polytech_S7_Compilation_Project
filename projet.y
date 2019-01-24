%token CLASS DEF EXTENDS IS NEW OBJECT THIS VAR
%token OVERRIDE SUPER RESULT RETURN AND
%token IF THEN ELSE
%token INTEGER STRING
%token ADD SUB MUL DIV AFF
%token <C> RELOP
%token <S> IDVAR
%token <V> IDCLASS
%token <I> CST
%token <U> STR


%type <T> S LDeclCOOpt DeclClasse LParamClasseOpt LParamClasse ParamClasse BlocDeclClasse
%type <T> LDeclChampOpt LDeclChamp DeclChamp LDefMethode LDefMethodeOpt DefMethode
%type <T> NomClasseOpt LParamOpt LParam Param DeclObject BlocDeclObject DefConstructObject
%type <T> Bloc ContenueBloc ListeInstrOpt ListeInstr Instr ListeDecl Decl InitOpt ListeDeclOpt
%type <T> Expression E0 E1 E2 E3 Identificateur Cast Instanciation ListeArgOpt ListeArg Arg Message
%type <T> Cible DefConstructClasse


%right Else
%left ADD SUB 
%left MIL DIV
%left AND

%{
/*
#include "tp.h"  le fichier contenant la définition des types etc..*/
#include "projet.h"
#include <stdio.h>
extern int yylex();	/* fournie par Flex */
extern void yyerror();  /* definie dans tp.c */

%}

/*Définition de la grammaire*/
%%

/*
	CO = Classe et Object concernes
	L ou Liste = Liste, se répète
	Opt = Optionnel
	Param = Parametre
	Arg = Argument
	Champ = Champ d'une classe ou un objet
	Constr = Constructeur
	Decl = Declaration
*/

/* -------------------------- AXIOME -------------------------- */
S : LDeclCOOpt Bloc       {
printExpr($1);
printExpr($2);
}  /*Ici on aura le genCode()   {printf("S\n");}
;

/* -------------------------- PARTIE DECLARATION -------------------------- */

LDeclCOOpt :  DeclClasse LDeclCOOpt {$$ = makeTree(EDECL, 2, $1, $2);}
	| DeclObject LDeclCOOpt         {$$ = makeTree(EDECL, 2, $1, $2);}
	|                               {$$ = NIL(Tree);}
;

/* -------------------------- DECLARATION CLASSE -------------------------- */

DeclClasse : CLASS IDCLASS '(' LParamClasseOpt ')' EXTENDS IDCLASS IS BlocDeclClasse   {$$ = makeTree(Eclass, 4, makeLeafStr(EIDCLASS, $2), $4, makeLeafStr(EIDCLASS, $7),$9);}  
    | CLASS IDCLASS '(' LParamClasseOpt ')' IS BlocDeclClasse   {$$ = makeTree(Eclass, 3, makeLeafStr(EIDCLASS, $2), $4, $7);}  
;

LParamClasseOpt : LParamClasse        {$$ = $1;}  
	   |                              {$$ = NIL(Tree);}  
;

LParamClasse : ParamClasse ',' LParamClasse       {$$ = makeTree(Eparam1, 2, $1, $3);} /*modif  ok */
	  | ParamClasse                               {$$ = $1;}  
;

ParamClasse : VAR IDVAR ':' IDCLASS        {$$ = makeTree(Eparam2, 2,makeLeafStr(EIDVAR, $2), makeLeafStr(EIDCLASS, $4));} /*modif ok*/ 
        | Param                           {$$ = $1;}  
;

BlocDeclClasse : '{' LDeclChampOpt DefConstructClasse LDefMethodeOpt '}'      {$$ = makeTree(Ebloc, 3, $2,$3,$4);}  
;

LDeclChampOpt : LDeclChamp        {$$ = $1;}  
    |                             {$$ = NIL(Tree);}  
;

LDeclChamp :  DeclChamp LDeclChamp        {$$ = makeTree(Echamp1, 2, $1, $2);} /*modif ok*/ 
	| DeclChamp                           {$$ = $1;}  
;

DeclChamp : VAR IDVAR ':' IDCLASS ';'      {$$ = makeTree(Echamp2, 2, makeLeafStr(EIDVAR, $2), makeLeafStr(EIDCLASS, $4));} /*modif  ok*/
;

DefConstructClasse : DEF IDCLASS '(' LParamClasseOpt ')' IS Bloc          {$$ = makeTree(Econstructor, 3, makeLeafStr(EIDCLASS, $2), $4, $7);}  
    | DEF IDCLASS '(' LParamClasseOpt ')' ':' IDCLASS '(' ListeArgOpt ')' IS Bloc     {$$ = makeTree(Econstructor, 5, makeLeafStr(EIDCLASS, $2), $4, makeLeafStr(EIDCLASS, $7), $9, $12);}  
;

LDefMethodeOpt : LDefMethode      {$$ = $1;}  
	|                             {$$ = NIL(Tree);}  
;

LDefMethode : DefMethode LDefMethode          {$$ = makeTree(Emeth, 2, $1, $2);}  
	| DefMethode                              {$$ = $1;}  
;

/* modif Emetod à Emeth{1-4} */
DefMethode : OVERRIDE DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression      {$$ = makeTree(Emeth1, 4, makeLeafStr(EIDVAR, $3), $5, makeLeafStr(EIDCLASS, $8), $10);}  
	| DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression                      {$$ = makeTree(Emeth2, 4, makeLeafStr(EIDVAR, $2), $4, makeLeafStr(EIDCLASS, $7), $9);}  
	| OVERRIDE DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc       {$$ = makeTree(Emeth3, 4,makeLeafStr(EIDVAR, $3), $5, $7, $9);}  
	| DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc                {$$ = makeTree(Emeth4, 4, makeLeafStr(EIDVAR, $2), $4, $6, $8);}  
;

NomClasseOpt: ':' IDCLASS           {$$ = makeLeafStr(EIDCLASS, $2);}  
|                             {$$ = NIL(Tree);}  
;

LParamOpt : LParam            {$$ = $1;}  
	|                         {$$ = NIL(Tree);}  
;

LParam : Param ',' LParam          {$$ = makeTree(Eparam1, 2, $1, $3);} /*modif ok*/
		| Param                   {$$ = $1;}
;

Param : IDVAR ':' IDCLASS      {$$ = makeTree(Eparam3, 2,makeLeafStr(EIDVAR, $1), makeLeafStr(EIDCLASS, $3));}
;

/* -------------------------- DECLARATION OBJECT -------------------------- */

DeclObject : OBJECT IDCLASS IS BlocDeclObject     {$$ = makeTree(EDeclObjet, 2, makeLeafStr(EIDCLASS, $2), $4);}
;

BlocDeclObject : '{' LDeclChampOpt DefConstructObject LDefMethodeOpt '}'      {$$ = makeTree(EBlocObjet, 3, $2, $3, $4);}
;

DefConstructObject : DEF IDCLASS IS Bloc      {$$ = makeTree(Eobjet, 2, makeLeafStr(EIDCLASS, $2), $4);}
;

/* ------------------------- BLOC --------------------------- */ 

Bloc: '{' ContenueBloc '}'        {$$ = $2; }
;

ContenueBloc : ListeDecl IS ListeInstr    {$$ = makeTree(Ebloc, 2, $1, $3);}
	| ListeInstrOpt                       {$$ = $1;}
;

/* ------------------------ INSTRUCTIONS -------------------------- */

ListeInstrOpt : ListeInstr        {$$ = $1;}
	|                             {$$ = NIL(Tree);}
;

ListeInstr: Instr ListeInstr      {$$ = makeTree(Einstruct, 2, $1, $2);}
|Instr                            {$$ = $1;}
;

Instr : Expression ';'        {$$ = $1;}
    |   Bloc                  {$$ = $1;}
    |   RETURN ';'            {$$ = makeLeafStr(Ereturn, " ");}
    |   Cible AFF Expression';'   {$$ = makeTree(AFFECT,2,$1,$3);}
    |   IF Expression THEN Instr ELSE Instr    {$$ = makeTree(ITE,3,$2,$4,$6);}
;

/* ------------------------ DECLARATION DANS BLOC ------------------------- */

ListeDecl : Decl ListeDeclOpt   {$$ = makeTree(EDECL, 2, $1, $2);}
;

Decl : IDVAR ':' IDCLASS InitOpt ';'   {$$ = makeTree(EDECL, 3, makeLeafStr(EIDVAR, $1), makeLeafStr(EIDCLASS, $3), $4);}
;

InitOpt : AFF Expression     {$$ = makeTree(AFFECT,1,$2);}
	|                           { $$ = NIL(Tree);}
;

ListeDeclOpt : ListeDecl     {$$ = $1;}
    |                       { $$ = NIL(Tree);}
;

/* ------------------------ EXPRESSION ------------------------ */

Expression : E0 RELOP E0   {$$ = makeTree($2, 2, $1, $3);}
	| E0   {$$ = $1;}
;

E0 : E0 AND E1   {$$ = makeTree(EAND, 2, $1, $3);}
	| E0 ADD E1   {$$ = makeTree(Eadd, 2, $1, $3);}
	| E0 SUB E1   {$$ = makeTree(Eminus, 2, $1, $3);}
	| E1   {$$ = $1;}
;

E1 : E1 MUL E2   {$$ = makeTree(Emult, 2, $1, $3);}
	| E1 DIV E2   {$$ = makeTree(Ediv, 2, $1, $3);}
	| E2   {$$ = $1;}
;

E2 : ADD E3   {$$ = makeTree(Eadd, 1, $2);}
	| SUB E3   {$$ = makeTree(Eminus, 1, $2);}
	| E3	    {$$ = $1;} /*modif ok*/
;

E3 :  
    Instanciation       {$$ = $1;} /*modif ok*/
    | Message           {$$ = $1;} /*modif revue */
;

Identificateur : THIS     {$$ = makeLeafStr(ETHIS, " ");}
	| SUPER               {$$ = makeLeafStr(ESUPER, " ");}
	| RESULT              {$$ = makeLeafStr(ERESULT, " ");}
;


Cast : '(' IDCLASS Expression ')'     {$$ = makeTree(Ecast, 2, makeLeafStr(EIDCLASS, $2), $3);}
;

Instanciation : NEW IDCLASS '(' ListeArgOpt ')'   {$$ = makeTree(Einstan, 2, makeLeafStr(EIDCLASS, $2), $4);}
;

ListeArgOpt : ListeArg    {$$ = $1;}
    |                     { $$ = NIL(Tree);}
;

ListeArg : Arg ',' ListeArg   {$$ = makeTree(Earg, 2, $1, $3);}
    |   Arg                   {$$ = $1;}
;

Arg : Expression              {$$ = $1;}
;

Message : Message '.' IDVAR  '(' ListeArgOpt ')'      {$$ = makeTree(Emessage1, 3, $1, makeLeafStr(EIDVAR, $3),$5);}
    |IDCLASS '.' IDVAR  '(' ListeArgOpt ')'	      {$$ = makeTree(Emessage2, 3, makeLeafStr(EIDCLASS, $1), makeLeafStr(EIDVAR, $3),$5);} /*modif ok*/
    | Cible					      {$$ = $1;} /*modif ok*/
;


Cible : Message '.' IDVAR                             { $$ = makeTree(Emessage1, 2, $1, makeLeafStr(EIDVAR, $3));}
    | IDVAR               {$$ = makeLeafStr(EIDVAR, $1);} 
    | Identificateur       {$$ = $1;}
    | CST                  { $$ = makeLeafInt(ECST, $1);}
	| '(' Expression ')'  {$$ = $2;}
	| Cast                {$$ = makeTree(Ecast, 1, $1);}
    | STR			     {$$ = makeLeafStr(Estring, $1);} /*modif revue*/
;
