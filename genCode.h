#ifndef GENCODE_H_
#define GENCODE_H_

/* gestion des declarations et traitement de la portee */
/*VarDeclP addToScope(VarDeclP list, VarDeclP nouv);*/
/*VarDeclP declVar(char *name, TreeP tree, VarDeclP decls);*/


VarDeclP genCodeDecls (TreeP tree);
int genCode(TreeP tree, VarDeclP decls);
int genCodeMain(TreeP tree, VarDeclP decls);


ClassP declClass(char* name, VarDeclP listParam, ClassP classeMere, VarDeclP listChamp, MethodP listMethod);
ClassP addClass(ClassP cl, ClassP nouv);
ClassP declObject(char* name, VarDeclP listParam, ClassP classeMere, VarDeclP listChamp, MethodP listMethod);
VarDeclP declChamp(char *name, ClassP type, bool isVar, bool isAttribute);
VarDeclP addChamp(VarDeclP ch, VarDeclP nouv);
MethodP declMethode(char *name, ClassP returnType, VarDeclP parameters, TreeP body, ClassP home, bool isOverride, bool isConsructor);
MethodP addMethod(MethodP m, MethodP nouv);
VarDeclP declParam(char* name, ClassP type, bool isVar, bool isAttribute);
VarDeclP addParam(VarDeclP list, VarDeclP nouv);


#endif
