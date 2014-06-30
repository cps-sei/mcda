%{
#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <boost/foreach.hpp>
#include "daig/Type.h"
#include "daig/Variable.h"
#include "daig/Expression.h"
#include "daig/Statement.h"
#include "daig/Function.h"
#include "daig/Node.h"
#include "DaigBuilder.hpp"
#include <math.h>
#

extern daig::DaigBuilder *builder; /* the dag builder */

extern int yylex();
void yyerror(const char *s) {
  printf("ERROR: %s. Rerun with --debug for more details\n", s);
  exit(1);
}

/** the current node being parsed */
daig::Node currNode;

/** a thunk */
std::string thunk;

#define printExpr(_x) if(builder->debug) printf("EXPR: %s\n",(_x)->toString().c_str())
#define MAKE_NULL(_res,_o) _res = new daig::Expr(new daig::CompExpr(_o)); printExpr(*_res)
#define MAKE_UN(_res,_o,_l) _res = new daig::Expr(new daig::CompExpr(_o,*_l)); delete _l; printExpr(*_res)
#define MAKE_BIN(_res,_o,_l,_r) _res = new daig::Expr(new daig::CompExpr(_o,*_l,*_r)); delete _l; delete _r; printExpr(*_res)
#define MAKE_TRI(_res,_o,_a,_b,_c) _res = new daig::Expr(new daig::CompExpr(_o,*_a,*_b,*_c)); delete _a; delete _b; delete _c; printExpr(*_res)
#define printStmt(_x) if(builder->debug) printf("STMT: %s\n",(_x)->toString().c_str())
%}

/* expect 5 shift reduce conflicts */
%expect 5

