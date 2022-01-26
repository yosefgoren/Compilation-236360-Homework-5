#include "AuxTypes.hpp"
#include "bp.hpp"
#include "assert.h"

static CodeBuffer& cb = CodeBuffer::instance();

std::string ExpTypeString(ExpType type, bool capital_letters){
	switch (type)
	{
	case INT_EXP:
		return capital_letters ? "INT" : "int";
	case BOOL_EXP:
		return capital_letters ? "BOOL" : "bool";
	case BYTE_EXP:
		return capital_letters ? "BYTE" : "byte";
	case STRING_EXP:
		return capital_letters ? "STRING" : "string";
	case VOID_EXP:
		return capital_letters ? "VOID" : "void";
	}
}

std::vector<std::string> ExpTypeStringVector(std::vector<ExpType> types, bool capital_letters){
	std::vector<std::string> result = {};
	for(ExpType type : types){
		result.push_back(ExpTypeString(type, capital_letters));
	}
	return result;
}

Expression::Expression(ExpType type)
	:type(type){};

RegStoredExp::RegStoredExp(ExpType type, const string& rvalue_exp, bool store_to_new_reg)
	:Expression(type){
	if(store_to_new_reg){
		reg = cb.getFreshReg();
		cb.emit(reg+" = "+rvalue_exp);
	} else {
		reg = rvalue_exp;
	}
}

NumericExp::NumericExp(ExpType type, const string& rvalue_exp, bool store_to_new_reg)
	:RegStoredExp(type, rvalue_exp, store_to_new_reg){}

void NumericExp::convertToInt(){
	if(type == BYTE_EXP){
		string new_reg = cb.getFreshReg("b2int_conv_reg");
		cb.emit(new_reg+" = zext i8 "+reg+" to i32");
		reg = new_reg;
	}
	type = INT_EXP;
}

void NumericExp::convertToByte(){
	if(type == INT_EXP){
		string new_reg = cb.getFreshReg("int2byte_conv_reg");
		cb.emit(new_reg+" = trunc i32 "+reg+" to i8");
		reg = new_reg;
	}
	type = BYTE_EXP;
}

string NumericExp::storeAsRawReg(){
	string res;
	if(type == INT_EXP){
		res = reg;
	} else {
		res = cb.getFreshReg("raw_reg");
		cb.emit(res+" = zext i8 "+reg+" to i32");
	}
	return res;
}

BoolExp::BoolExp(const std::string rvalue_reg, bool rvalue_reg_is_raw_data)
	:Expression(BOOL_EXP){

	string bool_value_reg;
	if(rvalue_reg_is_raw_data){
		bool_value_reg = cb.getFreshReg();
		cb.emit(bool_value_reg+" = trunc i32 "+rvalue_reg+" to i1");
	} else {
		bool_value_reg = rvalue_reg;
	}
	//now 'bool_value_reg' should hold the required value, it should also be of type 'i1'.

	int initial_branch_adderess = cb.emit("br i1 "+bool_value_reg+", label @, label @");

	string true_jump_label = cb.genLabel("true_case");
	cb.bpatch(cb.makelist(Backpatch(initial_branch_adderess, FIRST)), true_jump_label);
	int truelist_jump_address = cb.emit("br label @");

	string false_jump_label = cb.genLabel("false_case");
	cb.bpatch(cb.makelist(Backpatch(initial_branch_adderess, SECOND)), false_jump_label);
	int falselist_jump_address = cb.emit("br label @");
	
	//end_label = cb.genLabel("bool_ending");
	truelist = cb.makelist(Backpatch(truelist_jump_address, FIRST));
	falselist = cb.makelist(Backpatch(falselist_jump_address, FIRST));
}

BoolExp::BoolExp(std::vector<Backpatch> truelist, std::vector<Backpatch> falselist)
	:Expression(BOOL_EXP), truelist(truelist), falselist(falselist){}


