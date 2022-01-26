#include "bp.hpp"
#include "AuxTypes.hpp"
#include "Symtab.hpp"
#include "assert.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;
extern SimpleSymtab symtab;

bool replace(string& str, const string& from, const string& to, const BranchLabelIndex index);

CodeBuffer::CodeBuffer() : buffer(), globalDefs() {}

CodeBuffer &CodeBuffer::instance() {
	static CodeBuffer inst;//only instance
	return inst;
}

string CodeBuffer::genLabel(const string& label_name){
	std::stringstream label;
	label << label_name;
	label << "_";
	label << buffer.size();
	std::string ret(label.str());
	label << ":";
	emit(label.str());
	return ret;
}

int CodeBuffer::emit(const string &s){
    buffer.push_back(s);
	return buffer.size() - 1;
}

void CodeBuffer::bpatch(const vector<pair<int,BranchLabelIndex>>& address_list, const std::string &label){
    for(vector<pair<int,BranchLabelIndex>>::const_iterator i = address_list.begin(); i != address_list.end(); i++){
    	int address = (*i).first;
    	BranchLabelIndex labelIndex = (*i).second;
		replace(buffer[address], "@", "%" + label, labelIndex);
    }
}

void CodeBuffer::printCodeBuffer(){
	for (std::vector<string>::const_iterator it = buffer.begin(); it != buffer.end(); ++it) 
	{
		cout << *it << endl;
    }
}

vector<Backpatch> CodeBuffer::makelist(Backpatch item)
{
	vector<Backpatch> newList;
	newList.push_back(item);
	return newList;
}

vector<Backpatch> CodeBuffer::makeEmptyList(){
	return vector<Backpatch>();
}

vector<Backpatch> CodeBuffer::merge(const vector<Backpatch> &l1,const vector<Backpatch> &l2){
	vector<Backpatch> newList(l1.begin(),l1.end());
	newList.insert(newList.end(),l2.begin(),l2.end());
	return newList;
}

// ******** Methods to handle the global section ********** //
void CodeBuffer::emitGlobal(const std::string& dataLine) 
{
	globalDefs.push_back(dataLine);
}

void CodeBuffer::printGlobalBuffer()
{
	for (vector<string>::const_iterator it = globalDefs.begin(); it != globalDefs.end(); ++it)
	{
		cout << *it << endl;
	}
}

// ******** Helper Methods ********** //
bool replace(string& str, const string& from, const string& to, const BranchLabelIndex index) {
	size_t pos;
	if (index == SECOND) {
		pos = str.find_last_of(from);
	}
	else { //FIRST
		pos = str.find_first_of(from);
	}
    if (pos == string::npos)
        return false;
    str.replace(pos, from.length(), to);
    return true;
}

