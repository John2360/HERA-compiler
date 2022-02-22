#include "util.h"
#include "AST.h"
#include "errormsg.h"
#include "typecheck.h"

#include <map>
#include <list>

#include <hc_list.h>	// gets files from /home/courses/include folder, thanks to -I flag on compiler
#include <hc_list_helpers.h>


//
// Change this comment to describe the attributes you'll be using
//  to check types in tiger programs.
//

#define SYM_INFO_FOR_STDLIB

void typecheck(AST_node_ *root)
{
	// This function should take care of doing type checking,
	//  assuming "root" is the root of a tiger AST.
	// It should call on some functions to define the attributes
	//  related to type checking (and, in so doing, produce any necessary error messages).
    typecheck(root);


}

Ty_ty AST_node_::typecheck()
{
    EM_warning("Using generic node method", false);
    return Ty_Nil();
}

Ty_ty A_root_::typecheck()
{
    return main_expr->typecheck();
}

Ty_ty A_intExp_::typecheck()
{
    return Ty_Int();
}

Ty_ty A_stringExp_::typecheck()
{
    return Ty_String();
}

Ty_ty A_expList_::typecheck()
{
    A_expList my_pointer = _tail;
    A_exp my_node = _head;

    while (true) {

        if (my_pointer->_tail == 0) break;
        my_node = my_pointer->_head;
        my_node->typecheck();
        my_pointer = my_pointer->_tail;

    }

    return my_node->typecheck();
}

Ty_ty A_seqExp_::typecheck()
{
    A_expList my_pointer = _seq;
    A_exp my_node;

    while (true) {
        my_node = my_pointer->_head;
        my_node->typecheck();

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    return my_node->typecheck();
}

Ty_ty A_opExp_::typecheck()
{
    if (_left->typecheck() != Ty_Int() || _right->typecheck() != Ty_Int()) {
        EM_error("Oops, math operation expects two INTs as parameter types", true);
        return Ty_Error();
    } else {
        return Ty_Int();
    }
}

Ty_ty A_callExp_::typecheck()
{
    std::map<std::string, HaverfordCS::list<Ty_ty>> function_lookup = {
            { "print", HaverfordCS::ez_list(Ty_String(), Ty_Void() ) },
            { "printint", HaverfordCS::ez_list(Ty_Int(), Ty_Void()) }
    };

    std::list<Ty_ty> my_args;

    A_expList my_pointer = _args;
    while (true) {
        my_args.push_back(my_pointer->_head->typecheck());

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;
    }

    if (my_args.size() != length(function_lookup[Symbol_to_string(_func)])-1 ) {
        EM_error("Oops, function call "+Symbol_to_string(_func)+ " received too many or too few arguments", true); return Ty_Error();
    }

    HaverfordCS::list correct_arg_pointer = function_lookup[Symbol_to_string(_func)];
    while (my_args.size() != 0){
        Ty_ty my_arg = my_args.front();

        if (my_arg != head(function_lookup[Symbol_to_string(_func)])) {
            EM_error("Oops, function call "+Symbol_to_string(_func)+ " has incorrect parameter types; expected " + to_String(head(function_lookup[Symbol_to_string(_func)])) + " and got " + to_String(my_arg) , true);
            return Ty_Error();
        }

        my_args.pop_front();
        correct_arg_pointer = rest(correct_arg_pointer);
    }

    return first(correct_arg_pointer);
}

// The bodies of other type checking functions,
//  including any virtual functions you introduce into
//  the AST classes, should go here.

