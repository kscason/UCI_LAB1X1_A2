/***************************************
 * INTERPRETER.cpp
 *
 * Student Name: Kaitlyn Cason
 * Student ID: 20282205
 * UCI Net ID: kcason
 **************************************/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>

#define DATA_SEG_SIZE 100


/*Jumpt struct holds the jump destination, boolean condition, 
	and the left-/right-hand sides of the boolean expression.*/
struct Jumpt{
	int destination;
	int lhs;
	int rhs; 
	std::string bool_condition;
};

/*Jump struct holds the jump destination.*/
struct Jump{
	int destination;
};

/*Set struct holds the destination, source, and booleans if 
	the destination and source are either write and read,
	respectively. Destination and source are uninitialized
	and not used if isWrite and isRead are TRUE.*/
struct Set{
	int destination;
	int source;
	bool isRead;
	bool isWrite;
};

/*Statement struct holds possible Set, Jump, or Jumpt structs
	depending on the statement. The Set, Jump, or Jumpt structs
	are initialized and used only if its corresponding boolean
	(isSet, isJump, isJumpt) is TRUE. Boolean isHalt used if 
	statement is "halt."*/
struct Statement{
	Set set;
	Jump jump;
	Jumpt jumpt;
	bool isHalt;
	bool isSet;
	bool isJump;
	bool isJumpt;
};

class INTERPRETER {
public:
	INTERPRETER(char *sourceFile);
	void runProgram();

private:
	std::ifstream codeIn;
	std::ifstream inputIn;
	std::ofstream outFile;

	std::vector<std::string> C;
		int D[DATA_SEG_SIZE];
		int PC;
	std::string IR;
		bool run_bit;

	unsigned curIRIndex;

	void printDataSeg();

	void fetch();
	void incrementPC();
	void execute();

	// Output: used in the case of: set write, source
	void write(int source);

	// Input: used in the case of: set destination, read
	int read();

	/**
	 * Checks and returns if the character c is found at the current
	 * position in IR. If c is found, advance to the next
	 * (non-whitespace) character.
	 */
	bool accept(char c);

	/**
	 * Checks and returns if the string s is found at the current
	 * position in IR. If s is found, advance to the next
	 * (non-whitespace) character.
	 */
	bool accept(const char *s);

	/**
	 * Checks if the character c is found at the current position in
	 * IR. Throws a syntax error if c is not found at the current
	 * position.
	 */
	void expect(char c);

	/**
	 * Checks if the string s is found at the current position in
	 * IR. Throws a syntax error if s is not found at the current
	 * position.
	 */
	void expect(const char *s);

	void skipWhitespace();

	Statement parseStatement();
	Set parseSet();
	Jump parseJump();
	Jumpt parseJumpt();
	int parseExpr();
	int parseTerm();
	int parseFactor();
	int parseNumber();

	void syntaxError();
};


INTERPRETER::INTERPRETER(char *sourceFile) {
	codeIn.open(sourceFile, std::fstream::in);
	if (codeIn.fail()) {
		std::cerr << "init: Errors accessing source file "
							<< sourceFile << std::endl;
		exit(-2);
	}

	inputIn.open("input.txt", std::fstream::in);
	if (inputIn.fail()) {
		std::cerr << "init: Errors accessing input file input.txt" << std::endl;
		exit(-2);
	}

	outFile.open((std::string(sourceFile) + ".out").c_str(), std::fstream::out);
	if (outFile.fail()) {
		std::cerr << "init: Errors accessing output file "
							<< std::string(sourceFile) + ".out" << std::endl;
		exit(-2);
	}

	// Initialize the SIMPLESEM processor state
	// Initialize the Code segment
	while (codeIn.good()) {
		std::string line;
		std::getline(codeIn, line);
		C.push_back(line);
	}

	// Initialize the Data segment
	for (int i=0; i<DATA_SEG_SIZE ;i++) {
		D[i]=0;
	}
	PC = 0; // Every SIMPLESEM program begins at instruction 0
	run_bit = true; // Enable the processor
}

