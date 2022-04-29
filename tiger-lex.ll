%{

/* In this first section of the lex file (between %{ and %}),
   we define C++ functions that will be used later in the actions of part 3 */
   
 // Dave's list of useful references for flex and bison for C++:
 //  http://www.jonathanbeard.io/tutorials/FlexBisonC++
 //  http://dinosaur.compilertools.net/flex/flex_19.html
 //  https://www.gnu.org/software/bison/manual/html_node/Split-Symbols.html

#include <stdlib.h>
#include "tigerParseDriver.h"
#include "tiger-grammar.tab.hpp"
#include <bitset>



// next line from https://www.gnu.org/software/bison/manual/html_node/Calc_002b_002b-Scanner.html#Calc_002b_002b-Scanner
static yy::location loc;

// allows to keep track of nested comments
int comment_nesting = 0;
std::string string_input = "";

// The function below is somewhat overly verbose;
//  it is designed to serve as an example of
//  (a) calling a C function to process something from lex (see the "INT" pattern below), and
//  (b) processing a collection of characters one at a time, relying on their ASCII values

static int textToInt(std::string the_text)  // a C-style array of char will be converted to a String
{
	// here's a C-language way of doing this
	char zero = '0';  // the character 0
	char nine = '9';
	int result = 0;

	// the length() method of class string returns an "unsigned int", so make i the same type
	for (unsigned int i=0; i<the_text.length(); i++) {
		// the_text[i] is the i'th numeral, e.g. a '4' or a '2'
		// We need to convert this to a number, such as 4 or 2,
		//  and rely on the fact that the ASCII character set
		//  has '0', then '1', then '2', etc. in numeric order.
		// So '0'-'0' gives us 0, and '1'-'0' gives us 1, etc.
		// Start by saving the current character in a variable:
		char numeral = the_text[i];
		
		// We should get here *only* if we have only numerals,
		//  since we're called only for the pattern {digit}+ below.
		//
		assert (numeral >= zero and numeral <= nine);
		
		// now convert the numeral
		int numeral_value = numeral-zero;
		
		// and update the total value
		result = 10*result+numeral_value;
	}
	return result;
	
	// alternate implementation: use hc_string's to_int function:
	// return to_int(the_text);
}

static bool textToBool(std::string the_text)
{
    if (the_text == "true") {
        return true;
    } else if (the_text == "false") {
        return false;
    }

    return false;
}

static char textToControl(std::string the_text)
{

    if (the_text[2] > 96) {
        return the_text[2]-96;
    } else if (the_text[2] > 64) {
        return the_text[2]-64;
    }  else if (the_text[2] > 32) {
          return the_text[2]-32;
    }  else {
    //error
    EM_error("Invalid control key", true);
              return the_text[2];
          }

}

static char intToControl(std::string the_text)
{
    the_text.erase(0,1);
    int my_num_string = std::stoi(the_text);
    return char(my_num_string);
}

static Ty_ty textToType(std::string the_text)
{
    if (the_text == "int"){
        return Ty_Int();
    } else if (the_text == "string") {
        return Ty_String();
    } else if (the_text == "bool") {
              return Ty_Bool();
    } else if (the_text == "void") {
                   return Ty_Void();
         } else {
        return Ty_Error();
    }
}

// This uses some stuff created by flex, so it's easiest to just put it here.
int tigerParseDriver::parse (const std::string &f)
{
	fileName = f;

	if (fileName == "" || fileName == "-") {
		yyin = stdin;
	} else if (!(yyin = fopen (fileName.c_str (), "r"))) {
		error ("cannot open " + fileName + ".");
		exit (EXIT_FAILURE);
	}

	yy::tigerParser parser (*this);
	int res = parser.parse ();  // sets this->AST_root

	fclose (yyin);
	return res;
}

%}

/* In this second section of the lex file (after the %}),
   we can define variables in terms of regular expressions.
   A few other things, like %option, can also be used here.
   C-style comments (like this one) are also legal. */

/* options from the example */
%option noyywrap nounput
/* Not using these: batch debug noinput */


integer	[0-9]+

/* real numbers don't occur in tiger, but if they did,
   and we always wanted a "." with at least one numeral preceding it,
   we could do this: */
real	[0-9]+\.[0-9]*(e-?[0-9]+)?

/* cpp style comment
    uncomment below to enable REGEX and go to lines below
    and uncomment other REGEX comment

    add code from outtake labeled comments below comment!!

    add below newline definition
    {comment}   { loc.lines(yyleng); loc.step(); }
*/

/* type */
/* type (int|string|bool|void)*/

/* bool */
bool (true|false)

/* indentifier */
identifier [a-zA-Z][a-zA-Z0-9_]*

/* string */
string \"(.|\s)+?\"

/* In the third section of the lex file (after the %%),
   we can define the patterns for each token
   in terms of regular expressions and the variables above,
   and give the action (as C++ code) for each token.
   Comments are legal only inside the actions. */

/* define IN_COMMENT
 loop which takes start of comment and then eats everything until it finds
 end of comment

 resource: https://www.iith.ac.in/~ramakrishna/Compilers-Aug14/doc/flex.pdf
 */
%x IN_COMMENT
%x S_STRING

