#include "bp.hpp"
#include "AuxTypes.hpp"
#include "Symtab.hpp"
#include "assert.h"
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;
extern SimpleSymtab symtab;

bool replace(string& str, const string& from, const string& to, const BranchLabelIndex index);

CodeBuffer::CodeBuffer() : buffer(), globalDefs() {}

CodeBuffer &CodeBuffer::instance() {
	static CodeBuffer inst;//only instance
	return inst;
}

string CodeBuffer::genLabel(){
	std::stringstream label;
	label << "label_";
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

string CodeBuffer::emitRegDecl(const string& lvalue_id, const string& rvalue_exp){
	emit(lvalue_id + " = " + rvalue_exp);
	return lvalue_id;
}

void CodeBuffer::emitStoreVar(const string& id, Expression* exp_to_assign){
	ExpType type = exp_to_assign->type;
	assert(type != STRING_EXP && type != VOID_EXP);


	string res_reg;
	if(type == BOOL_EXP){
		res_reg = "...?";
		throw NotImplementedError();
		//TODO: complete this s.t. the truelist will jump to 
		//	something that puts 1 in there, and falselist 0 (both extented to 32 bits)
	} else {
		NumericExp* numeric_exp = dynamic_cast<NumericExp*>(exp_to_assign);
		res_reg = type == INT_EXP
			? numeric_exp->reg
			: emitRegDecl(getFreshReg(), "zext i8 "+numeric_exp->reg + " to i32");	
	}
	emitStoreVarBasic(id, res_reg);
}
void CodeBuffer::emitStoreVar(const string& id, int immidiate){
	emitStoreVarBasic(id, to_string(immidiate));
}

string CodeBuffer::emitLoadVar(const string& id){

}
void CodeBuffer::emitStoreVarBasic(const string& id, const string& immidiate_or_reg){
	int offset = symtab.getVariableOffset(id);
	string ptr = createPtrToStackVar(offset);
	emit("store i32 "+immidiate_or_reg+", i32* "+ptr);
}

string CodeBuffer::createPtrToStackVar(int offset){
	std::string ptr_reg = getFreshReg();
	emitRegDecl(ptr_reg, "getelementptr i32, [50 x i32]* %sp, i32 0, i32 "+std::to_string(offset));
	return ptr_reg;
}

string CodeBuffer::getFreshReg(){
	return "%"+to_string(reg_count++);
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
		ir_binop = (type == BYTE_EXP ? "udiv" : "idiv");
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