/* Represents the many different ways we can access our data */
%union {
    daig::Type *type;
    daig::Variable *var;
    daig::VarList *varList;
    daig::LvalExpr *lvalExpr;
    daig::Expr *expr;
    daig::ExprList *exprlist;
    daig::Stmt *stmt;
    daig::StmtList *stmtList;
    std::list<int> *intList;
    std::string *string;
    std::list<std::string> *strList;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TNAMESPACE
%token <token> TMOCSYNC TMOCASYNC TMOCPSYNC TSEMICOLON TCONST TNODE
%token <token> TGLOBAL TLOCAL TTARGET TTHUNK
%token <token> TBOOL TINT TDOUBLE_TYPE TVOID TCHAR TSIGNED TUNSIGNED
%token <token> TNODENUM TATOMIC TPRIVATE TEXTERN
%token <token> TIF TELSE TFOR TWHILE
%token <token> TBREAK TCONTINUE TRETURN TEXO TEXH TEXL TPROGRAM
%token <token> TINIT TSAFETY TFAN TFADNP TFAO TFAOL TFAOH
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLAND TLOR TLNOT TQUEST TCOLON
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE 
%token <token> TLBRACKET TRBRACKET TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV TMOD
%token <token> TBWNOT TBWAND TBWOR TBWXOR TBWLSH TBWRSH
%token <token> TON_PRE_TIMEOUT TON_POST_TIMEOUT TON_RECV_FILTER
%token <token> TTRACK_LOCATIONS
%token <token> TNODE_INIT TPERIODIC

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <token> program moc const_list constant node node_body
%type <token> node_body_elem_list node_body_elem
%type <token> global_var local_var dimension
%type <strList> target_id_list
%type <type> simp_type type
%type <lvalExpr> lval
%type <expr> expr
%type <exprlist> indices arg_list for_test
%type <stmt> stmt
%type <stmtList> stmt_list for_init for_update
%type <intList> dimensions
%type <var> var
%type <varList> var_list var_decl param_list var_decl_list
%type <string> callback_name

/* Operator precedence for ternary operators */
%left TQUEST TCOLON
/* Operator precedence for logical operators */
%left TLOR
%left TLAND
/* Operator precedence for bitwise operators */
%left TBWOR
%left TBWXOR
%left TBWAND
/* Operator precedence for comparison operators */
%left TCEQ TCNE
%left TCLT TCLE TCGT TCGE
/* Operator precedence for mathematical operators */
%left TBWLSH TBWRSH
%left TPLUS TMINUS
%left TMUL TDIV TMOD
/* precedence for logical not */
%right TLNOT TBWNOT

%start program

%%
program :
  moc target_list callback_list const_list extern_fn_list node prog_def init_def safety_def {}
| moc TTRACK_LOCATIONS TSEMICOLON target_list callback_list const_list extern_fn_list node prog_def init_def safety_def {
  builder->program.trackLocations=true;
}
;

moc : 
  TMOCSYNC TSEMICOLON { builder->program.moc.set_type("SYNC"); }
| TMOCASYNC TSEMICOLON { builder->program.moc.set_type("ASYNC"); }
| TMOCPSYNC TSEMICOLON { builder->program.moc.set_type("PARTIAL"); }
;

target_list : {}
| TTARGET target_id_list TTHUNK {
  BOOST_FOREACH(const std::string &t,*$2) builder->program.addTarget(t,thunk);
  delete $2;
}
| target_list TTARGET target_id_list TTHUNK {
  BOOST_FOREACH(const std::string &t,*$3) builder->program.addTarget(t,thunk);
  delete $3;
}
;

target_id_list : TIDENTIFIER {
  $$ = new std::list<std::string>();
  $$->push_back(*$1); delete $1;
}
| target_id_list TCOMMA TIDENTIFIER {
  $$ = $1;
  $$->push_back(*$3); delete $3;
}
;

callback_name : TIDENTIFIER { $$ = $1; }
| TNAMESPACE { $$ = $1; }
;

callback_list : {}
| callback_list TON_PRE_TIMEOUT TLPAREN callback_name TRPAREN TSEMICOLON {
  builder->program.addCallback("on_pre_round_barrier_timeout", *$4);
  delete $4;
}
| callback_list TON_POST_TIMEOUT TLPAREN callback_name TRPAREN TSEMICOLON {
  builder->program.addCallback("on_post_round_barrier_timeout", *$4);
  delete $4;
}
| callback_list TON_RECV_FILTER TLPAREN callback_name TRPAREN TSEMICOLON {
  builder->program.addCallback("on_receive_filter", *$4);
  delete $4;
}
;

const_list : {}
| constant {}
| const_list constant {}
;

extern_fn_list : {}
| extern_fn_list extern_fn_decl
;

extern_fn_decl : TEXTERN type TIDENTIFIER TLPAREN param_list TRPAREN TSEMICOLON {
  builder->program.addExternalFunction(daig::Function(*$2,*$3,*$5,daig::VarList(),daig::StmtList()));
  delete $2; delete $3; delete $5;
}
;

constant : TCONST TIDENTIFIER TEQUAL TINTEGER TSEMICOLON {
  builder->program.constDef[*$2] = *$4;
  delete $2; delete $4;
}
/** this is needed to handle constant definitions that have also been
supplied via the command line */
| TCONST TINTEGER TEQUAL TINTEGER TSEMICOLON {
  delete $2; delete $4;
}
| TCONST TIDENTIFIER TEQUAL TDOUBLE TSEMICOLON {
builder->program.constDef[*$2] = *$4;
delete $2; delete $4;
}
;

node : TNODE TIDENTIFIER TLPAREN TIDENTIFIER TRPAREN TLBRACE node_body TRBRACE {
  currNode.name = *$2;
  currNode.args.push_back(*$4);
  builder->program.nodes[currNode.name] = currNode;
  currNode.clear();
  delete $2; delete $4;
};

node_body : node_body_elem_list {};

node_body_elem_list :
  node_body_elem {}
| node_body_elem_list node_body_elem {}
;

node_body_elem :
  global_var {}
| local_var {}
| node_init_procedure {}
| periodic_procedure {}
| procedure {}
;

global_var : TGLOBAL var_decl TSEMICOLON {
  currNode.addGlobalVar(*$2); delete $2;
}
;

local_var : TLOCAL var_decl TSEMICOLON {
  currNode.addLocalVar(*$2); delete $2;
}
;

var_decl : type var_list {
  $$ = new daig::VarList();
  BOOST_FOREACH(const daig::Variable &v,*$2) {
    daig::BaseType *t = new daig::BaseType(**$1);
    t->dims = v.type->dims;
    daig::Variable newVar = v;
    newVar.type = daig::Type(t);
    $$->push_back(newVar);
  }
  delete $1; delete $2;
};

var_list : var {
  $$ = new daig::VarList(); $$->push_back(*$1); delete $1;
}
| var_list TCOMMA var { $$ = $1; $$->push_back(*$3); delete $3; }
;

var : TIDENTIFIER { $$ = new daig::Variable(*$1); delete $1; }
| TIDENTIFIER dimensions {
  $$ = new daig::Variable(*$1,*$2); 
  delete $1; delete $2;
}
;

dimensions : TLBRACKET dimension TRBRACKET {
  $$ = new std::list<int>(); $$->push_back($2);
}
| dimensions TLBRACKET dimension TRBRACKET { $$ = $1; $$->push_back($3); }
;

/* NODENUM is indicated by dimension -1 */
dimension : TINTEGER { $$ = atoi($1->c_str()); delete $1; }
| TNODENUM { $$ = -1; }
;

type : simp_type { $$ = $1; }
| TSIGNED simp_type { $$ = $2; (*$$)->setQual(TSIGNED); }
| TUNSIGNED simp_type { $$ = $2; (*$$)->setQual(TUNSIGNED); }
;

simp_type : TBOOL { $$ = new daig::Type(daig::boolType()); }
| TINT { $$ = new daig::Type(daig::intType()); }
| TDOUBLE_TYPE { $$ = new daig::Type(daig::doubleType()); }
| TVOID { $$ = new daig::Type(daig::voidType()); }
| TCHAR { $$ = new daig::Type(daig::charType()); }
;

node_init_procedure : TVOID TNODE_INIT TLPAREN TRPAREN TLBRACE var_decl_list stmt_list TRBRACE {
  /** set scope of temporary variables */
  BOOST_FOREACH(daig::Variable &v,*$6) v.scope = daig::Variable::TEMP;
  /** create and set node initialization function */
  /*boost::shared_ptr<daig::Function> f (new daig::Function(daig::voidType(),"NODE_INIT",daig::VarList(),*$6,*$7));*/
  daig::Function f = daig::Function(daig::voidType(),"NODE_INIT",daig::VarList(),*$6,*$7);
  currNode.setNodeInitFunction(f);
  delete $6; delete $7;
}
;

periodic_procedure : TPERIODIC TLPAREN TINTEGER TRPAREN TVOID TIDENTIFIER TLPAREN TRPAREN TLBRACE var_decl_list stmt_list TRBRACE {
  /** declare PERIOD as function's local variable */
  /*boost::shared_ptr<daig::Variable> p (new daig::Variable("PERIOD", daig::intType()));*/
  daig::Variable p = daig::Variable("PERIOD", daig::intType());
  $10->push_back(p);
  /** assign value to PERIOD */
  const daig::Expr l (new daig::LvalExpr("PERIOD"));
  const daig::Expr r (new daig::IntExpr(atoi($3->c_str())));
  const daig::Stmt s (new daig::AsgnStmt(l, r));
  $11->push_front(s);
  /** set scope of temporary variables */
  BOOST_FOREACH(daig::Variable &v,*$10) v.scope = daig::Variable::TEMP;
  /** create and add periodic function to the node */
  /*boost::shared_ptr<daig::Function> f (new daig::Function(daig::voidType(),*$6,daig::VarList(),*$10,*$11));*/
  daig::Function f = daig::Function(daig::voidType(),*$6,daig::VarList(),*$10,*$11);
  currNode.addPeriodicFunction(f, atoi($3->c_str()));
  delete $3; delete $6; delete $10; delete $11;
}
;

procedure : type TIDENTIFIER TLPAREN param_list TRPAREN TLBRACE var_decl_list stmt_list TRBRACE {
  /** set scope of parameters and temporary variables */
  BOOST_FOREACH(daig::Variable &v,*$4) v.scope = daig::Variable::PARAM;
  BOOST_FOREACH(daig::Variable &v,*$7) v.scope = daig::Variable::TEMP;
  /** create and add function to the node */
  currNode.addFunction(daig::Function(*$1,*$2,*$4,*$7,*$8));
  delete $1; delete $2; delete $4; delete $7; delete $8;       
}
;

param_list : { $$ = new daig::VarList(); }
| type var {
  $$ = new daig::VarList();
  daig::BaseType *t = new daig::BaseType(**$1);
  t->dims = $2->type->dims;
  daig::Variable newVar = *$2;
  newVar.type = daig::Type(t);
  $$->push_back(newVar);
  delete $1; delete $2;
}
| param_list TCOMMA type var {
  $$ = $1;
  daig::BaseType *t = new daig::BaseType(**$3);
  t->dims = $4->type->dims;
  daig::Variable newVar = *$4;
  newVar.type = daig::Type(t);
  $$->push_back(newVar);
  delete $3; delete $4;
}
;

var_decl_list : { $$ = new daig::VarList(); }
| var_decl_list var_decl TSEMICOLON {
  $$ = $1; $$->insert($$->end(),$2->begin(),$2->end());
  delete $2;
}
;

stmt_list : stmt { $$ = new daig::StmtList(); $$->push_back(*$1); delete $1; }
| stmt_list stmt { $$ = $1; $$->push_back(*$2); delete $2; }
;

stmt : TATOMIC stmt { $$ = new daig::Stmt(new daig::AtomicStmt(*$2)); delete $2; }
| TPRIVATE stmt { $$ = new daig::Stmt(new daig::PrivateStmt(*$2)); delete $2; }
| TLBRACE stmt_list TRBRACE { $$ = new daig::Stmt(new daig::BlockStmt(*$2)); delete $2; }
| lval TEQUAL expr TSEMICOLON { 
  $$ = new daig::Stmt(new daig::AsgnStmt(daig::Expr($1),*$3));
  delete $3;
}
| TIF TLPAREN expr TRPAREN stmt {
  $$ = new daig::Stmt(new daig::ITStmt(*$3,*$5));
  delete $3; delete $5; printStmt(*$$);
}
| TIF TLPAREN expr TRPAREN stmt TELSE stmt {
  $$ = new daig::Stmt(new daig::ITEStmt(*$3,*$5,*$7));
  delete $3; delete $5; delete $7; printStmt(*$$);
}
| TWHILE TLPAREN expr TRPAREN stmt {
  $$ = new daig::Stmt(new daig::WhileStmt(*$3,*$5));
  delete $3; delete $5;
}
| TFOR TLPAREN for_init TSEMICOLON for_test TSEMICOLON for_update TRPAREN stmt {
  $$ = new daig::Stmt(new daig::ForStmt(*$3,*$5,*$7,*$9));
  delete $3; delete $5; delete $7; delete $9;
}
| TBREAK TSEMICOLON { $$ = new daig::Stmt(new daig::BreakStmt()); }
| TCONTINUE TSEMICOLON { $$ = new daig::Stmt(new daig::ContStmt()); }
| TRETURN expr TSEMICOLON { $$ = new daig::Stmt(new daig::RetStmt(*$2)); delete $2; }
| TRETURN TSEMICOLON { $$ = new daig::Stmt(new daig::RetVoidStmt()); }
| lval TLPAREN arg_list TRPAREN TSEMICOLON {
  $$ = new daig::Stmt(new daig::CallStmt(daig::Expr($1), *$3));
  delete $3;
}
| TFAN TLPAREN TIDENTIFIER TRPAREN stmt { 
  $$ = new daig::Stmt(new daig::FANStmt(*$3,*$5));
  delete $3; delete $5;
}
| TFADNP TLPAREN TIDENTIFIER TCOMMA TIDENTIFIER TRPAREN stmt {
  $$ = new daig::Stmt(new daig::FADNPStmt(*$3,*$5,*$7));
  delete $3; delete $5; delete $7;
}
| TFAO TLPAREN TIDENTIFIER TRPAREN stmt {
  $$ = new daig::Stmt(new daig::FAOStmt(*$3,*$5));
  delete $3; delete $5;
}
| TFAOL TLPAREN TIDENTIFIER TRPAREN stmt {
  $$ = new daig::Stmt(new daig::FAOLStmt(*$3,*$5));
  delete $3; delete $5;
}
| TFAOH TLPAREN TIDENTIFIER TRPAREN stmt {
  $$ = new daig::Stmt(new daig::FAOHStmt(*$3,*$5));
  delete $3; delete $5;
}
;

lval : TIDENTIFIER { 
  $$ = new daig::LvalExpr(*$1);
  delete $1; printExpr($$);
}
| TIDENTIFIER TDOT TIDENTIFIER {
  $$ = new daig::LvalExpr(*$1,*$3);
  delete $1; delete $3; printExpr($$);
}
| TIDENTIFIER indices {
  $$ = new daig::LvalExpr(*$1,*$2);
  delete $1; delete $2; printExpr($$);
}
| TIDENTIFIER TDOT TIDENTIFIER indices {
  $$ = new daig::LvalExpr(*$1,*$3,*$4);
  delete $1; delete $3; delete $4; printExpr($$);
}
;

for_init : {}
| lval TEQUAL expr {
  $$ = new daig::StmtList();
  $$->push_back(daig::Stmt(new daig::AsgnStmt(daig::Expr($1),*$3)));
  delete $3;
}
| for_init TCOMMA lval TEQUAL expr {
  $$ = $1;
  $$->push_back(daig::Stmt(new daig::AsgnStmt(daig::Expr($3),*$5)));
  delete $5;
}
;

for_test : { $$ = new daig::ExprList(); }
| expr { $$ = new daig::ExprList(); $$->push_back(*$1); delete $1; }
;

for_update : {  $$ = new daig::StmtList(); }
| lval TEQUAL expr {
  $$ = new daig::StmtList();
  $$->push_back(daig::Stmt(new daig::AsgnStmt(daig::Expr($1),*$3)));
  delete $3;
}
;

expr : lval { $$ = new daig::Expr($1); printExpr(*$$); }
| TINTEGER { 
  $$ = new daig::Expr(new daig::IntExpr(atoi($1->c_str()))); 
  delete $1; printExpr(*$$); 
}
| TDOUBLE {
  if (*$1 == "NAN") $$ = new daig::Expr(new daig::DoubleExpr(*$1)); else $$ = new daig::Expr(new daig::DoubleExpr(atof($1->c_str())));
  delete $1; printExpr(*$$);
}
| TNODENUM { MAKE_NULL($$,$1); }
| TMINUS expr { MAKE_UN($$,$1,$2); }
| TPLUS expr { MAKE_UN($$,$1,$2); }
| expr TCEQ expr { MAKE_BIN($$,$2,$1,$3); }
| expr TCNE expr { MAKE_BIN($$,$2,$1,$3); }
| expr TCLT expr { MAKE_BIN($$,$2,$1,$3); }
| expr TCLE expr { MAKE_BIN($$,$2,$1,$3); }
| expr TCGT expr { MAKE_BIN($$,$2,$1,$3); }
| expr TCGE expr { MAKE_BIN($$,$2,$1,$3); }
| expr TPLUS expr { MAKE_BIN($$,$2,$1,$3); }
| expr TMINUS expr { MAKE_BIN($$,$2,$1,$3); }
| expr TMUL expr { MAKE_BIN($$,$2,$1,$3); }
| expr TDIV expr { MAKE_BIN($$,$2,$1,$3); }
| expr TMOD expr { MAKE_BIN($$,$2,$1,$3); }
| expr TLAND expr { MAKE_BIN($$,$2,$1,$3); }
| expr TLOR expr { MAKE_BIN($$,$2,$1,$3); }
| expr TBWAND expr { MAKE_BIN($$,$2,$1,$3); }
| expr TBWOR expr { MAKE_BIN($$,$2,$1,$3); }
| expr TBWXOR expr { MAKE_BIN($$,$2,$1,$3); }
| expr TBWLSH expr { MAKE_BIN($$,$2,$1,$3); }
| expr TBWRSH expr { MAKE_BIN($$,$2,$1,$3); }
| expr TQUEST expr TCOLON expr { MAKE_TRI($$,$2,$1,$3,$5); }
| TLNOT expr { MAKE_UN($$,$1,$2); }
| TBWNOT expr { MAKE_UN($$,$1,$2); }
| lval TLPAREN arg_list TRPAREN { 
  $$ = new daig::Expr(new daig::CallExpr(daig::Expr($1),*$3));
  delete $3; printExpr(*$$);
} 
| TEXO TLPAREN TIDENTIFIER TCOMMA expr TRPAREN {
  $$ = new daig::Expr(new daig::EXOExpr(*$3,daig::Expr(*$5)));
  delete $3; delete $5;
}
| TEXH TLPAREN TIDENTIFIER TCOMMA expr TRPAREN {
  $$ = new daig::Expr(new daig::EXHExpr(*$3,daig::Expr(*$5)));
  delete $3; delete $5;
}
| TEXL TLPAREN TIDENTIFIER TCOMMA expr TRPAREN {
  $$ = new daig::Expr(new daig::EXLExpr(*$3,daig::Expr(*$5)));
  delete $3; delete $5;
}
| TLPAREN expr TRPAREN { $$ = $2; }
;

indices : TLBRACKET expr TRBRACKET {
  $$ = new daig::ExprList();
  $$->push_back(*$2);
}
| indices TLBRACKET expr TRBRACKET {
  $$ = $1;
  $$->push_back(*$3);
}
;

arg_list : { $$ = new daig::ExprList(); }
| expr { $$ = new daig::ExprList(); $$->push_back(*$1); }
| arg_list TCOMMA expr {
  $$ = $1;
  $$->push_back(*$3);
}
;

prog_def : TPROGRAM TEQUAL node_instances TSEMICOLON {}
;

node_instances : TIDENTIFIER TLPAREN TINTEGER TRPAREN {
  builder->program.processes.push_back(daig::Process(*$1,atoi($3->c_str())));
  delete $1; delete $3;
}
| node_instances TLOR TIDENTIFIER TLPAREN TINTEGER TRPAREN {
  builder->program.processes.push_back(daig::Process(*$3,atoi($5->c_str())));
  delete $3; delete $5;
}
;

init_def : TVOID TINIT TLPAREN TRPAREN TLBRACE var_decl_list stmt_list TRBRACE {
  /** set scope of temporary variables */
  BOOST_FOREACH(daig::Variable &v,*$6) v.scope = daig::Variable::TEMP;
  /** create and add function to the node -- make its return type void */
  builder->program.addFunction(daig::Function(daig::voidType(),"INIT",daig::VarList(),*$6,*$7));
  delete $6; delete $7;
}
;

safety_def : TVOID TSAFETY TLPAREN TRPAREN TLBRACE var_decl_list stmt_list TRBRACE {
  /** set scope of temporary variables */
  BOOST_FOREACH(daig::Variable &v,*$6) v.scope = daig::Variable::TEMP;
  /** create and add function to the node -- make its return type void */
  builder->program.addFunction(daig::Function(daig::voidType(),"SAFETY",daig::VarList(),*$6,*$7));
  delete $6; delete $7;
}
;
%%