/*Run program while the run_bit is TRUE. Fetch new instruction,
	increment the PC, and execute the instruction.*/
void INTERPRETER::runProgram() {
	//FETCH-INCREMENT-EXECUTE CYCLE
	while(run_bit){
		fetch();
		incrementPC();
		execute();
	}
	printDataSeg();
}

void INTERPRETER::printDataSeg() {
	outFile << "Data Segment Contents" << std::endl; //outFile
	for(int i=0; i < DATA_SEG_SIZE; i++){
		outFile << i << ": " << D[i] << std::endl; //outFile
	}
}

/*Fetch instruction grabs the next instruction to be executed from
	the code segment according to the program counter. We reset the
	current IR index back to 0 (since we grab a new IR).*/
void INTERPRETER::fetch() {
	IR = C[PC];
	curIRIndex = 0;
}

/*Increment PC increments the program counter by one, in order to
	access the correct code segment*/
void INTERPRETER::incrementPC() {
	PC = PC+1;
}

/*Execute first parses the instruction statement. The instruction
	op code falls into 4 categories SET, JUMPT, JUMP, HALT. Evaluate
	and execute the op code structs based off of the given
	instruction semantics.*/
void INTERPRETER::execute() {
	//Parse current statement
	printf("%s\n",IR.c_str());
	Statement instr = parseStatement();


	//If the instruction is SET
	if(instr.isSet){
		/*Access SET struct from the instruction and change
			the data segment as specified in the specifications.*/
		Set mySet = instr.set;
		if(mySet.isWrite && mySet.isRead)			//set write, read
			write(read());
		else if(mySet.isWrite)						//set write, source
			write(mySet.source);
		else if(mySet.isRead)						//set destination, read
			D[mySet.destination] = read();
		else
			D[mySet.destination] = mySet.source;	//set destination, source
	}

	//If the instruction is JUMPT
	else if(instr.isJumpt){
		/*Access JUMPT struct from the instruction and change
			the program counter if the boolean condition
			evaluates to TRUE.*/
		Jumpt myJumpt = instr.jumpt;

		bool bool_eval = false;
		if(myJumpt.bool_condition == "!=")			//lhs != rhs
			bool_eval = myJumpt.lhs != myJumpt.rhs;
		else if(myJumpt.bool_condition == "==")		//lhs == rhs
			bool_eval = myJumpt.lhs == myJumpt.rhs;
		else if(myJumpt.bool_condition == ">=")		//lhs >= rhs
			bool_eval = myJumpt.lhs >= myJumpt.rhs;
		else if(myJumpt.bool_condition == "<=")		//lhs <= rhs
			bool_eval = myJumpt.lhs <= myJumpt.rhs;
		else if(myJumpt.bool_condition == ">")		//lhs > rhs
			bool_eval = myJumpt.lhs > myJumpt.rhs;
		else if(myJumpt.bool_condition == "<")		//lhs < rhs
			bool_eval = myJumpt.lhs < myJumpt.rhs;
		else
			syntaxError();

		//If the boolean condition evaluates to true, change PC
		if(bool_eval)
			PC = myJumpt.destination;
	}

	//If the instruction is JUMP
	else if(instr.isJump){
		//Change the PC to the given destination
		PC = instr.jump.destination;
	}

	//If the instruction is HALT
	else if(instr.isHalt){
		//Change the run_bit to FALSE (to stop the runProgram() cycle)
		run_bit = false;
	}


}

//Output: used in the case of: set write, source
void INTERPRETER::write(int source){
		outFile << source << std::endl;
}

//Input: used in the case of: set destination, read
int INTERPRETER::read() {
	int value;
	inputIn >> value;
	return value;
}

/**
 * Checks and returns if the character c is found at the current
 * position in IR. If c is found, advance to the next
 * (non-whitespace) character.
 */
