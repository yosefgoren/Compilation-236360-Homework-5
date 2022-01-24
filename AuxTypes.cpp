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

Expression* Expression::generateExpByType(ExpType type){
	#ifndef OLDT
	throw NotImplementedError();
	#else
	switch(type){
		case BOOL_EXP:
			return new BoolExp();
		case INT_EXP:
		case BYTE_EXP:
			return new NumericExp(type, "NO_REGISTER");
		case VOID_EXP:
			return new VoidExp();
		case STRING_EXP:
			return new StrExp();
		}
	#endif
}

Expression::Expression(ExpType type)
	:type(type){};


RegStoredExp::RegStoredExp(ExpType type, const string& rvalue_exp)
	:Expression(type), reg(cb.getFreshReg()){
	cb.emit(reg+" = "+rvalue_exp);
}

NumericExp::NumericExp(ExpType type, const string& rvalue_exp)
	:RegStoredExp(type, rvalue_exp){}

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

// Expression* cloneCast(ExpType type){
	
// }

BoolExp::BoolExp()
	:Expression(BOOL_EXP){};

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

// Expression* BoolExp::cloneCast(ExpType type){
// 	if(type != BOOL_EXP){
// 		throw InvalidCastException();
// 	}
// 	return new BoolExp();
// }

StrExp::StrExp()
	:Expression(STRING_EXP){};

// Expression* StrExp::cloneCast(ExpType type){
// 	if(type != STRING_EXP){
// 		throw InvalidCastException();
// 	}
// 	return new StrExp();
// }

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

// RunBlock* RunBlock::newBlockHere(){
// 	return new RunBlock(cb.genLabel("statment_start"));
// }

// void RunBlock::endBlock(){
// 	nextlist = std::vector<Backpatch>(cb.makelist(Backpatch(cb.emit("br label @"), FIRST)));
// }