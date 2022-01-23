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
	switch (type){
	case BOOL_EXP:
		return new BoolExp();
	case STRING_EXP:
		return new StrExp();
	case VOID_EXP:
		return new VoidExp();
	case INT_EXP:
	case BYTE_EXP:
		#ifndef OLDT
		throw NotImplementedError();
		#endif
		return new NumericExp(type, "NOT IMPLEMENTED");
	}
}

Expression::Expression(ExpType type)
	:type(type){};


RegStoredExp::RegStoredExp(ExpType type, const string& rvalue_exp)
	:Expression(type), reg(cb.getFreshReg()){
	cb.emitRegDecl(reg, rvalue_exp);
}

NumericExp::NumericExp(ExpType type, const string& rvalue_exp)
	:RegStoredExp(type, rvalue_exp){}

// Expression* cloneCast(ExpType type){
	
// }

BoolExp::BoolExp()
	:Expression(BOOL_EXP){};

BoolExp::BoolExp(std::vector<Backpatch> truelist, std::vector<Backpatch> falselist)
	:Expression(BOOL_EXP), truelist(truelist), falselist(falselist){}

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