bool INTERPRETER::accept(char c) {
	if (curIRIndex >= IR.length())
		return false;

	if (IR[curIRIndex] == c) {
		curIRIndex++;
		skipWhitespace();
		return true;
	} else {
		return false;
	}
}

/**
 * Checks and returns if the string s is found at the current
 * position in IR. If s is found, advance to the next
 * (non-whitespace) character.
 */
bool INTERPRETER::accept(const char *s) {
	unsigned s_len = strlen(s);

	if (curIRIndex+s_len > IR.length())
		return false;

	for (unsigned i = 0; i < s_len; ++i) {
		if (IR[curIRIndex+i] != s[i]) {
			return false;
		}
	}

	curIRIndex += s_len;
	skipWhitespace();
	return true;
}

/**
 * Checks if the character c is found at the current position in
 * IR. Throws a syntax error if c is not found at the current
 * position.
 */
void INTERPRETER::expect(char c) {
	if (!accept(c)){
		syntaxError();
	}
}

/**
 * Checks if the string s is found at the current position in
 * IR. Throws a syntax error if s is not found at the current
 * position.
 */
void INTERPRETER::expect(const char *s) {
	if (!accept(s))
		syntaxError();
}

void INTERPRETER::skipWhitespace() {
	while (curIRIndex < IR.length() && isspace(IR[curIRIndex]))
		curIRIndex++;
}

/*Parse statement by checking if first word is SET, JUMPT, JUMP, or
	HALT. Assign the correct struct (if any) in a Statement struct 
	to return to the execute() function. Also, set the correct
	boolean to TRUE depending on which instruction code we see.
	Returns a Statement structure.*/
Statement INTERPRETER::parseStatement() {
	std::cerr << "Statement" << std::endl;

	//Initialize the Statement struct, and its booleans to false.
	Statement stmt = {};
	stmt.isHalt = false;
	stmt.isSet = false;
	stmt.isJumpt = false;
	stmt.isJump = false;

	/*Change the boolean to TRUE depending on instruction code and 
		initialize any additional structures if not HALT.*/
	if (accept("halt")){
		stmt.isHalt = true;
	}
	else if(accept("set")){
		stmt.set = parseSet();
		stmt.isSet = true;
	}
	else if(accept("jumpt")){
		stmt.jumpt = parseJumpt();
		stmt.isJumpt = true;
	}
	else if(accept("jump")){
		stmt.jump = parseJump();
		stmt.isJump = true;
	}
	else
		syntaxError();

	return stmt;
}

/*Parse set by checking if WRITE or READ is found. Assign values
	to the source or destination elements in a Set structure if a
	READ or WRITE isn't found, respectively.
	Returns a Set structure.*/
Set INTERPRETER::parseSet() {
	std::cerr << "Set" << std::endl;

	int source = 100, destination = 100;
	bool isWrite = false, isRead = false;

	//Check if write is found, parseExpr() if not
	if (!accept("write")){
		destination = parseExpr();
	}
	else
		isWrite = true;
		

	expect(',');

	//Check if read is found, parseExpr() if not
	if (!accept("read")){
		source = parseExpr();
	}
	else
		isRead = true;
		

	//Initialize the set object to return
	Set set = {};
	set.source = source;
	set.destination = destination;
	set.isRead = isRead;
	set.isWrite = isWrite;

	return set;
		
}

/*Parse Expression by resolving the first Term, and accepting
	any + <Term> or - <Term> pairs found in the expression.
	Return the resolved value. */
int INTERPRETER::parseExpr() {
	std::cerr << "Expr" << std::endl;

	int retVal;
	char op;

	retVal = parseTerm();

	bool accepting = true;
	while(accepting){
		//Grab the current operator (+ or -) & resolve accordingly.
		op = IR[curIRIndex];
		if(accept('+') || accept('-')){
			if(op == '+')
				retVal = retVal + parseTerm();
			else //if op == '-')
				retVal = retVal - parseTerm();
		}
		else
			accepting = false; //Break out of loop when there are no more ops
	}

	return retVal;
}

