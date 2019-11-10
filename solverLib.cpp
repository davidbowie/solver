/*
EXP-SOLVER - program calculating math expressions

solverLib.cpp -  main library

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

//#define DEBUG

#include "solverTempl.h"
#include "solverLib.h"
#include <iostream>

using namespace std;

//Initialize static members
OperatorClasses* Expression::operatorClasses = NULL;
OperatorClasses* OperatorClasses::thisInstance = NULL;
Mapping* Expression::operatorMapping = NULL;
StatesMap* Expression::states = NULL;

CharType Expression::charType = CHARTYPE_UNKNOWN;

CharType Expression::currentCharType;
CharType Expression::precedingCharType;

Byte Expression::errorCode = 0;
long Expression::currentId = 0;
long Expression::openedBraces = 0;

string Expression::str = "";
string Expression::strVar = "";
string::iterator Expression::parsePos;

/*
 *  OperatorClass class methods
 */

void OperatorClass::updateList()
{
	ListIterator<Operator> i;

	for (i = objects.begin(); i != objects.end(); ++i)
		all += (*i)->symbol();
}

/*
 *  OperatorClasses class methods
 */

OperatorClasses& OperatorClasses::Instance()
{
	static OperatorClasses instance;
	thisInstance = &instance;
	return instance;
}


void OperatorClasses::updateList()
{
	thisInstance->first();
	do {
		all += thisInstance->getCurrent()->getAll();
	} while (thisInstance->next());
}


/*
 *  OperatorBindingClasses methods
 */
OperatorBindingClasses::OperatorBindingClasses(OperatorClasses* const operatorClasses)
{
	SharedPtr<OperatorBindingClass> ptr;

	operatorClasses->first();
	do {
		ptr = SharedPtr<OperatorBindingClass>(new OperatorBindingClass(operatorClasses->getCurrent()));
		add(ptr);

	} while (operatorClasses->next());
}

void OperatorBindingClasses::bindElement(SharedPtr<Element> element)
{
	string operators, op;
	SharedPtr<OperatorClass> opClass;

	first();
	do {
		op = element->strOperator;
		opClass = getCurrent()->getOperatorClass();
		operators = opClass->getAll();
		if (operators.find_first_of(op) != string::npos)
			getCurrent()->add(element);

	} while (next());
}

/*
 *  Mapping class methods
 */

void Mapping::add(string symbol, FPtrOneArg func)
{
	m1[symbol] = func;
}

void Mapping::add(string symbol, FPtrTwoArgs func)
{
	m2[symbol] = func;
}

Numeric Mapping::operate(string symbol, Numeric a, Numeric b)
{
	return m2[symbol](a,b);
}

Numeric Mapping::operate(string symbol, Numeric a)
{
	return m1[symbol](a);
}

Mapping& Mapping::Instance()
{
	static Mapping instance;
	return instance;
}

/*
 *  StateBinding methods
 */
void StateBinding::add(CharType precedingChar, FPtr function, Action action)
{
	ElemFunction elemFunction;
	elemFunction.function = function;
	elemFunction.action = action;

	map[precedingChar] = elemFunction;
}

ReturnCode StateBinding::exec(CharType precedingChar, Expression* thisExpression)
{
	if (map[precedingChar].function != NULL)
		return (thisExpression->*(map[precedingChar].function))(map[precedingChar].action); // call function with action
	return EXEC_ERROR;
}

CharType StateBinding::getType()
{
	return currentChar;
}

MapElemFunction::iterator StateBinding::find(CharType charType)
{
	return map.find(charType);
}

MapElemFunction::iterator StateBinding::end()
{
	return map.end();
}

/*
 *  StateMap methods
 */