void CodeBuffer::emitLibFuncs(){
	emitGlobal("declare i32 @printf(i8*, ...)");
	emitGlobal("declare void @exit(i32)");
	emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
	emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
	emitGlobal("define void @printi(i32) {");
	emitGlobal("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
	emitGlobal("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
	emitGlobal("    ret void");
	emitGlobal("}");
	emitGlobal("define void @print(i8*) {");
	emitGlobal("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
	emitGlobal("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
	emitGlobal("    ret void");
	emitGlobal("}");	
}

string CodeBuffer::emitCopyReg(const string& src_reg_or_imm, ExpType src_reg_type, const string& new_reg_prefix){
	string new_reg = getFreshReg(new_reg_prefix);
	string ir_type = IrType(src_reg_type);
	emit(new_reg+" = add "+ir_type+" 0, "+src_reg_or_imm);
	return new_reg;
}

// string CodeBuffer::emitRegDecl(const string& lvalue_id, const string& rvalue_exp){
// 	emit(lvalue_id + " = " + rvalue_exp);
// 	return lvalue_id;
// }

string storeBoolOrNumericAsRawReg(Expression* exp){
	ExpType type = exp->type;
	assert(type == BOOL_EXP || type == INT_EXP || type == BYTE_EXP);
	string res_reg;
	if(type == BOOL_EXP){
		BoolExp* bool_exp = dynamic_cast<BoolExp*>(exp);
		assert(bool_exp);
		res_reg = bool_exp->storeAsRawReg();
	} else {
		NumericExp* numeric_exp = dynamic_cast<NumericExp*>(exp);
		assert(numeric_exp);
		res_reg = numeric_exp->storeAsRawReg();
	}
	return res_reg;
}

void CodeBuffer::emitStoreVar(const string& id, Expression* exp_to_assign){
	ExpType type = exp_to_assign->type;
	assert(type != STRING_EXP && type != VOID_EXP);

	string res_reg = storeBoolOrNumericAsRawReg(exp_to_assign);
	emitStoreVarBasic(id, res_reg);
}

void CodeBuffer::emitStoreVar(const string& id, const string& reg_or_immidiate){
	emitStoreVarBasic(id, reg_or_immidiate);
}

string paramRegisterAtOffset(int offset){
	assert(offset < 0);
	return "%"+to_string(-offset-1);
}

Expression* CodeBuffer::emitLoadVar(const string& id){
	assert(symtab.rvalValidId(id));
	int offset = symtab.getVariableOffset(id);
	ExpType type = symtab.getVariableType(id);
	assert(type != VOID_EXP && type != STRING_EXP);

	string raw_value_reg;
	if(offset >= 0){
		//if this identifier is a local variable:
		string param_ptr = getFreshReg("param_ptr");
		emit(param_ptr+" = getelementptr [50 x i32], [50 x i32]* %sp, i32 0, i32 "+to_string(offset));
		raw_value_reg = getFreshReg("param_raw");
		emit(raw_value_reg+" = load i32, i32* "+param_ptr);
	} else {
		//if this id is a parameter:
		raw_value_reg = paramRegisterAtOffset(offset);
		//this is the parameter number as defined in llvm,
		// for example the first parameter has offset -1, and is stored in register %1.
	}

	return createIdentifiableFromReg(raw_value_reg, type, true);
}

vector<string> prefixToEach(const vector<string>& words, const string& prefix){
	vector<string> res;
	for(auto word: words){
		res.push_back(prefix+word);
	}
	return res;
}

/**
 * the spacing will be added between every two words but not at the edges.
 **/
string concatWithSpacing(const vector<string>& words, const string& spacing){
	string res;
	for(const string& word: words)
		res += word + spacing;
	if(words.size() > 0){
		for(int i = 0; i < spacing.size(); ++i){
			res.pop_back();
		}
	}
	return res;
}

string CodeBuffer::IrFuncTypeFormat(const string& func_id){
	//this function does not emmit any ir, it only calculates the representation
	//	 of the function type in ir and returns it as string.
	FunctionType& func_type = symtab.getFunctionType(func_id);
	string return_type = IrType(func_type.return_type);
	vector<string> ir_types;
	for(ExpType type: func_type.getParameterTypes()){
		//all types are just the raw data (i32):
		ir_types.push_back(type == STRING_EXP ? "i8*" : "i32");
	}
	return return_type+"("+concatWithSpacing(ir_types, ", ")+")";
}

Expression* CodeBuffer::createIdentifiableFromReg(const string& reg_name, ExpType type, bool rvalue_reg_is_raw_data){
	assert(type != VOID_EXP && type != STRING_EXP);
	string truncated_value_reg;
	switch(type){
	case INT_EXP:
		return new NumericExp(INT_EXP, "add i32 0, "+reg_name);
	case BYTE_EXP:
		if(rvalue_reg_is_raw_data){
			truncated_value_reg = getFreshReg("truncated_byte");
			emit(truncated_value_reg+" = trunc i32 "+reg_name+" to i8");
		} else {
			truncated_value_reg = reg_name;
		}
		return new NumericExp(BYTE_EXP, "add i8 0, "+truncated_value_reg);
	case BOOL_EXP:
		return new BoolExp(reg_name, rvalue_reg_is_raw_data);
	}
	assert(false);
	return nullptr;
}

Expression* CodeBuffer::emitFunctionCall(const string& func_id, const vector<Expression*>& param_expressions){
	assert(symtab.callableValidId(func_id));
	vector<string> param_raw_value_regs;
	//TODO: check this impl handles the case were the result is VoidExp!

	//this code will convert each parameter to raw data (i32 with the same value) into some new register: 
	for(Expression* exp: param_expressions){
		assert(exp->type != VOID_EXP);
		string new_typed_reg;
		switch(exp->type){
		case STRING_EXP:
			new_typed_reg = "i8* " + dynamic_cast<StrExp*>(exp)->loadPtrToReg();
			break;
		case BOOL_EXP: {
			RegStoredExp* reg_bool_exp = dynamic_cast<RegStoredExp*>(exp);
			assert(reg_bool_exp);
			new_typed_reg = "i32 " +  reg_bool_exp->reg;
			break;
		}
		default:
			NumericExp* numeric_exp = dynamic_cast<NumericExp*>(exp);
			assert(numeric_exp);
			new_typed_reg = "i32 " + numeric_exp->storeAsRawReg();
		}
		param_raw_value_regs.push_back(new_typed_reg);
	}
	//this is since we have recived the parameters in reverse order:
	reverse(param_raw_value_regs.begin(), param_raw_value_regs.end());

	ExpType return_type = symtab.getReturnType(func_id);
	
	string ir_params_list = concatWithSpacing(param_raw_value_regs, ", ");
	string ir_func_type = IrFuncTypeFormat(func_id);
	string call_format = "call "+ir_func_type+" @"+func_id+"("+ir_params_list+")";
	
	if(return_type == VOID_EXP){
		emit(call_format);
		return new VoidExp();
	} else {
		//this code will emit a function call with all generated registers:
		string result_reg = getFreshReg();
		emit(result_reg+" = "+call_format);
		return createIdentifiableFromReg(result_reg, return_type, false);
	}
	assert(false);
	return nullptr;
}


void CodeBuffer::emitStoreVarBasic(const string& id, const string& immidiate_or_reg){
	int offset = symtab.getVariableOffset(id);
	ExpType var_type = symtab.getVariableType(id);
	assert(offset >= 0);
	//this means that the parameter has to be a local variable, hence stored on stack:
	string ptr = createPtrToStackVar(offset);
	emit("store i32 "+immidiate_or_reg+", i32* "+ptr);
}

void CodeBuffer::emitFuncDecl(const string& id){
	assert(symtab.callableValidId(id));
	FunctionType& func_type = symtab.getFunctionType(id);
	string ir_ret_type = IrType(func_type.return_type);
	
	vector<string> ir_types;
	for(ExpType type: func_type.getParameterTypes()){
		//all types are just the raw data (i32):
		ir_types.push_back("i32");
	}
	string param_types = concatWithSpacing(ir_types, ", ");

	emit("define "+ir_ret_type+"@"+id+"("+param_types+"){");
}

string CodeBuffer::IrDefaultTypedValue(ExpType type){
	assert(type != STRING_EXP);
	switch(type){
		case INT_EXP:
			return "i32 0";
		case BYTE_EXP:
			return "i8 0";
		case BOOL_EXP:
			return "i1 0";
		case VOID_EXP:
			return "void";
	}
	assert(false);
	return "IML ERROR";
}

string CodeBuffer::createPtrToStackVar(int offset){
	std::string ptr_reg = getFreshReg();
	emit(ptr_reg+" = getelementptr [50 x i32], [50 x i32]* %sp, i32 0, i32 "+std::to_string(offset));
	return ptr_reg;
}

string CodeBuffer::getFreshReg(const string& reg_name){
	return "%"+reg_name+to_string(reg_count++);
}

string CodeBuffer::IrType(ExpType type){
	switch(type){
	case INT_EXP:
		return "i32";
	case BOOL_EXP:
		return "i1";
	case BYTE_EXP:
		return "i8";
	case STRING_EXP:
		return "i8*";
	case VOID_EXP:
		return "void";
	}
}

string CodeBuffer::literalRvalFormat(int value, ExpType type){
	assert(type == INT_EXP || type == BYTE_EXP);
	return "add "+IrType(type)+" 0, "+to_string(value);
}

string CodeBuffer::binopRvalFormat(const string& first_reg, const string& second_reg, ExpType type, Binop binop){
	assert(type == INT_EXP || type == BYTE_EXP);

	string ir_binop;
	switch(binop){
	case PLUS:
		ir_binop = "add";
		break;
	case MINUS:
		ir_binop = "sub";
		break;
	case MULT:
		ir_binop = "mul";
		break;
	case DIV:
		ir_binop = (type == BYTE_EXP ? "udiv" : "sdiv");
	}
	return ir_binop+" "+IrType(type)+" "+first_reg+", "+second_reg;
}	

std::string CodeBuffer::IrRelopType(Relop relop, ExpType type){
	assert(type == INT_EXP || type == BYTE_EXP);
	std::string prefix = type == INT_EXP ? "s" : "u";
	switch(relop){
	case EQUAL:
		return "eq";
	case NOT_EQUAL:
		return "ne";
	case LESS:
		return prefix + "lt";
	case GREATER:
		return prefix + "gt";
	case LESS_EQUAL:
		return prefix + "le";
	case GREATER_EQUAL:
		return prefix + "ge";
	}
}

std::string CodeBuffer::relopRvalFormat(const string& first_reg, const string& second_reg, ExpType type, Relop relop){
	string op = IrRelopType(relop, type);
	return "icmp " + op + " " + IrType(type) + " " + first_reg + ", " + second_reg;
}