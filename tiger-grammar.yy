// Note: to learn how to invoke bison/flex in Eclipse, see https://www.eclipse.org/forums/index.php/t/1075215/
// And refer to the example at https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html#A-Complete-C_002b_002b-Example

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines
%define api.parser.class {tigerParser}

/* next two from https://www.gnu.org/software/bison/manual/html_node/Complete-Symbols.html#Complete-Symbols */
%define api.value.type variant
%define api.token.constructor

%code requires
{
class tigerParseDriver;
}
%param { tigerParseDriver& driver }

// According to the Example, this turns on "location tracking"
%locations
// Thanks to Ian Fisher HC '19 for explaining the details of the parts I'd failed to get working


%{
#include "tigerParseDriver.h"
%}

%token END  0
%token <bool> BOOL
%token <int>  INT
%token <std::string> ID STRING
// NOTE that bison complains if you have the same symbol listed as %token (above) and %type (below)
//      so if you want to add attributes to a token, remove it from the list below

%token 
  COMMA COLON SEMICOLON LPAREN RPAREN L_SQUARE_BRACKET R_SQUARE_BRACKET 
  L_CURLY_BRACE R_CURLY_BRACE
  ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END_LET OF 
  BREAK NIL
  FUNCTION VAR TYPE DOT
  PLUS MINUS TIMES DIVIDE ASSIGN EQ NEQ LT LE GT GE OR AND NOT
;

/* precedence (stickiness) ... put the stickiest stuff at the bottom of the list */

%left IF WHILE DO BREAK FOR LET ASSIGN
%left THEN IN
%left ELSE END_LET
%left OR AND
%left EQ NEQ LT LE GT GE
%left MINUS PLUS
%left TIMES DIVIDE

%left NOT

/* https://www.gnu.org/software/bison/manual/html_node/Precedence-Only.html */
%nonassoc UMINUS

/* Attributes types for nonterminals are next, e.g. struct's from tigerParseDriver.h */
%type <expAttrs>  exp
%type <expAttrs>  seq
%type <argListAttrs>  args
%type <decListAttrs>  let_dec
%type <funcListAttrs> funcs_decs
%type <fieldListAttrs>  funcdec_args


// The line below means our grammar must not have conflicts
//  (no conflicts means it is "an LALR(1) grammar",
//   meaning it must be unambiguous and have some other properties).
%expect 1

// allowing one shift reduce in func_decs
// func_decs wants to shift and reduce at the end of func_decs this means that by reducing we are chunking up the func_dec lists
// by doing shift we are merging the chunks. Either are accepetable so we will allow the one execption.
// 8 funcs_decs: FUNCTION ID LPAREN funcdec_args RPAREN COLON MY_TYPE EQ exp •
// 9           | FUNCTION ID LPAREN funcdec_args RPAREN COLON MY_TYPE EQ exp • funcs_decs



%%

%start program;
program: exp[main]	{ EM_debug("Got the main expression of our tiger program.", $main.AST->pos());
		 			  driver.AST = new A_root_($main.AST);
		 			}
	;
seq: exp[i]					{ $$.AST = $i.AST;
      								  EM_debug("Got exp in seq", $$.AST->pos());
      								}
    | exp[exp1] SEMICOLON seq[seq1]    { $$.AST = A_SeqExp(Position::range($exp1.AST->pos(), $seq1.AST->pos()),
                                                                        A_ExpList($exp1.AST,
                                                                            A_ExpList($seq1.AST, 0)
                                                                       ));
                                        EM_debug("Got semicolon seq expression.", $$.AST->pos());
}
;

