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

string A_boolExp_::HERA_code()
{
    return indent_math + "SET(" + result_reg_s() + ", " + (value == true ? '1' : '0') +")\n";
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

    EM_debug((HERA_math_op(pos(), _oper)) + " - " + _right->result_reg_s(), false);

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

    string my_code;
    int call_registers = 1;

    A_expList my_pointer = _args;
    while (true) {

        my_code += my_pointer->_head->HERA_code();
        my_code += "MOVE(R"+str(call_registers)+", "+my_pointer->_head->result_reg_s()+") \n";
        call_registers += 1;

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    my_code += "CALL(FP_alt, "+Symbol_to_string(_func)+")\n MOVE("+this->result_reg_s()+", R1) \n\n";

    return my_code;
}

string A_seqExp_::HERA_code()
{
    A_expList my_pointer = _seq;
    string last_reg = "";
    while (true) {
        last_reg = my_pointer->_head->result_reg_s();

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;
    }

    return _seq->HERA_code() + "MOVE("+this->result_reg_s()+", "+last_reg+")\n";
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

string A_ifExp_::HERA_code()
{
    string test_cond = _test->HERA_code() + "\nCMP(" + _test->result_reg_s() + ", R0)" + "\nBZ(" + this->branch_label_else() + ")\n";
    string then_clause = "\nLABEL(" + this->branch_label_then() + ")\n" +  _then->HERA_code() + "\nBR(" + this->branch_label_post() + ")\n";

    string else_clause = "";
    if (_else_or_null != 0){
        else_clause =  "\nLABEL(" + this->branch_label_else() + ")\n"  + _else_or_null->HERA_code() + "\nBR(" + this->branch_label_post() + ")\n";
    }

    return test_cond + then_clause + else_clause + "LABEL(" + this->branch_label_post() + ")";
}