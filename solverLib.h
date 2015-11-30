/*
EXP-SOLVER - program calculating math expressions

solverLib.h -  Library header file

Copyright (C) 2015 Marcin Możejko

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLVERLIB_H
#define SOLVERLIB_H

#include <string>
#include <list>
#include <map>
#include "solverTempl.h"

typedef enum ReturnCodeEnum {
	OK,
	PARSE_ERROR,
	EXEC_ERROR,
	SIGNAL_ERROR,
	SUBEQUATION_EXIT,
} ReturnCode;

typedef enum CharTypeEnum {
	type_unknown,
	type_digit,
	type_letter,
	type_operator,
	type_brace_open,
	type_brace_close,
	type_dot
} CharType;

typedef enum SignalEnum {
	SIGNAL_DEFAULT,
	STORE_NUMBER,
	STORE_FUNCTION,
	//SUB_EQUATION SIGNALS
	ATTACH_FUNCTION,
	ATTACH_OPERATOR,
	ATTACH_MUL_OPERATOR,
	ATTACH_MUL_STORE_NUMBER,
	CHECK_IF_MINUS
} Signal;	


typedef unsigned short int Byte;
typedef double Numeric;

class Expression;

typedef Numeric (*FPtrOneArg) (Numeric);
typedef Numeric (*FPtrTwoArgs)(Numeric, Numeric);
typedef ReturnCode (Expression::*FPtr)(Signal);

typedef struct ElemFunction {
				FPtr function;
				Signal signal;
} ElemFunction;

typedef std::map<std::string, FPtrOneArg> MapOneArg;
typedef std::map<std::string, FPtrTwoArgs> MapTwoArgs;
typedef std::map<CharType, ElemFunction> MapElemFunction;


typedef struct EssentialElement {
				Numeric value;
				std::string strOperator;
} EssentialElement;


class Mapping {

	MapOneArg m1;
	MapTwoArgs m2;

	Mapping(){};
	Mapping(Mapping const&) = delete;
	Mapping& operator=(Mapping const&) = delete;

public:
	void add(std::string symbol, FPtrOneArg func);
	void add(std::string symbol, FPtrTwoArgs func);

	Numeric operate(std::string symbol, Numeric a, Numeric b);
	Numeric operate(std::string symbol, Numeric a);

	static Mapping& Instance();
};

// Forward declarations 
class OperatorClasses;
class OperatorBindingClasses;
class StatesMap;

class Expression : List<EssentialElement> {

	static CharType charType;

	static long openedBraces;		// Number of braces opened

	static long currentId; // wywalic 

	long id; // do wywalenia potem
	static std::string strVar;				// temporary field holding number or function name
	std::string strFunction;		// function
	std::string strOperator;		// operator

	static Byte errorCode;	
	static std::string str;				// String to be parsed

	static CharType currentCharType;
	static CharType precedingCharType;

	static std::string::iterator parsePos;	// Current position in parsing str
	std::string temp_operator;


	Numeric value;					// Value
	static StatesMap* states;
	static OperatorClasses* operatorClasses;
    static Mapping* operatorMapping;	
	SharedPtr<OperatorBindingClasses> operatorBindingClasses;

	CharType getCharType();
	char getChar() {return *parsePos;}
	
	SharedPtr<EssentialElement> getNumber();
	void addElement(SharedPtr<EssentialElement> ptrElem);

	Expression(std::string strOperator, std::string strFunction);

	ReturnCode elementaryFunctionExec(CharType currentCharType, CharType precedingCharType);
	
	// Elementary functions
	
	ReturnCode _newSubExpression(Signal signal);
	ReturnCode _closeSubExpression(Signal signal);
	ReturnCode _getNumber(Signal signal);
	ReturnCode _getFunction(Signal signal);
	ReturnCode _getOperator(Signal signal);
	ReturnCode _null(Signal signal);


public:
	Expression(std::string str, OperatorClasses* operatorClasses, Mapping* operatorMapping);
	Numeric getValue() {return value;}
	long getId() {return id;}            // WYWALIC
	SharedPtr<EssentialElement> calculate();
	ReturnCode parse();

	friend class StatesMap;

};

class Operator {
/* Class holding single operator */

	std::string sym;												// Symbol

public:
	Operator(std::string symbol) : sym(symbol) {}
	std::string symbol() {return sym;}								// Return symbol
};

class OperatorClass : public List<Operator> {
/* Class handling operator classes due to its priority
 *	for ex.: (add, sub), (div, mul), (pow) 
 */
	std::string all;
	Byte priority;

public:
	OperatorClass(Byte priority) :
		priority(priority) {}

	void updateList();
	std::string getAll(){return all;}							// List all operators
	Byte getPriority(){return priority;}

};

class OperatorClasses : public List<OperatorClass> {

	std::string all;
	static OperatorClasses* thisInstance;

	OperatorClasses(){};
	OperatorClasses(OperatorClasses const&) = delete;
	OperatorClasses& operator=(OperatorClasses const&) = delete;
	
public:
	static OperatorClasses& Instance();
	void updateList();
	std::string getAll(){return all;}
};

/*
 * Classes similar to OperatorClass and OperatorClasses, used to optimize the calculating algorithm
 */
class OperatorBindingClass : public List<EssentialElement> {

	SharedPtr<OperatorClass> operatorClass;

	OperatorBindingClass(SharedPtr<OperatorClass> operatorClass) :
		operatorClass(operatorClass){};
	SharedPtr<OperatorClass> getOperatorClass() {return operatorClass;}

	friend class OperatorBindingClasses;
	friend class Expression;

};

class OperatorBindingClasses : public List<OperatorBindingClass> {

	OperatorBindingClasses(OperatorClasses* const operatorClasses);
	void bindElement(SharedPtr<EssentialElement> essentialElement);

	friend class Expression;

};


class StateBinding {

	CharType currentChar;
	MapElemFunction map;

	StateBinding(CharType currentChar) : currentChar(currentChar) {};
	void add(CharType precedingChar, FPtr function, Signal signal);
	ReturnCode exec(CharType precedingChar, Expression* thisExpression);
	MapElemFunction::iterator find(CharType charType);
	MapElemFunction::iterator end();

	CharType getType();

	friend class StatesMap;
	friend class Expression;
};

class StatesMap : public List<StateBinding> {

	void initialize(Expression* thisExpression);

	friend class Expression;
};


#endif // EQUATIONLIB_H