args: exp[i]					{ $$.DEC = A_DecList( A_VarDec($i.AST->pos(),
                                                             to_Symbol("!preprogram1"),
                                                             to_Symbol("unknown"),
                                                             $i.AST), 0);
                             $$.AST = A_ExpList(A_VarExp($i.AST->pos(), A_SimpleVar($i.AST->pos(), to_Symbol("!preprogram1"))), 0);
      								  EM_debug("Got exp in args", $$.AST->pos());
      					    $$.num = 1;

      								}
    | exp[exp1] COMMA args[seq1]    { $$.DEC = A_DecList( A_VarDec($exp1.AST->pos(),
                                                                       to_Symbol("!preprogram"+str($seq1.num+1)),
                                                                       to_Symbol("unknown"),
                                                                       $exp1.AST), $seq1.DEC);
                                       $$.AST = A_ExpList(A_VarExp($exp1.AST->pos(), A_SimpleVar($exp1.AST->pos(), to_Symbol("!preprogram"+str($seq1.num+1)))), $seq1.AST);
                                        EM_debug("Got comma arg expression.", $$.AST->pos());
                                        $$.num = $$.num+1;
}
;

funcdec_args: { $$.AST = $$.AST;}

    | ID[name] COLON ID[type]		{ $$.AST = A_FieldList(A_Field(Position::fromLex(@name), to_Symbol($name), to_Symbol($type)), 0);
      								  EM_debug("Got param in funcdec", $$.AST->pos());
      								}
    | ID[name] COLON ID[type] COMMA funcdec_args[seq1]    { $$.AST = A_FieldList(A_Field(Position::fromLex(@name), to_Symbol($name), to_Symbol($type)), $seq1.AST);
                                        EM_debug("Got comma param in funcdec", $$.AST->pos());

}
;

funcs_decs:  FUNCTION ID[name] LPAREN funcdec_args[params] RPAREN COLON ID[type] EQ exp[seq1] {
$$.AST = A_FundecList(A_Fundec(Position::fromLex(@name), to_Symbol($name), $params.AST, to_Symbol($type), $seq1.AST),
                                                                         0
                                                                         );
                                                       EM_debug("Got single func dec", $$.AST->pos());
                }
             |  FUNCTION ID[name] LPAREN funcdec_args[params] RPAREN COLON ID[type] EQ exp[seq1] funcs_decs[funcs] { $$.AST = A_FundecList(A_Fundec(Position::fromLex(@name), to_Symbol($name), $params.AST, to_Symbol($type), $seq1.AST),
                                                                                        $funcs.AST
                                                                                        );
                                                                                        EM_debug("Got multi func dec", $$.AST->pos());
                               }
;

let_dec: VAR ID[name] COLON ID[type] ASSIGN exp[seq1]			{ $$.AST = A_DecList(
                                  A_VarDec($seq1.AST->pos(),
                                            to_Symbol($name),
                                            to_Symbol($type),
                                            $seq1.AST), 0);
      								  EM_debug("Got single let dec", $$.AST->pos());
      								}
    | VAR ID[name] COLON ID[type] ASSIGN exp[seq1] let_dec[decs]    {
                $$.AST = A_DecList(
                      A_VarDec($seq1.AST->pos(),
                                to_Symbol($name),
                                to_Symbol($type),
                                $seq1.AST), $decs.AST);
                          EM_debug("Got multiple let dec", $$.AST->pos());
                          }
    | funcs_decs[funcs] let_dec[decs]  {
                    $$.AST = A_DecList(A_FunctionDec($funcs.AST->pos(), $funcs.AST), $decs.AST);
                    EM_debug("Got func dec followed by let dec", $$.AST->pos());
        }
   | funcs_decs[funcs]  {
                       $$.AST = A_DecList(A_FunctionDec(Position::undefined(), $funcs.AST), 0);
                       EM_debug("Got closing func dec", $$.AST->pos());
           }
;

