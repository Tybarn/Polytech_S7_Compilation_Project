%token CLASS DEF EXTENDS IS NEW OBJECT THIS VAR
%token OVERRIDE SUPER RESULT RETURN AND
%token IF THEN ELSE
%token INTEGER STRING
%token ADD SUB MUL DIV AFF
%token <C> RELOP
%token <S> IDVAR
%token <V> IDCLASS
%token <I> CST

/*
%type <T> S LDeclCOOpt DeclClasse LParamClasseOpt LParamClasse ParamClasse BlocDeclClasse
%type <T> LDeclChampOpt LDeclChamp DeclChamp DefConstructClasse LDefMethodeOpt DefMethode
%type <T> NomClasseOpt LParamOpt LParam Param DeclObject BlocDeclObject DefConstructObject
%type <T> TypeCO Bloc ContenueBloc ListeInstrOpt ListeInstr Instr ListeDecl Decl InitOpt ListeDeclOpt
%type <T> Expression E0 E1 E2 E3 Identificateur Cast Instanciation ListeArgOpt ListeArg Arg Message
*/

%right Else
%left ADD SUB 
%left MIL DIV
%left AND

%{
/*
#include "tp.h"  /*le fichier contenant la définition des types etc..*/

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

S : LDeclCOOpt Bloc
;

/* -------------------------- PARTIE DECLARATION -------------------------- */

LDeclCOOpt :  DeclClasse LDeclCOOpt
	| DeclObject LDeclCOOpt
	| 
;

/* -------------------------- DECLARATION CLASSE -------------------------- */

DeclClasse : CLASS IDCLASS '(' LParamClasseOpt ')' EXTENDS TypeCO IS BlocDeclClasse
    | CLASS IDCLASS '(' LParamClasseOpt ')' IS BlocDeclClasse//TODO
;

LParamClasseOpt : LParamClasse
	   | 
;

LParamClasse : ParamClasse ',' LParamClasse
	  | ParamClasse
;

ParamClasse : VAR IDVAR ':' TypeCO
        | Param
;

BlocDeclClasse : '{' LDeclChampOpt DefConstructClasse LDefMethodeOpt '}'
;

LDeclChampOpt : LDeclChamp
    |
;

LDeclChamp :  DeclChamp LDeclChamp
	| DeclChamp
;

DeclChamp : VAR IDVAR ':' TypeCO ';'
;

DefConstructClasse : DEF IDCLASS '(' LParamClasseOpt ')' IS Bloc
    | DEF IDCLASS '(' LParamClasseOpt ')' ':' IDCLASS '(' ListeArgOpt ')' IS Bloc
;

LDefMethodeOpt : LDefMethode
	|
;

LDefMethode : DefMethode LDefMethode
	| DefMethode
;

DefMethode : OVERRIDE DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression
	| DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression
	| OVERRIDE DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc
	| DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc
;

NomClasseOpt:IDCLASS
	|
;

LParamOpt : LParam
	|
;

LParam : Param ',' LParam
		| Param
;

Param : IDVAR ':' TypeCO
;

/* -------------------------- DECLARATION OBJECT -------------------------- */

DeclObject : OBJECT IDCLASS IS BlocDeclObject
;

BlocDeclObject : '{' LDeclChampOpt DefConstructObject LDefMethodeOpt '}'
;

DefConstructObject : DEF IDCLASS IS Bloc
;

/* -------------------------- AUTRE --------------------------- */

TypeCO : INTEGER
	| STRING
	| IDCLASS
;

/* ------------------------- BLOC --------------------------- */ 

Bloc: '{' ContenueBloc '}'
;

ContenueBloc : ListeDecl IS ListeInstr 
	| ListeInstrOpt
;

/* ------------------------ INSTRUCTIONS -------------------------- */

ListeInstrOpt : ListeInstr
	|
;

ListeInstr: Instr ListeInstr
	|Instr
;

Instr : Expression ';'
    |   Bloc
    |   RETURN ';'
    |   IDVAR AFF Expression';'
    |   IF Expression THEN '{' ListeInstrOpt '}' ELSE '{' ListeInstrOpt '}'
;

/* ------------------------ DECLARATION DANS BLOC ------------------------- */

ListeDecl : Decl ListeDeclOpt /*{$$ = makeTree(DECL, 2, $1, $2);}*/
;

Decl : IDVAR ':' IDCLASS InitOpt ';' /*{$$ = makeTree(DECL, 3, $1, $3, $4);}*/
;

InitOpt : AFF Expression   /*{$$ = makeTree(AFF,1,$2);}*/
	|
;

ListeDeclOpt : ListeDecl   /*{$$ = $1;}*/
    |                     /*{ $$ = NIL(Tree);}*/
;

/* ------------------------ EXPRESSION ------------------------ */

Expression : E0 RELOP E0 /*{$$ = makeTree($2, 2, $1, $3);}*/
	| E0 /*{$$ = $1;}*/
;

E0 : E0 '&' E1 /*{$$ = makeTree($2, 2, $1, $3);}*/
	| E0 ADD E1 /*{$$ = makeTree(Eadd, 2, $1, $3);}*/
	| E0 SUB E1 /*{$$ = makeTree(Eminus, 2, $1, $3);}*/
	| E1 /*{$$ = $1;}*/
;

E1 : E1 MUL E2 /*{$$ = makeTree(Emult, 2, $1, $3);}*/
	| E1 DIV E2 /*{$$ = makeTree(Ediv, 2, $1, $3);}*/
	| E2 /*{$$ = $1;}*/
;

E2 : ADD E3 /*{$$ = makeTree(Eadd, 1, $2);}*/
	| SUB E3 /*{$$ = makeTree(Eminus, 1, $2);}*/
	| E3
;

E3 : CST                /*{ $$ = makeLeafInt(CONST, $1);}*/
	| IDVAR             /*{$$ = makeLeafStr(IDVAR, $1);}*/ 
	| '(' Expression ')'/*{$$ = $2;}*/
	| Identificateur    /*{$$ = $1;}*/
	| Cast              /*{$$ = makeTree(Ecast, 1, $1);}*/
	| Instanciation     /*{$$ = makeTree(Einstan, 1, $1);}*/
	| Message
;

Identificateur : THIS
	| SUPER
	| RESULT
;


Cast : '(' IDCLASS Expression ')'   /*{$$ = makeTree(Ecast, 2, $2, $3);}*/
;

Instanciation : NEW IDCLASS '(' ListeArgOpt ')' /*{$$ = makeTree(Einstan, 2, $2, $4);}*/
;

ListeArgOpt : ListeArg  /*{$$ = $1;}*/
    |                   /*{ $$ = NIL(Tree);}*/
;

ListeArg : Arg ',' ListeArg /*{$$ = makeTree(Earg, 2, $1, $3);}*/
    |   Arg                 /*{$$ = $1;}*/
;

Arg : Expression            /*{$$ = $1;}*/
;

Message : E3 '.' IDVAR  '(' ListeArgOpt ')'   /* {$$ = makeTree(Emessage, 3, $1, $3,$5);}*/
    |   E3 '.' IDVAR                          /* { $$ = makeTree(Eselect, 2, $1, $3);}*/
;