void StatesMap::initialize(Expression* thisExpression)
{
	SharedPtr<StateBinding> ptr;

	// currently read characters
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_DOT)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_BRACE_OPEN)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_BRACE_CLOSE)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_LETTER)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_DIGIT)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_OPERATOR)));
	add(SharedPtr<StateBinding>(new StateBinding(CHARTYPE_UNKNOWN)));

	// now we assign to them preceding characters and function to execute
	// DOT
	first();
	ptr = getCurrent();
	ptr->add(CHARTYPE_DIGIT, &Expression::_getNumber, SIGNAL_DEFAULT);

	// BRACE_OPEN
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_BRACE_OPEN, &Expression::_newSubExpression, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_BRACE_CLOSE, &Expression::_newSubExpression, ATTACH_MUL_OPERATOR);
	ptr->add(CHARTYPE_LETTER, &Expression::_newSubExpression, ATTACH_FUNCTION);
	ptr->add(CHARTYPE_DIGIT, &Expression::_newSubExpression, ATTACH_MUL_STORE_NUMBER);
	ptr->add(CHARTYPE_OPERATOR, &Expression::_newSubExpression, ATTACH_OPERATOR);
	ptr->add(CHARTYPE_UNKNOWN, &Expression::_newSubExpression, SIGNAL_DEFAULT);

	// BRACE_CLOSE
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_BRACE_CLOSE, &Expression::_closeSubExpression, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_DIGIT, &Expression::_closeSubExpression, STORE_NUMBER);

	// LETTER
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_BRACE_OPEN, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_LETTER, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_OPERATOR, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_UNKNOWN, &Expression::_getFunction, SIGNAL_DEFAULT);

	// NUMBER
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_DOT, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_BRACE_OPEN, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_DIGIT, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_OPERATOR, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_UNKNOWN, &Expression::_getNumber, SIGNAL_DEFAULT);

	// OPERATOR
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_BRACE_OPEN, &Expression::_getOperator, CHECK_IF_MINUS);
	ptr->add(CHARTYPE_BRACE_CLOSE, &Expression::_getOperator, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_DIGIT, &Expression::_getOperator, STORE_NUMBER);
	ptr->add(CHARTYPE_UNKNOWN, &Expression::_getOperator, CHECK_IF_MINUS);

	// CHARTYPE_UNKNOWN
	next();
	ptr = getCurrent();
	ptr->add(CHARTYPE_BRACE_CLOSE, &Expression::_null, SIGNAL_DEFAULT);
	ptr->add(CHARTYPE_DIGIT, &Expression::_null, STORE_NUMBER);
}

/*
 *  Expression methods
 */

// This constructor is called only once for the main expression
Expression::Expression(std::string str, OperatorClasses* operatorClasses, Mapping* operatorMapping)
{
	id = currentId++;
	this->str = str;
	this->value = 0;
	this->operatorClasses = operatorClasses;
	this->operatorMapping = operatorMapping;
	parsePos = this->str.begin();					// Start position
	this->currentCharType = CHARTYPE_UNKNOWN;
	this->precedingCharType = CHARTYPE_UNKNOWN;


	states = new StatesMap();
	states->initialize(this);

	operatorBindingClasses = SharedPtr<OperatorBindingClasses>(new OperatorBindingClasses(operatorClasses));
}

// This constructor is purposed for subexpressions
Expression::Expression(std::string strOperator, std::string strFunction)
{
	id = currentId++;
	this->value = 0;
	this->strOperator = strOperator;
	this->strFunction = strFunction;

	operatorBindingClasses = SharedPtr<OperatorBindingClasses>(new OperatorBindingClasses(operatorClasses));
}


CharType Expression::getCharType()
{
	if ((*parsePos >= 'a' && *parsePos <= 'z') ||
		(*parsePos >= 'A' && *parsePos <= 'Z'))
		return CHARTYPE_LETTER;

	else if (*parsePos >= '0' && *parsePos <= '9')
		return CHARTYPE_DIGIT;

	else if (operatorClasses->getAll().find_first_of(*parsePos) != string::npos)
		return CHARTYPE_OPERATOR;

	else if (*parsePos == '(')
		return CHARTYPE_BRACE_OPEN;

	else if (*parsePos == ')')
		return CHARTYPE_BRACE_CLOSE;

	else if (*parsePos == '.')
		return CHARTYPE_DOT;

	return CHARTYPE_UNKNOWN;
}

SharedPtr<Element> Expression::getNumber()
{
	SharedPtr<Element> ptrElem = make_shared<Element>();
	ptrElem->value = stod(strVar, NULL);
	ptrElem->strOperator = temp_operator;
	strVar = "";
	return ptrElem;

}

void Expression::addElement(SharedPtr<Element> ptrElem)
{
//	cout << "Added element: " << ptrElem->strOperator << ptrElem->value << endl;
	add(ptrElem);
	operatorBindingClasses->bindElement(ptrElem);
}

ReturnCode Expression::elementaryFunctionExec(CharType currentCharType, CharType precedingCharType)
{
	SharedPtr<StateBinding> current_state;

	states->first();
	do {
		current_state = states->getCurrent();
		if ((current_state->getType() == currentCharType) &&						// check if current char matches
			(current_state->find(precedingCharType) != current_state->end())) {		// check if preceding char matches
			return current_state->exec(precedingCharType, this);					// Executing function
		}
	} while (states->next());

	return PARSE_ERROR;															// this should never happen
}

