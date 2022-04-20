#include "AST.h"

/*
 * methods for working with "result_reg" attribute
 */


static int next_unique_number = 1;
static int next_unique_string_number = 0;
static int next_unique_while_number = 0;
static int next_unique_if_arith_number = 0;
static int next_unique_if_cond_number = 0;
static int next_unique_for_number = 0;
static int next_unique_skip_func_number = 0;

//int AST_node_::fp_plus_for_me(A_exp which_child) {
//    return which_child->regular_fp_plus();
//}
int A_exp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    EM_error("Using old reg method. Please update.", false);
	return 1;
}
int A_exp_::init_regular_fp_plus() {
    if (this->stored_result_fp_plus > -1){
        return this->stored_result_fp_plus;
    } else {
        return this->parent()->regular_fp_plus();
    }
}
int A_exp_::init_result_fp_plus() {
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus();
    } else {
        return for_me;
    }
}

int A_arithExp_::init_result_fp_plus(){
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus();
    } else {
        return for_me;
    }
}

int A_callExp_::init_result_fp_plus() {
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus();
    } else {
        return for_me;
    }
}
int A_forExp_::init_result_fp_plus() {
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus()+1;
    } else {
        return for_me+1;
    }
}
int A_varExp_::init_result_fp_plus() {
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus();
    } else {
        return for_me;
    }
}
int A_letExp_::init_result_fp_plus() {
    int for_me = this->parent()->fp_plus_for_me(this);

    if (for_me == -1){
        return this->parent()->regular_fp_plus()+1;
    } else {
        return for_me+1;
    }
}

int A_letExp_::init_result_end_fp_plus() {
    return _decs->result_end_fp_plus();
}
int A_decList_::init_result_end_fp_plus() {
    if (_tail == 0) return _head->result_fp_plus();
    return _tail->result_end_fp_plus();
}


int A_varDec_::init_result_fp_plus() {
    return this->parent()->result_fp_plus();
}

int A_fundec_::init_result_fp_plus() {
    return 3;
}

int A_decList_::init_result_fp_plus(){
    if (parent()->my_let_fp_plus() != -1) return parent()->my_let_fp_plus();
    return this->parent()->result_fp_plus()+1;
}

int A_fieldList_::init_result_fp_plus(){
    if (parent()->my_func_fp_plus() != -1) return parent()->my_func_fp_plus();
    return this->parent()->result_fp_plus()+1;
}

int A_expList_::init_result_fp_plus() {
    return this->parent()->result_fp_plus();
}
int A_simpleVar_::init_result_fp_plus() {
    return this->parent()->result_fp_plus();
}
int A_field_::init_result_fp_plus() {
    return this->parent()->result_fp_plus();
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
    return 1;
}

int A_stringExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 1;
}

int A_boolExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 1;
}

int A_varDec_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 1;
}

int A_nilExp_::init_result_reg()  // generate unique numbers, starting from 1, each time this is called
{
    // for those who've taken CS355/356, this should be an atomic transaction, in a concurrent environment
    // TODO: Put this back to one once we have different function parameters on the stack
    return 1;
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

//posible extra regs
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

// possible issue
int A_decList_::init_result_reg()
{
    return 1;
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

string A_fundec_::init_label_skip(){
    int my_number = next_unique_skip_func_number;
    next_unique_skip_func_number = next_unique_skip_func_number + 1;
    std::string my_string = "my_skip_func_"+str(next_unique_skip_func_number);
    // end of atomic transaction
    return my_string;
}

int A_condExp_::init_labels()
{
    int my_number = next_unique_if_cond_number;
    next_unique_if_cond_number = my_number + 1;
    return next_unique_if_cond_number;
}

int A_forExp_::init_result_reg() {
    if (_hi->result_reg() == _lo->result_reg() || _hi->result_reg() == _body->result_reg() || _lo->result_reg() == _body->result_reg()){
        return std::max(_hi->result_reg(), std::max(_lo->result_reg(), _body->result_reg()))+1;
    }
    return std::max(_hi->result_reg(), std::max(_lo->result_reg(), _body->result_reg()));
}

int A_letExp_::init_result_reg(){
    return _body->result_reg();
}

int A_varExp_::init_result_reg() {
    return _var->result_reg();

}

int A_ifExp_::init_if_labels()
{
    int my_number = next_unique_if_arith_number;
    next_unique_if_arith_number = my_number + 1;
    return next_unique_if_arith_number;
}

int A_whileExp_::init_labels()
{
    int my_number = next_unique_while_number;
    next_unique_while_number = my_number + 1;
    return next_unique_while_number;
}

int A_forExp_::init_labels()
{
    int my_number = next_unique_for_number;
    next_unique_for_number = my_number + 1;
    return next_unique_for_number;
}

int A_ifExp_::init_result_reg()
{
    int else_reg_num = -1;
    if (_else_or_null != 0) {else_reg_num = _else_or_null->result_reg();};
    if (_then->result_reg() == else_reg_num) return _then->result_reg()+1;
    return std::max(_then->result_reg(), else_reg_num);
}

int A_assignExp_::init_result_reg() {
    return _exp->result_reg();
}

int A_whileExp_::init_result_reg()
{
    return _body->result_reg();
}

int A_breakExp_::init_result_reg(){
    return 1;
}

string A_breakExp_::init_break_label(){
    return this->parent()->break_label();
}

int A_simpleVar_::init_result_reg() {
    return 1;
}

int A_simpleVar_::get_offest() {
    if(stored_offest == -1){
        stored_offest = this->find_local_variables_fp(_sym, this->result_fp_plus());
    }
    return stored_offest;
}

local_variable_scope A_forExp_::init_local_variable(){
    this->create_variable(_var, Ty_Int(), this->result_fp_plus());

    return vars_data_shell;
}

local_variable_scope AST_node_::init_local_variable(){
    return vars_data_shell;
}

tiger_standard_library AST_node_::init_local_funcs(){

    return funcs_data_shell;
}

tiger_standard_library A_fundec_::init_local_funcs(){
    this->create_function(_name, from_String(str(_result)), this->type_field_list());

    return funcs_data_shell;
}

local_variable_scope A_letExp_::init_local_variable(){
    return _decs->my_local_variables();
}

local_variable_scope A_fundec_::init_local_variable(){
    return _params->my_local_variables();
}

local_variable_scope A_decList_::init_local_variable(){
    if (_tail == 0) return _head->my_local_variables();
    return merge(_head->my_local_variables(), _tail->my_local_variables());
}

local_variable_scope A_fieldList_::init_local_variable(){
    if (_tail == 0) return _head->my_local_variables();
    return merge(_head->my_local_variables(), _tail->my_local_variables());
}

local_variable_scope A_varDec_::init_local_variable(){
    this->create_variable(_var, from_String(str(_typ)), this->result_fp_plus());

    return vars_data_shell;
}

local_variable_scope A_field_::init_local_variable(){
    this->create_variable(_name, from_String(str(_typ)), this->result_fp_plus());

    return vars_data_shell;
}

HaverfordCS::list<Ty_ty> A_fieldList_::type_field_list(){
    if (_tail != 0) return _head->type_field_list();
    return _head->type_field_list().operator=(_tail->type_field_list());
}

HaverfordCS::list<Ty_ty> A_fundec_::type_field_list(){
    return _params->type_field_list();
}