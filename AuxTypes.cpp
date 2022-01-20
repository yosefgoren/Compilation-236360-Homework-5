#include "AuxTypes.hpp"

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

const std::string NumericExp::REG_NOT_ASSIGNED = "";

NumericExp::NumericExp(ExpType type)
	:Expression(type), reg(REG_NOT_ASSIGNED){}

bool NumericExp::isRegisterAssigned() const{
	return reg != REG_NOT_ASSIGNED;
}

BoolExp::BoolExp()
	:Expression(BOOL_EXP){};