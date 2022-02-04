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

string A_callExp_::HERA_data()
{
    return _args->_head->HERA_data();
}

string A_stringExp_::HERA_data()
{

    string my_code;
    my_code += "DLABEL(the_string) \n";
    my_code += indent + "LP_STRING("+ repr(value)+") \n\n";
    return my_code;
}