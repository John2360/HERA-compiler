#include "util.h"
#include "AST.h"
#include "errormsg.h"
#include "typecheck.h"

#include <map>
#include <list>

//#include <hc_list.h>	// gets files from /home/courses/include folder, thanks to -I flag on compiler
//#include <hc_list_helpers.h>


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
    return Ty_Error();
}

Ty_ty AST_node_::init_my_type(){
    EM_warning("Error, my_type not implemented here", false);
    return Ty_Error();
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

Ty_ty A_boolExp_::typecheck()
{
    return Ty_Bool();
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

Ty_ty A_arithExp_::typecheck()
{
    if (_left->typecheck() != Ty_Int() || _right->typecheck() != Ty_Int()) {
        EM_error("Oops silly goose, math operation expects two INTs as parameter types", true);
        return Ty_Error();
    } else {
        return Ty_Int();
    }
}

Ty_ty A_condExp_::typecheck()
{
    // Fix inputs
    if (_right->typecheck() != _left->typecheck()) {
        EM_error("Oops silly goose, left and right of conditional operator must be of the same type", true);
        return Ty_Error();
    } else {
        return Ty_Bool();
    }
}

Ty_ty A_ifExp_::typecheck()
{
    Ty_ty my_type = Ty_Void();
    if (_test->typecheck() != Ty_Bool()) {
        EM_error("Oops silly goose, if expression requires boolean", true);
        return Ty_Error();
    } else {
        // return children type and make sure have same type
        if (_else_or_null != 0) {
            if (_then->typecheck() != _else_or_null->typecheck()) {
                EM_error("Oops silly goose, if requires then and else to be of the same type", true);
                return Ty_Error();
            }

            // only set type if has then because could be else null
            my_type = _then->typecheck();
        }

        return my_type;
    }
}

Ty_ty A_whileExp_::typecheck() {
    if (_cond->typecheck() != Ty_Bool()){
        EM_error("Oops silly goose, while cond requires boolean", true);
        return Ty_Error();
    }

    return Ty_Void();
}

Ty_ty A_callExp_::typecheck()
{
    try {
        function_type_info my_func = this->find_local_functions(_func);

        // check num of args
        int total_func_args = 0;
        A_expList my_pointer = _args;

        while (true) {
            total_func_args += 1;

            if (my_pointer->_tail == 0) break;
            my_pointer = my_pointer->_tail;

        }
        int found_func_length_of_param = length(my_func.param_types);

        if (length(my_func.param_types) != total_func_args) {
            EM_error("Oops silly goose, the number of parameters do not match", true);
            return Ty_Error();
        }

        // check args match
        HaverfordCS::list<Ty_ty> my_pointer_func_list = my_func.param_types;
        A_expList my_pointer_func = _args;

        if (head(my_pointer_func_list) != Ty_Void()) {
            while (true){
                Ty_ty test = my_pointer_func->_head->typecheck();
                if (head(my_pointer_func_list) != my_pointer_func->_head->typecheck()) {
                    EM_error("Oops silly goose, the function inputs do not match the expected types", true);
                    return Ty_Error();
                }

                if (my_pointer_func->_tail == 0 || empty(my_pointer_func_list)) break;
                my_pointer_func_list = rest(my_pointer_func_list);
                my_pointer_func = my_pointer_func->_tail;
            }
        } else if (head(my_pointer_func_list) == Ty_Void() && _args->length() >= 1 && !_args->_head->null_input()) {
            EM_error("Oops silly goose, the function should have no inputs", true);
            return Ty_Error();
        }

        return my_func.return_type;

    }
    catch(const tiger_standard_library::undefined_symbol &missing) {
        EM_debug("Oops silly goose, the function " + str(missing.name) + " is not defined");
        return Ty_Error();
    }
}

Ty_ty A_forExp_::typecheck() {

    if (_hi->typecheck() != Ty_Int() || _lo->typecheck() != Ty_Int()){
        EM_error("Oops silly goose, for requires bounds to be type int", true);
        return Ty_Error();
    }

    return Ty_Void();

}

Ty_ty A_varExp_::typecheck() {
    return _var->typecheck();
}

Ty_ty A_simpleVar_::typecheck() {
    Ty_ty my_type = this->find_local_variables(_sym).type;

    if (my_type == Ty_Nil()) return this->implicit_type_init(_sym);
    return my_type;
}

Ty_ty A_decList_::typecheck() {

    _head->typecheck();

    if (_tail != 0) _tail->typecheck();
    return Ty_Void();
}

Ty_ty A_letExp_::typecheck(){
    _decs->typecheck();
    return _body->typecheck();
}

//Ty_ty A_decList_::typecheck() {
//    Ty_ty head_type = _head->typecheck();
//
//    if (_tail != 0) return _tail->typecheck();
//    return head_type;
//}

Ty_ty A_varDec_::typecheck() {
    Ty_ty my_type = _init->typecheck();
    if (from_String(str(_typ)) != my_type && from_String(str(_typ)) != Ty_Nil()) {EM_error("Oops silly goose, the declared type does not match variable type", true); return Ty_Error();}
    if (from_String(str(_typ)) != my_type) _typ = to_Symbol(from_Type(my_type));
    return my_type;
}

Ty_ty A_assignExp_::typecheck() {
    return Ty_Void();
}

Ty_ty A_fundec_::typecheck() {
    return Ty_Nil();
}
// The bodies of other type checking functions,
//  including any virtual functions you introduce into
//  the AST classes, should go here.

