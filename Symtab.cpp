#include "Symtab.hpp"
#include "assert.h"
#include "hw3_output.hpp"
#include <iostream>
using namespace std;

const std::string SimpleSymtab::NO_CURRENTLY_PARSED_FUNC = "";

void SimpleSymtab::pushScope(){
	scope_ids_stack.push_back(vector<string>());
}

void SimpleSymtab::popScope(bool print_end_scope){
	if(print_end_scope)
		output::endScope();
	
	auto& top_scope = scope_ids_stack.back();
	int i = 0;
	const int initial_offset = curr_offset;
	const int scope_size = top_scope.size();
	for(auto& id : top_scope){
		//note the order of operations is important here:
		assert(containsVar(id));
		--curr_offset;
		//int ordered_offset = curr_offset - (top_scope.size() )- 1;
		int ordered_offset = 2*initial_offset - scope_size - curr_offset - 1; 
		output::printID(id, ordered_offset, ExpTypeString(getVariableType(id), true));
		variable_decls.erase(id);
		++i;
	}
	scope_ids_stack.pop_back();
	assert(curr_offset >= 0);
}

void SimpleSymtab::declareConstVar(const string& id, ExpType type, const string& reg_value){
	assert(declarableValidId(id));
	assert(type == INT_EXP || type == BYTE_EXP);
	variable_decls[id] = {.type = type, .is_const = true
		, .offset = curr_offset, .const_value = reg_value};
	scope_ids_stack.back().push_back(id);
	++curr_offset;
}

void SimpleSymtab::declareVar(const string& id, ExpType type){
	assert(declarableValidId(id));
	variable_decls[id] = {.type = type, .is_const = false, .offset = curr_offset};
	scope_ids_stack.back().push_back(id);
	++curr_offset;
}

void SimpleSymtab::declareFunc(const string& func_id, ExpType type, vector<Parameter>* params){
	assert(currently_parsed_func == NO_CURRENTLY_PARSED_FUNC);
	currently_parsed_func = func_id;
	assert(declarableValidId(func_id));
	assert(func_scope.size() == 0);
	function_decls[func_id] = FunctionType(type, params);

	int param_offset = 0;
	for(auto it = params->rbegin(); it != params->rend(); ++it){
		Parameter& p = *it;
		assert(declarableValidId(p.id));
		--param_offset;
		variable_decls[p.id] = {.type = p.type, .is_const = p.is_const, .offset = param_offset};
		func_scope.push_back(p.id);
	}
	func_ids_stack.push_back(func_id);
	pushScope();
}

void SimpleSymtab::finishFunc(bool print_decls){
	assert(currently_parsed_func != NO_CURRENTLY_PARSED_FUNC);
	currently_parsed_func = NO_CURRENTLY_PARSED_FUNC;

	if(print_decls)
		output::endScope();
	int param_offset = -1;
	for(const string& id : func_scope){
		assert(containsVar(id));
		if(print_decls)
			output::printID(id, param_offset, ExpTypeString(getVariableType(id), true));
		--param_offset;
		assert(containsVar(id));
		variable_decls.erase(id);
	}
	func_scope.clear();
	popScope(false);
}

bool SimpleSymtab::containsVar(const string& id) const{
	return variable_decls.count(id) == 1;
}

bool SimpleSymtab::declarableValidId(const string& id) const{
	return variable_decls.count(id) == 0 && function_decls.count(id) == 0;
}

bool SimpleSymtab::rvalValidId(const std::string& id) const{
	return variable_decls.count(id) == 1;
}
bool SimpleSymtab::callableValidId(const std::string& id) const{
	return function_decls.count(id) == 1;
}

bool SimpleSymtab::isConst(const std::string& id) const {
	assert(containsVar(id));
	return variable_decls.at(id).is_const;
}

std::string SimpleSymtab::getConstValue(const std::string& id) const{
	assert(isConst(id));
	return variable_decls.at(id).const_value;
}

ExpType SimpleSymtab::getVariableType(const string& id) const{
	assert(variable_decls.count(id) == 1);
	return variable_decls.at(id).type;
}
int SimpleSymtab::getVariableOffset(const std::string& id) const{
	return variable_decls.at(id).offset;
}

ExpType SimpleSymtab::getReturnType(const string& id) const{
	assert(callableValidId(id));
	return function_decls.at(id).return_type;
}

FunctionType& SimpleSymtab::getFunctionType(const std::string& id){
	assert(function_decls.count(id) == 1);
	return function_decls[id];
}
FunctionType& SimpleSymtab::getCurrentlyParsedFuncType(){
	assert(currently_parsed_func != NO_CURRENTLY_PARSED_FUNC);
	return getFunctionType(currently_parsed_func);
}
void SimpleSymtab::printFuncDecls(){
	for(string& func_id : func_ids_stack){
		FunctionType& func_type = getFunctionType(func_id);
		//get arg types (vector of strings containig the types of parameters):
		vector<string> str_arg_types = ExpTypeStringVector(func_type.getParameterTypes(), true);
		//magic number (0) is the constant offset for functions:
		string func_types_str = output::makeFunctionType(ExpTypeString(func_type.return_type, true), str_arg_types);
		output::printID(func_id, 0, func_types_str);
	}
}

//for debugging:
void SimpleSymtab::printFuncScope() const{
	for(auto item : func_scope){
		std::cout << item << std::endl;
	}
}