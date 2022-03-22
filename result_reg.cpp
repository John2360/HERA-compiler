#include "AST.h"

/*
 * methods for working with "result_reg" attribute
 */


static int next_unique_number = 1;
static int next_unique_string_number = 0;
static int next_unique_if_number = 0;
static int next_unique_cond_number = 0;

int A_exp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    EM_error("Using old reg method. Please update.", false);
	return 1;
}

string A_exp_::init_result_dlabel()  // generate unique numbers, starting from 1, each time this is called
{
    EM_error("Using old reg method. Please update.", false);
    return "my_string";
}

int A_intExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 4;
}

int A_stringExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 4;
}

int A_boolExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 4;
}

int A_nilExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 4;
}

int A_arithExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    if (_left->result_reg() == _right->result_reg()) return _left->result_reg()+1;
    return std::max(_left->result_reg(), _right->result_reg());
}

int A_condExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    if (_left->result_reg() == _right->result_reg()) return _left->result_reg()+1;
    return std::max(_left->result_reg(), _right->result_reg());
}

int A_expList_::init_result_reg()
{
    A_expList my_pointer = _tail;
    int max_reg = 0;
    while (true) {

        string my_register;
        if (max_reg < my_pointer->_head->result_reg()){
            max_reg = my_pointer->_head->result_reg();
        }

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    return max_reg;
}

int A_seqExp_::init_result_reg()
{
    A_expList my_pointer = _seq;
    int max_reg = 0;
    while (true) {

        string my_register;
        if (max_reg < my_pointer->_head->result_reg()){
            max_reg = my_pointer->_head->result_reg();
        }

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    return max_reg;
}

int A_callExp_::init_result_reg()
{
    A_expList my_pointer = _args;
    int max_reg = 0;
    while (true) {

        string my_register;
        if (max_reg < my_pointer->_head->result_reg()){
            max_reg = my_pointer->_head->result_reg();
        }

        if (my_pointer->_tail == 0) break;
        my_pointer = my_pointer->_tail;

    }

    // TODO: fix error
//    return 1;

    return max_reg + 1;
}

string A_stringExp_::init_result_dlabel()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    int my_number = next_unique_string_number;
    next_unique_string_number = next_unique_string_number + 1;
    std::string my_string = "my_string_"+str(next_unique_string_number);
    // end of atomic transaction
    return my_string;
}

int A_condExp_::init_labels()
{
    int my_number = next_unique_cond_number;
    next_unique_if_number = my_number + 1;
    return next_unique_if_number;
}

int A_ifExp_::init_if_labels()
{
    int my_number = next_unique_if_number;
    next_unique_if_number = my_number + 1;
    return next_unique_if_number;
}

int A_ifExp_::init_result_reg()
{
    return std::max(_else_or_null->result_reg(), std::max(_test->result_reg(), _then->result_reg()));
}