//====================================ELEMENTARY FUNCTIONS============================================

ReturnCode Expression::_newSubExpression(Action action)
{
#ifdef DEBUG
	cout << "S";
#endif
	openedBraces++;

//	return;

	string strOperator = "";
	string strFunction = "";

	switch (action) {

		case SIGNAL_DEFAULT:
			strOperator = "+";
			break;
		case ATTACH_FUNCTION:
			strOperator = this->temp_operator;
			strFunction = this->strVar;
			strVar = "";
#ifdef DEBUG
			cout << "\n\n===============FUNC!!!!===========\n\n";
#endif
			break;
		case ATTACH_MUL_STORE_NUMBER:
			addElement(getNumber());
		case ATTACH_MUL_OPERATOR:
			strOperator = "*";
			break;
		case ATTACH_OPERATOR:
			strOperator = this->temp_operator;
			break;

		default:
			return SIGNAL_ERROR;

	}

	// Creating new Expression instance
	SharedPtr<Expression> subExp = SharedPtr<Expression>(new Expression(strOperator, strFunction));
	SharedPtr<Element> subExpVal;

	this->parsePos++;
	ReturnCode code = subExp->parse();			// Parsing sub-expression

	if (code == OK)
	{
		subExpVal = subExp->calculate();
		addElement(subExpVal);
	}

	subExpVal.reset();
	subExp.reset();
	return code;
}

ReturnCode Expression::_closeSubExpression(Action action)
{
#ifdef DEBUG
	cout << "C";
#endif
	if (--openedBraces < 0)
	{
		return PARSE_ERROR;
	}

	switch (action) {

		case STORE_NUMBER:
			addElement(getNumber());
			//cout << "STORE_NUMBER!!";
		case SIGNAL_DEFAULT:
			//cout << "SUB_EXIT!!";
			return SUBEQUATION_EXIT;

		default:
			return SIGNAL_ERROR;
	}
}

ReturnCode Expression::_getNumber(Action action)
{
#ifdef DEBUG
	cout << "N";
#endif
	switch (action) {

		case SIGNAL_DEFAULT:
			strVar+=getChar();					// get character
			return OK;

		default:
			return SIGNAL_ERROR;

	}
}

ReturnCode Expression::_getFunction(Action action)
{
#ifdef DEBUG
	cout << "F";
#endif
	switch (action) {

		case SIGNAL_DEFAULT:
			strVar+=getChar();
			return OK;

		default:
			return SIGNAL_ERROR;
	}
}

ReturnCode Expression::_getOperator(Action action)
{
#ifdef DEBUG
	cout << "O";
	cout << " - SIGNAL: " << action << "\n";
#endif
	switch (action) {

		case STORE_NUMBER:
			addElement(getNumber());
		case SIGNAL_DEFAULT:
			temp_operator = getChar();
			return OK;
		case CHECK_IF_MINUS:
			temp_operator = getChar();
			if (temp_operator != "-"){
				return PARSE_ERROR;
			}
			return OK;

		default:
			return SIGNAL_ERROR;

	}
}

ReturnCode Expression::_null(Action action)
{
#ifdef DEBUG
	cout << ".";
#endif
	switch (action) {

		case STORE_NUMBER:
			addElement(getNumber());
			return OK;

		default:
			return SIGNAL_ERROR;
	}
}


//====================================================================================================

ReturnCode Expression::parse()
{
	string s;

#ifdef DEBUG
	cout << "EQUATION NO: " << this->getId() << endl;
#endif
	ReturnCode code = OK;

	temp_operator = "+";

	string::iterator end = str.end();		// Getting last position

	while (parsePos != end) {


		precedingCharType = currentCharType;
		currentCharType = getCharType();

		code = elementaryFunctionExec(currentCharType, precedingCharType);

		if (code == PARSE_ERROR) {
#ifdef DEBUG
			cout << "P";
#endif
			return code;
		}
		else
		if (code == SUBEQUATION_EXIT) {
#ifdef DEBUG
			cout << endl << "SUBEXIT CHAR: " << getChar() << endl;
#endif
			return OK;
		}
		else
			parsePos++;

	} // while(parsePos != end)

	//cout << "\n\nEND - CHAR: " << getChar() << "  TYPE: " << getCharType() << "\n";

	code = elementaryFunctionExec(CHARTYPE_UNKNOWN, currentCharType);

	if (code == PARSE_ERROR) {
#ifdef DEBUG
	    cout << "P";
#endif
		return code;
	}

	if (openedBraces != 0) {
#ifdef DEBUG
	cout << "SYNTAX ERROR" << endl;
#endif
		return PARSE_ERROR;
	}


#ifdef DEBUG
	cout << endl;
#endif
	return OK;

}


