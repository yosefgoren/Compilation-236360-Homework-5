#ifndef SYMTAB_H
#define SYMTAB_H

#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include "AuxTypes.hpp"


class SimpleSymtab{
public:
	void pushScope();
	void popScope(bool print_end_scope = true);

	void declareVar(const std::string& id, ExpType type, bool is_const);
	void declareFunc(const std::string& id, ExpType return_type, std::vector<Parameter>* params);
	//void declareLibFunc(const std::string& func_id, ExpType type, std::vector<Parameter>* params);
	void finishFunc(bool print_decls = true);
	bool declarableValidId(const std::string& id) const;
	bool containsVar(const std::string& id) const;
	bool rvalValidId(const std::string& id) const;
	bool callableValidId(const std::string& id) const;
	bool isConst(const std::string& id) const;
	ExpType getVariableType(const std::string& id) const;
	int getVariableOffset(const std::string& id) const;
	ExpType getReturnType(const std::string& id) const;
	/**
	 * @param id - the identifier assigned to to the function; the function we want the type of.
	 * @return FunctionType& - a reference to the function-type object stored in the symbol table. 
	 */
	FunctionType& getFunctionType(const std::string& id);
	FunctionType& getCurrentlyParsedFuncType();

	void printFuncDecls();
	//for debugging:
	void printFuncScope() const;
private:
	struct SymInfo {
		ExpType type;
		bool is_const;
		int offset;
	};
	std::unordered_map<std::string, SymInfo> variable_decls;
	//in each scope here, each id is a variable defined in the last scope:
	std::vector<std::vector<std::string>> scope_ids_stack;
	std::unordered_map<std::string, FunctionType> function_decls;
	std::vector<std::string> func_ids_stack;
	std::vector<std::string> func_scope;//each id here is a function parameter
	int curr_offset = 0;//at any stable point, this will point to the first offset that is avaliable.
	std::string currently_parsed_func = NO_CURRENTLY_PARSED_FUNC;
	static const std::string NO_CURRENTLY_PARSED_FUNC;
};

#endif