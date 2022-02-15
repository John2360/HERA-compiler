#include "AST.h"
#include <hc_list.h>
#include <hc_list_helpers.h>

/*
 * HERA_code methods
 */

const string indent_math = "    ";  // might want to use something different for, e.g., branches


string AST_node_::HERA_code()  // Default used during development; could be removed in final version
{
	string message = "HERA_code() requested for AST node type not yet having a HERA_code() method";
	EM_error(message);
	return "#error " + message;  //if somehow we try to HERA-C-Run this, it will fail
}


string A_root_::HERA_code()
{
	return  main_expr->HERA_data() +"CBON()\n\n" + main_expr->HERA_code();  // was SETCB for HERA 2.3
}



string A_intExp_::HERA_code()
{
	return indent_math + "SET(" + result_reg_s() + ", " + str(value) +")\n";
}

string A_stringExp_::HERA_code()
{
    string my_code;
    my_code += "SET("+this->result_reg_s()+", "+this->result_dlabel()+")\n";

    return my_code;
}



static string HERA_math_op(Position p, A_oper op) // needed for opExp
{
	switch (op) {
	case A_plusOp:
		return "ADD";
    case A_minusOp:
        return "SUB";
	case A_timesOp:
		return "MUL";	// was MULT for HERA 2.3
    case A_divideOp:
        return "DIV";
	default:
		EM_error("Unhandled case in HERA_math_op", false, p);
		return "Oops_unhandled_hera_math_op";
	}
}
string A_opExp_::HERA_code()
{

    string left_side = _left->HERA_code();
    string right_side = _right->HERA_code();

    string pre_build;
    string my_code;

    if (_left->result_reg() >= _right->result_reg()){
        pre_build = left_side + indent_math + "MOVE(R"+str( _left->result_reg()+1)+", "+_left->result_reg_s()+")\n" + right_side;

        my_code = indent_math + (HERA_math_op(pos(), _oper) + "(" +
                                 this->result_reg_s() + ", " +
                "R"+str( _left->result_reg()+1) + ", " +
                                 _right->result_reg_s() + ")\n\n");
    } else {
        pre_build = right_side + indent_math + "MOVE(R"+str( _right->result_reg()+1)+", "+ _right->result_reg_s()+")\n" + left_side;

        my_code = indent_math + (HERA_math_op(pos(), _oper) + "(" +
                                 this->result_reg_s() + ", " +
                                 _left->result_reg_s() + ", " +
                "R"+str( _right->result_reg()+1)+")\n\n");
    }

	return pre_build + my_code;
}


string A_callExp_::HERA_code()
{

    string prefix_code;
    string my_code;

    A_expList my_pointer = _args;
    while (true) {

        string my_register;
        if (my_pointer->_head->is_dlabel()){
            my_register = my_pointer->_head->result_dlabel();
        } else {
            my_register = my_pointer->_head->result_reg_s();
        }

        prefix_code += _args->_head->HERA_code();
        my_code += "MOVE(R1, "+my_register+") \n";

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    my_code += "CALL(FP_alt, "+Symbol_to_string(_func)+") \n\n";

    return prefix_code + my_code;
}

string A_seqExp_::HERA_code()
{
    return _seq->HERA_code();
}

string A_expList_::HERA_code()
{
    string my_code;
    my_code += _head->HERA_code();

    if (_tail != 0){
        my_code += _tail->HERA_code();
    }
    return my_code;
}