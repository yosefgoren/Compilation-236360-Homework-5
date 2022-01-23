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
	cb.emitRegDecl(reg, rvalue_exp);
}

NumericExp::NumericExp(ExpType type, const string& rvalue_exp)
	:RegStoredExp(type, rvalue_exp){}

string NumericExp::storeAsRawReg(){
	return type == INT_EXP ? reg : cb.emitRegDecl(cb.getFreshReg(), "zext i8 "+reg+" to i32");
}

// Expression* cloneCast(ExpType type){
	
// }

BoolExp::BoolExp()
	:Expression(BOOL_EXP){};

BoolExp::BoolExp(const std::string rvalue_reg, bool rvalue_reg_is_raw_data)
	:Expression(BOOL_EXP){

	string bool_value_reg = cb.getFreshReg();
	if(rvalue_reg_is_raw_data){
		cb.emitRegDecl(bool_value_reg, "trunc i32 "+rvalue_reg+" to i1");
	} else {
		cb.emitRegDecl(bool_value_reg, rvalue_reg);
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

string BoolExp::storeAsRawReg(){
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
	cb.emitRegDecl(res_reg, "phi i32 [1, %"+true_label+"], [0, %"+false_label+"]");
	return res_reg;
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