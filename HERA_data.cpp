//
// Created by John Finberg on 2/3/22.
//
#include "AST.h"

const string indent = "    ";

string AST_node_::HERA_data()  // Default used during development; could be removed in final version
{
    string message = "HERA_data() requested for AST node type not yet having a HERA_data() method";
    EM_error(message);
    return "#error " + message;  //if somehow we try to HERA-C-Run this, it will fail
}

string A_arithExp_::HERA_data()
{
    return _left->HERA_data() + _right->HERA_data();
}

string A_condExp_::HERA_data()
{
    return _left->HERA_data() + _right->HERA_data();
}

string A_intExp_::HERA_data()
{
    return "";
}

string A_boolExp_::HERA_data()
{
    return "";
}

string A_callExp_::HERA_data()
{
    string my_code;

//    A_expList my_pointer = _args;
//    while (true){
//        my_code += my_pointer->_head->HERA_data();
//        if (my_pointer->_tail == 0) break;
//        my_pointer = my_pointer->_tail;
//    }
    my_code += _args->HERA_data();

//    A_expList my_pointer = _args->;
//    while (true && my_pointer != 0){
//        my_code += my_pointer->HERA_data();
//        if (my_pointer->_tail == 0) break;
//        my_pointer = my_pointer->_tail;
//    }
    return my_code;
}

string A_seqExp_::HERA_data()
{
    return _seq->HERA_data();
}

string A_expList_::HERA_data() {
    //May have screwed this up in the process
    string my_code;
    my_code += _head->HERA_data();

    A_expList my_pointer = _tail;
    while (true && my_pointer != 0){
        my_code += my_pointer->HERA_data();
        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;
    }
    return my_code;
}

string A_stringExp_::HERA_data()
{
    string my_code;
    my_code += "DLABEL("+this->result_dlabel()+") \n";
    my_code += indent + "LP_STRING("+ repr(value)+") \n\n";
    return my_code;
}

string A_assignExp_::HERA_data() {
    return _exp->HERA_data() + _var->HERA_data();
}

string A_letExp_::HERA_data() {
    return _decs->HERA_data() + _body->HERA_data();
}

string A_decList_::HERA_data() {
    string my_code;
    my_code += _head->HERA_data();

    if (_tail != 0){
        my_code += _tail->HERA_data();
    }

    return my_code;
}

string A_fundecList_::HERA_data() {
    string my_code;
    my_code += _head->HERA_data();

    if (_tail != 0){
        my_code += _tail->HERA_data();
    }

    return my_code;
}

string A_fieldList_::HERA_data() {
    string my_code;
    my_code += _head->HERA_data();

    if (_tail != 0){
        my_code += _tail->HERA_data();
    }

    return my_code;
}

string A_field_::HERA_data() {
    return "";
}

string A_varDec_::HERA_data() {
    return _init->HERA_data();
}

string A_ifExp_::HERA_data() {
    return (_else_or_null != 0) ? _test->HERA_data()+_then->HERA_data()+_else_or_null->HERA_data() : _test->HERA_data()+_then->HERA_data();
}

string A_whileExp_::HERA_data() {
    return _cond->HERA_data() + _body->HERA_data();
}

string A_forExp_::HERA_data() {
    return _hi->HERA_data() + _lo->HERA_data() + _body->HERA_data();
}

string A_simpleVar_::HERA_data(){
    return "";
}

string A_breakExp_::HERA_data() {
    return "";
}

string A_varExp_::HERA_data() {
    return _var->HERA_data();
}

string A_fundec_::HERA_data() {
    string my_code;

    my_code += "BR("+this->branch_label_post()+")\n";
    my_code += "LABEL("+str(_name)+")\n";
    my_code += "STORE(PC_ret, 0,FP)\nSTORE(FP_alt,1,FP)\n";
    my_code += _body->HERA_code();
    my_code += "LOAD(PC_ret, 0,FP)\nLOAD(FP_alt,1,FP)\n";
    my_code += "STORE("+_body->result_reg_s()+", 3, FP)\nRETURN(FP_alt, PC_ret)\n";
    my_code += "LABEL("+this->branch_label_post()+")\n\n";

    return my_code;
}

string A_functionDec_::HERA_data() {
    return theFunctions->HERA_data();
}