exp:  INT[i]					{ $$.AST = A_IntExp(Position::fromLex(@i), $i);
								  EM_debug("Got int " + str($i), $$.AST->pos());
								}
    | BOOL[i]					{ $$.AST = A_BoolExp(Position::fromLex(@i), $i);
                                      std::string bool_str = ($i == true ? "true" : "false");
      								  EM_debug("Got bool " + bool_str , $$.AST->pos());
      								}
    | STRING[i]					{ $$.AST = A_StringExp(Position::fromLex(@i), $i);
      								  EM_debug("Got str " + $i, $$.AST->pos());
      								}
    | ID[i]					{ $$.AST = A_VarExp(Position::fromLex(@i), A_SimpleVar(Position::fromLex(@i), to_Symbol($i)));
          								  EM_debug("Got simple var " + $i, $$.AST->pos());
          								}
    | LPAREN RPAREN              {  $$.AST = A_NilExp(Position::undefined());
                                        EM_debug("Got nil", Position::undefined());
                                   }
     //bison manual 75
    | MINUS exp[exp1] %prec UMINUS  { $$.AST = A_ArithExp($exp1.AST->pos(),
                                               A_minusOp,  A_IntExp($exp1.AST->pos(), 0),$exp1.AST);
                              EM_debug("Got negative expression.", $$.AST->pos());
                                }
	| exp[exp1] PLUS exp[exp2]	{ $$.AST = A_ArithExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_plusOp,  $exp1.AST,$exp2.AST);
								  EM_debug("Got plus expression.", $$.AST->pos());
								}
    | exp[exp1] MINUS exp[exp2]	{ $$.AST = A_ArithExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
    												   A_minusOp,  $exp1.AST,$exp2.AST);
    						      EM_debug("Got minus expression.", $$.AST->pos());
    							}
	| exp[exp1] TIMES exp[exp2]	{ $$.AST = A_ArithExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
												   A_timesOp, $exp1.AST,$exp2.AST);
								  EM_debug("Got times expression.", $$.AST->pos());
								 }
	| exp[exp1] DIVIDE exp[exp2]	{  $$.AST = A_CallExp( Position::range($exp1.AST->pos(), $exp2.AST->pos()),
                                                                                                       to_Symbol("div"),
                                                                                                       A_ExpList($exp1.AST, A_ExpList($exp2.AST, 0)));
                                      EM_debug("Got divide expression.", $$.AST->pos());

    								 }
    | exp[exp1] EQ exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
    												   A_eqOp, $exp1.AST,$exp2.AST);
    								  EM_debug("Got equals expression.", $$.AST->pos());
    								 }
    | exp[exp1] NEQ exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
        												   A_neqOp, $exp1.AST,$exp2.AST);
        								  EM_debug("Got not equals expression.", $$.AST->pos());
        								 }
    | exp[exp1] LT exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
        												   A_ltOp, $exp1.AST,$exp2.AST);
        								  EM_debug("Got less than expression.", $$.AST->pos());
        								 }
    | exp[exp1] LE exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
        												   A_leOp, $exp1.AST,$exp2.AST);
        								  EM_debug("Got less than equals expression.", $$.AST->pos());
        								 }
    | exp[exp1] GT exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
        												   A_gtOp, $exp1.AST,$exp2.AST);
        								  EM_debug("Got greator than expression.", $$.AST->pos());
        								 }
    | exp[exp1] GE exp[exp2]	{ $$.AST = A_CondExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
        												   A_geOp, $exp1.AST,$exp2.AST);
        								  EM_debug("Got greator than equals expression.", $$.AST->pos());
        								 }
    | LPAREN seq[seq1] RPAREN { $$.AST = $seq1.AST;
                                EM_debug("Got seq expression.", $$.AST->pos());
                                }
    | ID[name] LPAREN args[arg1] RPAREN { $$.AST = A_LetExp($arg1.AST->pos() ,$arg1.DEC, A_CallExp( Position::fromLex(@name),
                                                                to_Symbol($name),
                                                                $arg1.AST
                                                                 ));

                                  EM_debug("Got function call to "+$name, $$.AST->pos());
                                }
    | ID[name] LPAREN RPAREN { $$.AST = A_CallExp( Position::fromLex(@name),
                                                 to_Symbol($name),
                                                 A_ExpList(A_NilExp(Position::undefined()), 0)
                                                  );

                                       EM_debug("Got void function call to "+$name, $$.AST->pos());
                                     }
    | exp[seq1] AND exp[seq2] { $$.AST = A_IfExp(Position::range($seq1.AST->pos(), $seq2.AST->pos()),
                                                              $seq1.AST,
                                                              $seq2.AST,
                                                              A_BoolExp(Position::undefined(), false));

                        EM_debug("Got and exp", $$.AST->pos());
                        }
    | exp[seq1] OR exp[seq2] { $$.AST = A_IfExp(Position::range($seq1.AST->pos(), $seq2.AST->pos()),
                                                                  $seq1.AST,
                                                                  A_BoolExp(Position::undefined(), true),
                                                                  $seq2.AST);

                            EM_debug("Got or exp", $$.AST->pos());
                            }
    | NOT exp[seq1] {  $$.AST = A_IfExp($seq1.AST->pos(),
                                          $seq1.AST,
                                          A_BoolExp(Position::undefined(), false),
                                          A_BoolExp(Position::undefined(), true));

                                EM_debug("Got not exp", $$.AST->pos());
                                }
    | WHILE exp[seq1] DO exp[seq2] {  $$.AST = A_WhileExp(Position::range($seq1.AST->pos(), $seq2.AST->pos()),
                                              $seq1.AST,
                                              $seq2.AST);

                                    EM_debug("Got while loop", $$.AST->pos());
                                    }
    | BREAK {  $$.AST = A_BreakExp(Position::undefined());
                    EM_debug("Got break exp", $$.AST->pos());
                                        }
    | IF exp[seq1] THEN exp[seq2] ELSE exp[seq3] { $$.AST = A_IfExp(Position::range($seq1.AST->pos(), $seq3.AST->pos()),
                                                                       $seq1.AST,
                                                                       $seq2.AST,
                                                                       $seq3.AST);
                                   }
    | IF exp[seq1] THEN exp[seq2] { $$.AST = A_IfExp(Position::range($seq1.AST->pos(), $seq2.AST->pos()),
                                                                           $seq1.AST,
                                                                           $seq2.AST,
                                                                           0);
                                       }
    | FOR ID[name] ASSIGN exp[seq1] TO exp[seq2] DO exp[seq3] {  $$.AST = A_ForExp(Position::range($seq1.AST->pos(), $seq3.AST->pos()),
                                                  to_Symbol($name),
                                                  $seq1.AST,
                                                  $seq2.AST,
                                                  $seq3.AST);
                                        //$$.AST->create_variable(to_Symbol($name), Ty_Int(), 0);
                                        EM_debug("Got for loop", $$.AST->pos());
                                        }
    | LET let_dec[decs] IN seq[seq1] END_LET { $$.AST = A_LetExp($seq1.AST->pos(),
                                                    $decs.AST,
                                                    $seq1.AST
                                                    );
                                                    EM_debug("Got let",$seq1.AST->pos());

                        }
    | ID[name] ASSIGN exp[seq1] { $$.AST = A_AssignExp($seq1.AST->pos(),
                                                        A_SimpleVar(Position::fromLex(@name), to_Symbol($name)),
                                                         $seq1.AST
                                                         );
    }

//
// Note: In older compiler tools, instead of writing $exp1 and $exp2, we'd write $1 and $3,
//        to refer to the first and third elements on the right-hand-side of the production.
//        We can still use this notation (and note that Appel uses it in his book), e.g. the above is like
//					$$.AST = A_OpExp(Position::range($exp1.AST->pos(), $exp2.AST->pos()),
//									 A_timesOp, $1.AST,$3.AST);
// Also: Bison's location system produces information about nonterminals as well as terminals,
//        so we could use @exp1 to get it's information about the locations of exp1
//        writing, e.g., Position::fromLex(@exp1) or instead of $exp1.AST->pos()
//
	;

%%

void
yy::tigerParser::error(const location_type& l,
          	       const std::string& m)
  {
	  EM_debug("In yy::tigerParser::error");
	  EM_error(m, true, Position::fromLex(l));
  }
