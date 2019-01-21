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
#include "projet.h"
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

S : LDeclCOOpt Bloc         /*Ici on aura le genCode()*/
;

/* -------------------------- PARTIE DECLARATION -------------------------- */

LDeclCOOpt :  DeclClasse LDeclCOOpt /*{$$ = makeTree(EDECL, 2, $1, $2);}*/
	| DeclObject LDeclCOOpt         /*{$$ = makeTree(EDECL, 2, $1, $2);}*/
	|                               /*{$$ = NIL(Tree);}*/
;

/* -------------------------- DECLARATION CLASSE -------------------------- */

DeclClasse : CLASS IDCLASS '(' LParamClasseOpt ')' EXTENDS TypeCO IS BlocDeclClasse /*{$$ = makeTree(Eclass, 4, $2, $4, $7,$9);}*/
    | CLASS IDCLASS '(' LParamClasseOpt ')' IS BlocDeclClasse /*{$$ = makeTree(Eclass, 3, $2, $4, $7);}*/
;

LParamClasseOpt : LParamClasse      /*{$$ = $1;}*/
	   |                            /*{$$ = NIL(Tree);}*/
;

LParamClasse : ParamClasse ',' LParamClasse     /*{$$ = makeTree(Eparam, 2, $1, $3);}*/
	  | ParamClasse                             /*{$$ = $1;}*/
;

ParamClasse : VAR IDVAR ':' TypeCO      /*{$$ = makeTree(Eparam, 2, $2, $4);}*/
        | Param                         /*{$$ = $1;}*/
;

BlocDeclClasse : '{' LDeclChampOpt DefConstructClasse LDefMethodeOpt '}'    /*{$$ = makeTree(Ebloc, 3, $2,$3,$4);}*/
;

LDeclChampOpt : LDeclChamp      /*{$$ = $1;}*/
    |                           /*{$$ = NIL(Tree);}*/
;

LDeclChamp :  DeclChamp LDeclChamp      /*{$$ = makeTree(Echamp, 2, $1, $2);}*/
	| DeclChamp                         /*{$$ = $1;}*/
;

DeclChamp : VAR IDVAR ':' TypeCO ';'    /*{$$ = makeTree(Echamp, 2, $2, $4);}*/
;

DefConstructClasse : DEF IDCLASS '(' LParamClasseOpt ')' IS Bloc        /*{$$ = makeTree(Econstructor, 3, $2, $4, $6);}*/
    | DEF IDCLASS '(' LParamClasseOpt ')' ':' IDCLASS '(' ListeArgOpt ')' IS Bloc   /*{$$ = makeTree(Econstructor, 5, $2, $4, $7, $9, $12);}*/
;

LDefMethodeOpt : LDefMethode    /*{$$ = $1;}*/
	|                           /*{$$ = NIL(Tree);}*/
;

LDefMethode : DefMethode LDefMethode        /*{$$ = makeTree(Emeth, 2, $1, $2);}*/
	| DefMethode                            /*{$$ = $1;}*/
;

DefMethode : OVERRIDE DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression    /*{$$ = makeTree(Emeth, 4, $3, $5, $8, $10);}*/
	| DEF IDVAR '(' LParamOpt ')' ':' IDCLASS AFF Expression                    /*{$$ = makeTree(Emeth, 4, $2, $4, $7, $9);}*/
	| OVERRIDE DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc     /*{$$ = makeTree(Emeth, 4, $3, $5, $7, $9);}*/
	| DEF IDVAR '(' LParamOpt ')' NomClasseOpt IS Bloc              /*{$$ = makeTree(Emeth, 4, $2, $4, $6, $8);}*/
;

NomClasseOpt:IDCLASS         /*{$$ = makeLeafStr(EIDCLASS, $1);}*/
|                           /*{$$ = NIL(Tree);}*/
;

LParamOpt : LParam          /*{$$ = $1;}*/
	|                       /*{$$ = NIL(Tree);}*/
;

LParam : Param ',' LParam       /* {$$ = makeTree(Eparam, 2, $1, $3);}*/
		| Param                 /*{$$ = $1;}*/
;

Param : IDVAR ':' TypeCO    /*{$$ = makeTree(Eparam, 2, $1, $3);}*/
;

/* -------------------------- DECLARATION OBJECT -------------------------- */

DeclObject : OBJECT IDCLASS IS BlocDeclObject   /*{$$ = makeTree(EDeclObjet, 2, $2, $3);}*/
;

BlocDeclObject : '{' LDeclChampOpt DefConstructObject LDefMethodeOpt '}'    /*{$$ = makeTree(EBlocObjet, 3, $2, $3, $4);}*/
;

DefConstructObject : DEF IDCLASS IS Bloc    /*{$$ = makeTree(Eobjet, 2, $2, $4);}*/
;

/* -------------------------- AUTRE --------------------------- */

TypeCO : INTEGER    /*{$$ = makeLeafStr(Einteger, $1);} */
	| STRING        /*{$$ = makeLeafStr(Estring, $1);} */
	| IDCLASS       /*{$$ = makeLeafStr(EIDCLASS, $1);} */
;

/* ------------------------- BLOC --------------------------- */ 

Bloc: '{' ContenueBloc '}'      /*{$$ = $1;}*/
;

ContenueBloc : ListeDecl IS ListeInstr  /*{$$ = makeTree(Ebloc, 2, $1, $3);}*/
	| ListeInstrOpt                     /*{$$ = $1;}*/
;

/* ------------------------ INSTRUCTIONS -------------------------- */

ListeInstrOpt : ListeInstr      /*{$$ = $1;}*/
	|                           /*{$$ = NIL(Tree);}*/
;

ListeInstr: Instr ListeInstr    /*{$$ = makeTree(Einstruct, 2, $1, $2);}*/
|Instr                          /*{$$ = $1;}*/
;

Instr : Expression ';'      /*{$$ = $1;}*/
    |   Bloc                /*{$$ = $1;}*/
    |   RETURN ';'          /*{$$ = makeLeafStr(Ereturn, $1);}*/
    |   IDVAR AFF Expression';' /*{$$ = makeTree(AFFECT,2,$1,$3);}*/
    |   IF Expression THEN '{' ListeInstrOpt '}' ELSE '{' ListeInstrOpt '}' /*{$$ = makeTree(ITE,3,$2,$5,$9);}*/
;

/* ------------------------ DECLARATION DANS BLOC ------------------------- */

ListeDecl : Decl ListeDeclOpt /*{$$ = makeTree(EDECL, 2, $1, $2);}*/
;

Decl : IDVAR ':' IDCLASS InitOpt ';' /*{$$ = makeTree(EDECL, 3, $1, $3, $4);}*/
;

InitOpt : AFF Expression   /*{$$ = makeTree(AFFECT,1,$2);}*/
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
	| IDVAR             /*{$$ = makeLeafStr(EIDVAR, $1);}*/ 
	| '(' Expression ')'/*{$$ = $2;}*/
	| Identificateur    /*{$$ = $1;}*/
	| Cast              /*{$$ = makeTree(Ecast, 1, $1);}*/
	| Instanciation     /*{$$ = makeTree(Einstan, 1, $1);}*/
	| Message
;

Identificateur : THIS   /*{$$ = makeLeafStr(ETHIS, $1);}*/
	| SUPER             /*{$$ = makeLeafStr(ESUPER, $1);}*/
	| RESULT            /*{$$ = makeLeafStr(ERESULT, $1);}*/
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