/*Parse Term by resolving the first Factor, and accepting
	any * <Factor>, / <Factor>, or % <Factor> pairs found 
	in the term. Return the resolved value. */
int INTERPRETER::parseTerm() {
	std::cerr << "Term" << std::endl;

	int retVal;
	char op;

	retVal = parseFactor();

	bool accepting = true;
	while(accepting){
		//Grab the current operator (*, /, or %) & resolve accordingly.
		op = IR[curIRIndex];
		if (accept('*') || accept('/') || accept('%')){
			if(op == '*')
				retVal = retVal * parseFactor();
			else if(op == '/')
				retVal = retVal / parseFactor();
			else // if(op == '%')
				retVal = retVal % parseFactor();
		}
		else
			accepting = false; //Break out of loop when there are no more ops
	}

	return retVal;

}

/*Parse Factor by resolving either the Number, D[<Expr>], or (<Expr>). 
	Return the resolved value. */
int INTERPRETER::parseFactor() {
	std::cerr << "Factor" << std::endl;

	//Resolve return value based on which Factor option we have
	int retVal;
	if (accept("D[")) {
		retVal = D[parseExpr()]; //D[<Expr>]

		expect(']');
	} 
	else if (accept('(')) {
		retVal = parseExpr(); //(<Expr>)

		expect(')');
	} 
	else {
		retVal = parseNumber(); //<Number>
	}
	return retVal;
}

/*Parse Number by resolving either the value. Returns the value.*/
int INTERPRETER::parseNumber() {
	std::cerr << "Number" << std::endl;

	//Initialize the digit string
	std::string retVal;

	if (curIRIndex >= IR.length())
		syntaxError();

	//If the digit is 0
	if (IR[curIRIndex] == '0') {
		retVal = (IR[curIRIndex]);
		curIRIndex++;
		skipWhitespace();
	} 
	//Otherwise append the digit characters to retVal
	else if (isdigit(IR[curIRIndex])) {
		while (curIRIndex < IR.length() &&
					 isdigit(IR[curIRIndex])) {
			retVal.append(IR,curIRIndex,1);
			curIRIndex++;
		}
		skipWhitespace();
	} 
	else {
		
		syntaxError();
	}
	//Return a value, not string
	return stoi(retVal);
}

/*Parse Jump by parsing the Expression and saving the result
	into a Jump structure to use in the execute() function. 
	Return the Jump structure.*/
Jump INTERPRETER::parseJump() {
	std::cerr << "Jump" << std::endl;

	Jump jump = {parseExpr()};

	return jump;
}

/*Parse Jumpt by parsing the Expression and saving the result
	into a Jumpt structure to use in the execute() function. 
	Grab the boolean comparison operator and both the left-/right-
	hand sides of the boolean expression to be evaluated.
	Return the Jumpt structure.*/
Jumpt INTERPRETER::parseJumpt() {
	std::cerr << "Jumpt" << std::endl;

	Jumpt jumpt = {};
	jumpt.destination = parseExpr();

	expect(',');

	//Grab left hand side of boolean expression
	jumpt.lhs = parseExpr();
	
	/*Grab the boolean comparison operator. The operator is
		either 1 or 2 characters. If 2 chars, it ends in a '='.*/
	if(IR[curIRIndex+1] != '=')
		jumpt.bool_condition.append(IR,curIRIndex,1);
	else
		jumpt.bool_condition.append(IR,curIRIndex,2);

	//Grab right hand side of boolean expression
	if (accept("!=") || accept("==") || accept(">=") || accept("<=") ||
			accept('>') || accept('<')) {
		jumpt.rhs = parseExpr();
	} 
	else {
		syntaxError();
	}

	return jumpt;
}

void INTERPRETER::syntaxError() {
	std::cerr << "Syntax Error!" << std::endl;
	exit(-1);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "init: This program requires an input "
							<< "filename as a parameter " << std::endl;
		exit(-2);
	}
	INTERPRETER i(argv[1]);
	i.runProgram();
}