string BoolExp::storeAsRegPrototype(bool as_raw_reg){
	string true_label = cb.genLabel("true_case");
	cb.bpatch(truelist, true_label);
	int true_jump_addr = cb.emit("br label @");
	
	string false_label = cb.genLabel("false_case");
	cb.bpatch(falselist, false_label);
	int false_jump_addr = cb.emit("br label @");
	
	string bool_reg_label = cb.genLabel("set_bool_reg");
	cb.bpatch(cb.makelist(Backpatch(true_jump_addr, FIRST)), bool_reg_label);
	cb.bpatch(cb.makelist(Backpatch(false_jump_addr, FIRST)), bool_reg_label);
	string res_reg = cb.getFreshReg();
	string resulting_ir_type = as_raw_reg ? "i32" : "i1";
	cb.emit(res_reg+" = phi "+resulting_ir_type+" [1, %"+true_label+"], [0, %"+false_label+"]");
	return res_reg;
}

string BoolExp::storeAsReg(){
	return storeAsRegPrototype(false);
}
string BoolExp::storeAsRawReg(){
	return storeAsRegPrototype(true);
}

int StrExp::str_count = 0;
std::string StrExp::getFreshStringId(){
	return "@.string_id"+to_string(StrExp::str_count++);
}

StrExp::StrExp(const string& value)
	:Expression(STRING_EXP), llvm_global_id(getFreshStringId()){
	string clipped_value = value.substr(1, value.size()-2);//this removes the quotation marks from the string.
	
	ir_type = "[" + to_string(clipped_value.size()+1) + " x i8]";

	cb.emitGlobal(llvm_global_id + " = constant "+ir_type+" c\""+ clipped_value + "\\00\"");
}

std::string StrExp::loadPtrToReg(){
	string ptr_reg = cb.getFreshReg("str_ptr_reg");
	cb.emit(ptr_reg+" = getelementptr "+ir_type+", "+ir_type+"* "+llvm_global_id+", i32 0, i32 0");
	return ptr_reg;
}

VoidExp::VoidExp()
	:Expression(VOID_EXP){};

BranchBlock::BranchBlock(std::string cond_label, Expression* cond_exp)
	:cond_label(cond_label){
	BoolExp* bool_exp = dynamic_cast<BoolExp*>(cond_exp);
	assert(bool_exp);
	truelist = bool_exp->truelist;
	falselist = bool_exp->falselist;
}	


RunBlock::RunBlock(const std::string& start_label)
	:start_label(start_label){}

RunBlock::RunBlock(const std::string& start_label, const RunBlock& first_merge_part, const RunBlock& second_merge_part)
	:start_label(start_label)
	, nextlist(cb.merge(first_merge_part.nextlist, second_merge_part.nextlist))
	, continuelist(cb.merge(first_merge_part.continuelist, second_merge_part.continuelist))
	, breaklist(cb.merge(first_merge_part.breaklist, second_merge_part.breaklist)){}

RunBlock* RunBlock::newSinkBlockEndingHere(const std::string& block_start_label){
	RunBlock* res = new RunBlock(block_start_label); 
	res->nextlist = std::vector<Backpatch>();
	return res;
}

RunBlock* RunBlock::newBlockEndingHere(const std::string& block_start_label){
	RunBlock* res = new RunBlock(block_start_label); 
	res->nextlist = cb.makelist(Backpatch(cb.emit("br label @"), FIRST));
	return res;
}

RunBlock* RunBlock::newContinueBlockHere(const std::string& block_start_label){
	RunBlock* res = new RunBlock(block_start_label); 
	res->continuelist = cb.makelist(Backpatch(cb.emit("br label @"), FIRST));
	return res;
}

RunBlock* RunBlock::newBreakBlockHere(const std::string& block_start_label){
	RunBlock* res = new RunBlock(block_start_label); 
	res->breaklist = cb.makelist(Backpatch(cb.emit("br label @"), FIRST));
	return res;
}