SharedPtr<Element> Expression::calculate()
{
#ifdef DEBUG
	cout << "\n\n============CALCULATING===========\n\n";
#endif
	/*
	 * Everything has been parsed into elementary elements with preceding operators
	 * Now its time to perform all aritmethic operations
	 */

	SharedPtr<OperatorBindingClass> opBindClassPtr;
	SharedPtr<OperatorClass> opClassPtr;
	SharedPtr<Element> bindElemPtr;
	SharedPtr<Element> expElemPtr;
	SharedPtr<Element> obj;
	string allOperators;
	ListIterator<Element> it;
	bool highestPriority = false;
	Numeric a, b;

	first();
	do {

		expElemPtr = getCurrent();
#ifdef DEBUG
		cout << expElemPtr->strOperator;
		cout << expElemPtr->value << " ";
#endif
	} while (next());


#ifdef DEBUG
	cout << "\n\n======================================\n\n";
#endif

	operatorBindingClasses->first();

	do {

		if (operatorBindingClasses->getIterator() ==
			operatorBindingClasses->getIteratorLast()) {
#ifdef DEBUG
		   cout << "LAST CLASS!\n";
#endif
		   highestPriority = true;
		}

		opBindClassPtr = operatorBindingClasses->getCurrent();
		opClassPtr = opBindClassPtr->getOperatorClass();
#ifdef DEBUG
		cout << "OPERATOR CLASS: " << opClassPtr->getPriority() << endl;
#endif
		opClassPtr->first();
		allOperators = opClassPtr->getAll();
#ifdef DEBUG
		cout << "OPERATORS: " << allOperators << endl;
#endif
		opBindClassPtr->first();
		if (!opBindClassPtr->empty()) {


			do {
				bindElemPtr = opBindClassPtr->getCurrent();


#ifdef DEBUG
				cout << bindElemPtr->strOperator;
				cout << bindElemPtr->value << " ";
#endif

				it = find(bindElemPtr);


				obj = *it;

				// checking if the element has function attached to it
/*				if (obj->strFunction != "" ) {
#ifdef DEBUG
					cout << "\n\n+++++++++++++CALCULATING FUNCTION!+++++++++++\nVALUE BEFORE: " << obj->value;
#endif
					// assigning the value returned from the function
					b = operatorMapping->operate(bindElemPtr->strFunction, obj->value);
#ifdef DEBUG
					cout << "\nVALUE AFTER: " << b << "\n\n";
#endif
				}
				else*/
					// no function
					b = obj->value;						// Getting numeric value

				del(it);							// Deleting pointer from list
				obj.reset();						// Deleting ownership of Element

				if (highestPriority) {
					this->value =
						operatorMapping->operate(bindElemPtr->strOperator, this->value, b);
				}
				else {
					it--;								// Getting predecessor
					obj = *it;
					a = obj->value;						// Getting value

					// Assigning new value to predecessor
					obj->value = operatorMapping->operate(bindElemPtr->strOperator, a, b);
				}

				bindElemPtr.reset();					// We wont use it anymore
			/*
			 * Ownerships of Element are fully removed, so destructor's gonna be called
			 * No memory leaks. (I think so)
			 */


			} while (opBindClassPtr->next());
		} // if (!opBindClassPtr->empty())
#ifdef DEBUG
	cout << endl;
#endif

	} while (operatorBindingClasses->next());

	operatorBindingClasses.reset(); 		// Disposing OperatorBindingClasses instance


#ifdef DEBUG
	cout << "\n\n========================================\n\n";
	cout << "Value EQ NO" << this->id << ": " << this->value << "\n\n";
#endif

	if (this->strFunction != "" ) {
#ifdef DEBUG
		cout << "\n\n+++++++++++++CALCULATING FUNCTION!+++++++++++\nVALUE BEFORE: " << this->value;
#endif
					// assigning the value returned from the function
		this->value = operatorMapping->operate(this->strFunction, this->value);
#ifdef DEBUG
		cout << "\nVALUE AFTER: " << this->value << "\n\n";
#endif
	}

	SharedPtr<Element> ptrElem = make_shared<Element>();
	ptrElem->value = this->value;
	ptrElem->strOperator = this->strOperator;

	return ptrElem;
}
