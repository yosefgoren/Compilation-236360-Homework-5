#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <vector>
#include <string>
#include "AuxTypes.hpp"

using namespace std;

class CodeBuffer{
	CodeBuffer();
	CodeBuffer(CodeBuffer const&);
    void operator=(CodeBuffer const&);
	std::vector<std::string> buffer;
	std::vector<std::string> globalDefs;
public:
	static CodeBuffer &instance();

	// ******** Methods to handle the code section ******** //

	//generates a jump location label for the next command, writes it to the buffer and returns it
	std::string genLabel(const string& label_name = "label");

	//writes command to the buffer, returns its location in the buffer
	int emit(const std::string &command);

	//gets a pair<int,BranchLabelIndex> item of the form {buffer_location, branch_label_index} and creates a list for it
	static vector<Backpatch> makelist(pair<int,BranchLabelIndex> item);
	static vector<Backpatch> makeEmptyList();

	//merges two lists of {buffer_location, branch_label_index} items
	static vector<Backpatch> merge(const vector<Backpatch> &l1,const vector<Backpatch> &l2);

	/* accepts a list of {buffer_location, branch_label_index} items and a label.
	For each {buffer_location, branch_label_index} item in address_list, backpatches the branch command 
	at buffer_location, at index branch_label_index (FIRST or SECOND), with the label.
	note - the function expects to find a '@' char in place of the missing label.
	note - for unconditional branches (which contain only a single label) use FIRST as the branch_label_index.
	example #1:
	int loc1 = emit("br label @");  - unconditional branch missing a label. ~ Note the '@' ~
	bpatch(makelist({loc1,FIRST}),"my_label"); - location loc1 in the buffer will now contain the command "br label %my_label"
	note that index FIRST referes to the one and only label in the line.
	example #2:
	int loc2 = emit("br i1 %cond, label @, label @"); - conditional branch missing two labels.
	bpatch(makelist({loc2,SECOND}),"my_false_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @, label %my_false_label"
	bpatch(makelist({loc2,FIRST}),"my_true_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @my_true_label, label %my_false_label"
	*/
	void bpatch(const vector<Backpatch>& address_list, const std::string &label);
	
	//prints the content of the code buffer to stdout
	void printCodeBuffer();

	// ******** Methods to handle the data section ******** //
	//write a line to the global section
	void emitGlobal(const string& dataLine);
	//print the content of the global buffer to stdout
	void printGlobalBuffer();

	// ******** Methods to produce LLVM IR ******** //
	void emitLibFuncs();
	
	/**
	 * @brief creates a new register and assigns it the value of 'src_reg_type'.
	 * @param src_reg_type - either a name of a register in the form of '%reg' or an immidiate value like '3'.
	 * @param new_reg_prefix - the prefix to the name of the newly created register.
	 * @return the name of the newly created register.
	 **/
	string emitCopyReg(const string& src_reg_or_imm, ExpType src_reg_type, const string& new_reg_prefix = "copy");
	//string emitRegDecl(const string& lvalue_id, const string& rvalue_exp); 
	void emitStoreVar(const string& id, Expression* exp_to_assign);
	void emitStoreVar(const string& id, const string& reg_or_immidiate);
	void emitFuncDecl(const string& id);
	Expression* emitFunctionCall(const string& func_id, const vector<Expression*>& param_expressions);
	Expression* emitLoadVar(const string& id);
	Expression* createIdentifiableFromReg(const string& reg_name, ExpType type, bool rvalue_reg_is_raw_data);
	
	string createPtrToStackVar(int offset);
	string getFreshReg(const string& reg_name = "reg");
	string IrDefaultTypedValue(ExpType type);
	string IrType(ExpType type);
	string IrRelopType(Relop rel_type, ExpType type);
	string IrFuncTypeFormat(const string& func_id);
	string relopRvalFormat(const string& first_reg, const string& second_reg, ExpType type, Relop relop);
	string binopRvalFormat(const string& first_reg, const string& second_reg, ExpType type, Binop binop);
	string literalRvalFormat(int value, ExpType type);
private:
	int reg_count = 1;
	void emitStoreVarBasic(const string& id, const string& immidiate_or_reg);
};

#endif

