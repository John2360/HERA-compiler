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
        //A_ExpList(_left, A_ExpList(_right, 0))
        Symbol lname = to_Symbol("!callleft"+str(this->my_unique_num()));
        Symbol rname = to_Symbol("!callright"+str(this->my_unique_num()));
        A_letExp_ my_call = A_letExp_(Position::undefined(), A_DecList(A_VarDec(Position::undefined(), lname,
                                                                                  to_Symbol("string"), _left),
                                                                       A_DecList(A_VarDec(Position::undefined(), rname, to_Symbol("string"), _right), 0)),
                                      A_CallExp(Position::undefined(), to_Symbol("tstrcmp"), A_ExpList(A_VarExp(Position::undefined(), A_SimpleVar(Position::undefined(), lname)),
                                                                                                        A_ExpList(A_VarExp(Position::undefined(), A_SimpleVar(Position::undefined(), rname)), 0))));

        my_call.set_parent_pointers_for_me_and_my_descendants(this);
//        A_callExp_ my_call = A_callExp_(Position::undefined(), to_Symbol("tstrcmp"), A_ExpList(_left, A_ExpList(_right, 0)));
        my_code += my_call.HERA_code();
//        my_code += "LOAD("+this->result_reg_s()+", "+str(this->result_fp_plus())+", FP)";
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
    function_type_info my_func = find_local_functions(_func);

    int starting_frame_size = 3;
    int stack_pointer = starting_frame_size;
    my_code += "MOVE(FP_alt, SP)\nINC(SP, "+str(_args->length()+starting_frame_size)+")\n";

    A_expList my_pointer = _args;
    while (true) {

        my_code += my_pointer->_head->HERA_code();
        my_code += "STORE("+my_pointer->_head->result_reg_s()+", "+str(stack_pointer)+", FP_alt) \n";
        stack_pointer += 1;

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    my_code += "CALL(FP_alt, "+Symbol_to_string(_func)+my_func.unique_id+")\nLOAD("+this->result_reg_s()+", 3, FP_alt)\nDEC(SP, "+str(_args->length()+starting_frame_size)+") \n\n";

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

string A_decList_::HERA_code()
{
    string my_code;
    my_code += _head->HERA_code();

    if (_tail != 0){
        my_code += _tail->HERA_code();
    }
    return my_code;
}

string A_fundecList_::HERA_code()
{
    string my_code;
    my_code += _head->HERA_code();

    if (_tail != 0){
        my_code += _tail->HERA_code();
    }
    return my_code;
}

string A_fieldList_::HERA_code()
{
    string my_code;
    my_code += _head->HERA_code();

    if (_tail != 0){
        my_code += _tail->HERA_code();
    }
    return my_code;
}

string A_field_::HERA_code() {
    return "";
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

    return test_cond + then_clause + else_clause + "LABEL(" + this->branch_label_post() + ")\n";
}

string A_whileExp_::HERA_code() {
    string my_code;
    my_code += "LABEL("+this->branch_label_cond()+")\n";
    my_code += _cond->HERA_code() + "\n";
    my_code += "CMP("+_cond->result_reg_s()+", R0)\n";
    my_code += "BZ("+this->branch_label_post()+")\n";

    my_code += _body->HERA_code()  + "\n";
    my_code += "BR("+this->branch_label_cond()+")\n";
    my_code += "LABEL("+this->branch_label_post()+")\n";

    return my_code;
}

string A_breakExp_::HERA_code() {
    return "BR("+ this->break_label() +")\n";
}

string A_forExp_::HERA_code() {
    string my_code;

    int starting_frame_size = 1;
    int stack_pointer = this->result_fp_plus();

    my_code += "// for loop\n";
    my_code += _lo->HERA_code();
    my_code += "INC(SP, "+str(starting_frame_size)+")\n";
    my_code += "STORE("+_lo->result_reg_s()+", "+str(stack_pointer)+", FP) \n\n";
    my_code +=  _hi->HERA_code() + "\n";
    my_code += "MOVE("+this->result_reg_s()+", "+_hi->result_reg_s()+")\n";
    my_code += "LABEL("+this->branch_label_cond()+")\n";
    my_code += "LOAD(R"+ str(this->result_reg()-1) +", "+str(stack_pointer)+", FP)\n";
    my_code += "CMP(R"+str(this->result_reg()-1)+", "+this->result_reg_s()+")\n";
    my_code += "BG("+this->branch_label_post()+")\n";

    my_code += _body->HERA_code() + "\n";

    my_code += "LOAD(R"+ str(this->result_reg()-1) +", "+str(stack_pointer)+", FP)\n";
    my_code += "INC(R"+ str(this->result_reg()-1) +", 1)\n";
    my_code += "STORE(R"+ str(this->result_reg()-1) +", "+str(stack_pointer)+", FP) \n\n";
    my_code += "BR("+this->branch_label_cond()+")\n";
    my_code += "LABEL("+this->branch_label_post()+")\n";
    my_code += "DEC(SP, "+str(starting_frame_size)+")\n";

    return my_code;
}

string A_varExp_::HERA_code() {
    return _var->HERA_code();
}

string A_simpleVar_::HERA_code() {
    return "//load "+str(_sym)+" from mem\nLOAD("+ this->result_reg_s()+", "+str(this->get_offest())+", FP)\n";
};

string A_assignExp_::HERA_code() {
    string my_code;

    my_code += _exp->HERA_code();
    my_code += _var->HERA_assign();

    return my_code;
}

string A_simpleVar_::HERA_assign() {
    return "STORE(R"+str(parent()->result_reg())+", "+str(this->get_offest())+", FP)\n";
}
string A_letExp_::HERA_code() {
    string my_code;
    int dec_amount = this->let_fp_plus_total();

    my_code += _decs->HERA_code();
    my_code += _body->HERA_code();
    if (dec_amount > 0) {
        my_code += "DEC(SP, " + str(dec_amount) + ")\n";
    }

    return my_code;
}

string A_varDec_::HERA_code(){

    string my_code;
    my_code += "//store "+str(_var)+"\n";
    my_code += "INC(SP, 1)\n";
    my_code += _init->HERA_code();
    my_code += "STORE("+_init->result_reg_s()+", "+str(this->result_fp_plus()) + ", FP)\n";

    return my_code;
}

string A_fundec_::HERA_code() {
    string my_code;

    my_code += "BR("+this->branch_label_post()+")\n";
    my_code += "LABEL("+str(_name)+this->set_unique_id()+")\n";
    my_code += "STORE(PC_ret, 0,FP)\nSTORE(FP_alt,1,FP)\n";
    my_code += _body->HERA_code();
    my_code += "LOAD(PC_ret, 0,FP)\nLOAD(FP_alt,1,FP)\n";
    my_code += "STORE("+_body->result_reg_s()+", 3, FP)\nRETURN(FP_alt, PC_ret)\n";
    my_code += "LABEL("+this->branch_label_post()+")\n\n";

    return my_code;
}

string A_functionDec_::HERA_code() {
    return theFunctions->HERA_code();
}