%%

%{
/* Surrounding four lines, and other things involving "loc", are from
   https://www.gnu.org/software/bison/manual/html_node/Calc_002b_002b-Scanner.html#Calc_002b_002b-Scanner */
  // Code run each time yylex is called.
  loc.step();
%}

[ \t]	{ loc.step(); }
[\n\r]	{ loc.lines(yyleng); loc.step(); }

\/\*             { BEGIN(IN_COMMENT); }
<IN_COMMENT>{
    \/\*           { loc.lines(yyleng); loc.step(); ++comment_nesting; }
    \*+\/        { loc.lines(yyleng); loc.step();
                    if (comment_nesting) --comment_nesting;
                   else BEGIN(INITIAL); }
    \*+           { loc.lines(yyleng); loc.step(); }
    [^/*\n]+      { loc.lines(yyleng); loc.step(); }
    [/]           { loc.lines(yyleng); loc.step(); }
    \n            { loc.lines(yyleng); loc.step(); }
}

\:\=    { return yy::tigerParser::make_ASSIGN(loc); }

\&		{ return yy::tigerParser::make_AND(loc); }
\|		{ return yy::tigerParser::make_OR(loc); }
\!		{ return yy::tigerParser::make_NOT(loc); }

\:		{ return yy::tigerParser::make_COLON(loc); }
\;		{ return yy::tigerParser::make_SEMICOLON(loc); }
\+		{ return yy::tigerParser::make_PLUS(loc); }
\-      { return yy::tigerParser::make_MINUS(loc); }
\*		{ return yy::tigerParser::make_TIMES(loc); }
\/		{ return yy::tigerParser::make_DIVIDE(loc); }
\,		{ return yy::tigerParser::make_COMMA(loc); }

\<\=		{ return yy::tigerParser::make_LE(loc); }
\>\=		{ return yy::tigerParser::make_GE(loc); }
\=		{ return yy::tigerParser::make_EQ(loc); }
\<\>		{ return yy::tigerParser::make_NEQ(loc); }
\>		{ return yy::tigerParser::make_GT(loc); }
\<		{ return yy::tigerParser::make_LT(loc); }

\(		{ return yy::tigerParser::make_LPAREN(loc); }
\)		{ return yy::tigerParser::make_RPAREN(loc); }

if      { return yy::tigerParser::make_IF(loc); }
then    { return yy::tigerParser::make_THEN(loc); }
else    { return yy::tigerParser::make_ELSE(loc); }
while    { return yy::tigerParser::make_WHILE(loc); }
do    { return yy::tigerParser::make_DO(loc); }
break    { return yy::tigerParser::make_BREAK(loc); }
for    { return yy::tigerParser::make_FOR(loc); }
to    { return yy::tigerParser::make_TO(loc); }
in    { return yy::tigerParser::make_IN(loc); }
let { return yy::tigerParser::make_LET(loc); }
end { return yy::tigerParser::make_END_LET(loc); }
var { return yy::tigerParser::make_VAR(loc); }
function { return yy::tigerParser::make_FUNCTION(loc); }


{bool}	{
   return yy::tigerParser::make_BOOL(textToBool(yytext), loc);
   }

{identifier}   { return yy::tigerParser::make_ID(yytext, loc); }

\"             { string_input = ""; BEGIN(S_STRING); }
<S_STRING>{
  [^"\\]+       { string_input += yytext; loc.lines(yyleng); loc.step(); }
  \\n           { string_input += '\n'; loc.lines(yyleng); loc.step(); }
  \\t           { string_input += '\t'; loc.lines(yyleng); loc.step(); }
  \\\"           { string_input += '\"'; loc.lines(yyleng); loc.step(); }
  \\\^[a-zA-Z]    { string_input += textToControl(yytext); loc.lines(yyleng); loc.step(); }
  \\[0-9][0-9][0-9]    { string_input += intToControl(yytext); loc.lines(yyleng); loc.step(); }
  "\\"            { string_input += '\\'; loc.lines(yyleng); loc.step(); }
  \\[0-9][0-9] {EM_error("Oops, invalid use of escape string", true);}
  \\[0-9] {EM_error("Oops, invalid use of escape string", true);}

   /* Above handle escape strings; Then get out. */
  \"            { loc.lines(yyleng); loc.step(); BEGIN(INITIAL); return yy::tigerParser::make_STRING(string_input, loc); }

  /* Flex knows it cannot get here and that is good! */
  . {loc.lines(yyleng); loc.step();}
}

{integer}	{
   return yy::tigerParser::make_INT(textToInt(yytext), loc);
   /* textToInt is defined above */
   /* make_INT, make_END from example at https://www.gnu.org/software/bison/manual/html_node/Complete-Symbols.html#Complete-Symbols */	  
   }

\<[Ee][Oo][Ff]\>		{ return yy::tigerParser::make_END(loc); /* this RE matches the literal five characters <EOF>, regardless of upper/lower case   */ }
<<EOF>>					{ return yy::tigerParser::make_END(loc); /* <<EOF>> is a flex built-in for an actual end of a file, when there is no more input */ }

.	{ string it = "?"; it[0] = yytext[0]; EM_error("illegal token: " + it); }
%%
