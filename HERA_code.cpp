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
	return  "#include \"Tiger-stdlib-stack-data.hera\"\n\n" + main_expr->HERA_data() +"CBON()\n\n" + main_expr->HERA_code();  // was SETCB for HERA 2.3
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
    case A_eqOp:
        return "BZ";
    case A_geOp:
        return "BGE";
    case A_gtOp:
        return "BG";
    case A_leOp:
        return "BLE";
    case A_ltOp:
        return "BL";
    case A_neqOp:
        return "BNZ";
	default:
		EM_error("Unhandled case in HERA_math_op", false, p);
		return "Oops_unhandled_hera_math_op";
	}
}
string A_condExp_::HERA_code()
{

    string left_side = _left->HERA_code();
    string right_side = _right->HERA_code();

    string pre_build;
    string my_code;
    Ty_ty left_type = _left->typecheck();

    if(left_type == Ty_Int()) {

        if (_left->result_reg() >= _right->result_reg()) {
            pre_build =
                    left_side + indent_math + "MOVE(" + this->result_reg_s() + ", " + _left->result_reg_s() +
                    ")\n" + right_side;

            my_code = indent_math + ("CMP(" +
                    this->result_reg_s() + ", " +
                                     _right->result_reg_s() + ")\n");
        } else {
            pre_build = right_side + indent_math + left_side;

            my_code = indent_math + ("CMP(" +
                                     _left->result_reg_s() + ", "
                                      + this->result_reg_s() + ")\n\n");
        }
        my_code += HERA_math_op(pos(), _oper) + "(" + this->branch_label_true() + ")\n";

    } else if (left_type == Ty_String()){
        A_callExp_ my_call = A_callExp_(Position::undefined(), to_Symbol("tstrcmp"), A_ExpList(_left, A_ExpList(_right, 0)));

        my_code += my_call.HERA_code();
        my_code += "CMP("+my_call.result_reg_s()+", R0)\n";
        //returns neg # if a < b, 0 if =, pos # if a > b
        my_code += HERA_math_op(pos(), _oper) + "(" + this->branch_label_true() + ")\n";


    }

    my_code += "SET(" + this->result_reg_s() + ", 0)\n";
    my_code += "BR(" + this->branch_label_end() + ")\n";
    my_code += "LABEL(" + this->branch_label_true() + ")\n";
    my_code += "SET(" + this->result_reg_s() + ", 1)\n";
    my_code += "LABEL(" + this->branch_label_end() + ")\n";

    return pre_build + my_code;
}

string A_arithExp_::HERA_code()
{

    string left_side = _left->HERA_code();
    string right_side = _right->HERA_code();

    string pre_build;
    string my_code;

    if (_left->result_reg() >= _right->result_reg()){
        pre_build = left_side + indent_math + "MOVE("+this->result_reg_s()+", "+_left->result_reg_s()+")\n" + right_side;

        my_code = indent_math + (HERA_math_op(pos(), _oper) + "(" +
                                 this->result_reg_s() + ", " +
                                 this->result_reg_s()+", " +
                                 _right->result_reg_s() + ")\n\n");
    } else {
        pre_build = right_side + indent_math + left_side;

        my_code = indent_math + (HERA_math_op(pos(), _oper) + "(" +
                                 this->result_reg_s() + ", " +
                                 _left->result_reg_s() + ", " +
                                 this->result_reg_s()+")\n\n");
    }

    return pre_build + my_code;
}

string A_callExp_::HERA_code()
{

    string my_code;
    my_code += "MOVE(FP_alt, SP)\nINC(SP, "+str(_args->length()+3)+")\n";
    int stack_pointer = 3;

    A_expList my_pointer = _args;
    while (true) {

        my_code += my_pointer->_head->HERA_code();
        my_code += "STORE("+my_pointer->_head->result_reg_s()+", "+str(stack_pointer)+", FP_alt) \n";
        stack_pointer += 1;

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    my_code += "CALL(FP_alt, "+Symbol_to_string(_func)+")\nLOAD("+this->result_reg_s()+", 3, FP_alt)\nDEC(SP, "+str(_args->length()+2)+") \n\n";

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
    string then_clause = "\nLABEL(" + this->branch_label_then() + ")\n" +  _then->HERA_code() + "\nMOVE(" + this->result_reg_s() + ", " + _then->result_reg_s() + ")\n" + "BR(" + this->branch_label_post() + ")\n";

    string else_clause = "\nLABEL(" + this->branch_label_else() + ")\n";
    if (_else_or_null != 0){
        else_clause += _else_or_null->HERA_code() + "\nMOVE(" + this->result_reg_s() + ", " + _else_or_null->result_reg_s() + ")\n";
    }
    else_clause += "\nBR(" + this->branch_label_post() + ")\n";

    return test_cond + then_clause + else_clause + "LABEL(" + this->branch_label_post() + ")";
}

string A_whileExp_::HERA_code() {
    string my_code;
    my_code += "LABEL("+this->branch_label_cond()+")\n";
    my_code += _cond->HERA_code() + "\n";
    my_code += "CMP("+_cond->result_reg_s()+", R0)\n";
    my_code += "BNZ("+this->branch_label_post()+")\n";

    my_code += _body->HERA_code()  + "\n";
    my_code += "BR("+this->branch_label_cond()+")\n";
    my_code += "LABEL("+this->branch_label_post()+")\n";

    return my_code;
}

string A_breakExp_::HERA_code() {
    return "BR("+ this->break_label() +")\n";
}