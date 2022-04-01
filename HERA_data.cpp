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

    A_expList my_pointer = _args;
    while (true){
        my_code += my_pointer->_head->HERA_data();
        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;
    }
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