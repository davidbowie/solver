// SOLVERLIB.CPP
//

#include "solverTempl.h"
#include "solverLib.h"
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

//Initialize static members
OperatorClasses* Expression::operatorClasses = NULL;
OperatorClasses* OperatorClasses::thisInstance = NULL;
Mapping* Expression::operatorMapping = NULL;
StatesMap* Expression::states = NULL;

CharType Expression::charType = type_unknown;

CharType Expression::currentCharType;
CharType Expression::precedingCharType;

Byte Expression::errorCode = 0;
long Expression::currentId = 0;
long Expression::openedBraces = 0;

string Expression::str = "";
string Expression::strVar = "";
string::iterator Expression::parsePos;


/*
 * Functions class methods
 */

Functions& Functions::Instance() {

	static Functions instance;
	return instance;
}



/*
 *  OperatorClass class methods 
 */

void OperatorClass::updateList() {

	ListIterator<Operator> i;

	for (i = objects.begin(); i != objects.end(); ++i)
		all += (*i)->symbol();
}

/*
 *  OperatorClasses class methods 
 */

OperatorClasses& OperatorClasses::Instance() {

	static OperatorClasses instance;
	thisInstance = &instance;
	return instance;
}


void OperatorClasses::updateList() {
	
	thisInstance->first();
	do {
		all += thisInstance->getCurrent()->getAll();
	} while (thisInstance->next()); 	
}


/*
 *  OperatorBindingClasses methods
 */
OperatorBindingClasses::OperatorBindingClasses(OperatorClasses* const operatorClasses) {

	SharedPtr<OperatorBindingClass> ptr;

	operatorClasses->first();
	do {
		ptr = SharedPtr<OperatorBindingClass>(new OperatorBindingClass(operatorClasses->getCurrent()));
		//ptr = make_shared<OperatorBindingClass>(operatorClasses->getCurrent());
		add(ptr);	

	} while (operatorClasses->next());
}

void OperatorBindingClasses::bindElement(SharedPtr<EssentialElement> essentialElement) {

	string operators, op;
	SharedPtr<OperatorClass> opClass;


	first();
	do {
		op = essentialElement->strOperator;
		opClass = getCurrent()->getOperatorClass();
		operators = opClass->getAll();
		if (operators.find_first_of(op) != string::npos)
			getCurrent()->add(essentialElement);

	} while (next());
}

/*
 *  Mapping class methods
 */

void Mapping::add(string symbol, FPtrOneArg func) {

	m1[symbol] = func;
}

void Mapping::add(string symbol, FPtrTwoArgs func) {

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

Mapping& Mapping::Instance() {

	static Mapping instance;
	return instance;
}

/* 
 *  StateBinding methods
 */
void StateBinding::add(CharType precedingChar, FPtr function, Signal signal) {

	ElemFunction elemFunction;
	elemFunction.function = function;
	elemFunction.signal = signal;

	map[precedingChar] = elemFunction;
}

ReturnCode StateBinding::exec(CharType precedingChar, Expression* thisExpression) {

	if (map[precedingChar].function != NULL)
		return (thisExpression->*(map[precedingChar].function))(map[precedingChar].signal); // call function with signal
	return EXEC_ERROR;
}

CharType StateBinding::getType() {

	return currentChar; 
}

MapElemFunction::iterator StateBinding::find(CharType charType) {

	return map.find(charType);
}

MapElemFunction::iterator StateBinding::end() {

	return map.end();
}

/*
 *  StateMap methods
 */
void StatesMap::initialize(Expression* thisExpression) {

	SharedPtr<StateBinding> ptr;

	// currently read characters
	add(SharedPtr<StateBinding>(new StateBinding(type_dot)));
	add(SharedPtr<StateBinding>(new StateBinding(type_brace_open)));
	add(SharedPtr<StateBinding>(new StateBinding(type_brace_close)));
	add(SharedPtr<StateBinding>(new StateBinding(type_letter)));
	add(SharedPtr<StateBinding>(new StateBinding(type_digit)));
	add(SharedPtr<StateBinding>(new StateBinding(type_operator)));
	add(SharedPtr<StateBinding>(new StateBinding(type_unknown)));
	
	// now we assign to them preceding characters and function to execute
	// DOT
	first();
	ptr = getCurrent();
	ptr->add(type_digit, &Expression::_getNumber, SIGNAL_DEFAULT);
	
	// BRACE_OPEN
	next();
	ptr = getCurrent();
	ptr->add(type_brace_open, &Expression::_newSubExpression, SIGNAL_DEFAULT);
	ptr->add(type_brace_close, &Expression::_newSubExpression, ATTACH_MUL_OPERATOR);
	ptr->add(type_letter, &Expression::_newSubExpression, ATTACH_FUNCTION);
	ptr->add(type_digit, &Expression::_newSubExpression, ATTACH_MUL_STORE_NUMBER);
	ptr->add(type_operator, &Expression::_newSubExpression, ATTACH_OPERATOR);
	ptr->add(type_unknown, &Expression::_newSubExpression, SIGNAL_DEFAULT);
	
	// BRACE_CLOSE
	next();
	ptr = getCurrent();
	ptr->add(type_brace_close, &Expression::_closeSubExpression, SIGNAL_DEFAULT);
	ptr->add(type_digit, &Expression::_closeSubExpression, STORE_NUMBER);

	// LETTER
	next();
	ptr = getCurrent();
	ptr->add(type_brace_open, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(type_letter, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(type_operator, &Expression::_getFunction, SIGNAL_DEFAULT);
	ptr->add(type_unknown, &Expression::_getFunction, SIGNAL_DEFAULT);

	// NUMBER 
	next();
	ptr = getCurrent();
	ptr->add(type_dot, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(type_brace_open, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(type_digit, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(type_operator, &Expression::_getNumber, SIGNAL_DEFAULT);
	ptr->add(type_unknown, &Expression::_getNumber, SIGNAL_DEFAULT);

	// OPERATOR
	next();
	ptr = getCurrent();
	ptr->add(type_brace_close, &Expression::_getOperator, SIGNAL_DEFAULT);
	ptr->add(type_digit, &Expression::_getOperator, STORE_NUMBER);
	ptr->add(type_unknown, &Expression::_getOperator, SIGNAL_DEFAULT);

	// UNKNOWN
	next();
	ptr = getCurrent();
	ptr->add(type_brace_close, &Expression::_null, SIGNAL_DEFAULT);
	ptr->add(type_digit, &Expression::_null, STORE_NUMBER);
}

/*
 *  Expression methods
 */

// This constructor is called only once for the main expression
Expression::Expression(std::string str, OperatorClasses* operatorClasses, Mapping* operatorMapping) {

	id = currentId++;
	this->str = str;
	this->value = 0;
	this->operatorClasses = operatorClasses;
	this->operatorMapping = operatorMapping;
	parsePos = this->str.begin();					// Start position
	this->currentCharType = type_unknown;
	this->precedingCharType = type_unknown;


	states = new StatesMap();
	states->initialize(this);

	operatorBindingClasses = SharedPtr<OperatorBindingClasses>(new OperatorBindingClasses(operatorClasses));
}

// This constructor is purposed for subexpressions
Expression::Expression(std::string strOperator, std::string strFunction) {

	id = currentId++;
	this->value = 0;
	this->strOperator = strOperator;
	this->strFunction = strFunction;

	operatorBindingClasses = SharedPtr<OperatorBindingClasses>(new OperatorBindingClasses(operatorClasses));
}


CharType Expression::getCharType() {
	
	if ((*parsePos >= 'a' && *parsePos <= 'z') ||
		(*parsePos >= 'A' && *parsePos <= 'Z'))
		return type_letter;

	else if (*parsePos >= '0' && *parsePos <= '9')
		return type_digit;

	else if (operatorClasses->getAll().find_first_of(*parsePos) != string::npos)
		return type_operator;

	else if (*parsePos == '(')
		return type_brace_open;
	
	else if (*parsePos == ')')
		return type_brace_close;

	else if (*parsePos == '.')
		return type_dot;

	return type_unknown;	
}

SharedPtr<EssentialElement> Expression::getNumber() {

	SharedPtr<EssentialElement> ptrElem = make_shared<EssentialElement>();
	ptrElem->value = stod(strVar, NULL);
	ptrElem->strOperator = temp_operator;
	strVar = "";
	return ptrElem;

	// TODO: FUNKCJE!!!!

}

void Expression::addElement(SharedPtr<EssentialElement> ptrElem) {

//	cout << "Added element: " << ptrElem->strOperator << ptrElem->value << endl;
	add(ptrElem);
	operatorBindingClasses->bindElement(ptrElem);

}

ReturnCode Expression::elementaryFunctionExec(CharType currentCharType, CharType precedingCharType){

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

ReturnCode Expression::_newSubExpression(Signal signal){

	cout << "S";
	openedBraces++;

//	return;

	string strOperator = "";
	string strFunction = "";

	switch (signal) {

		case SIGNAL_DEFAULT:
			strOperator = "+";
			break;
		case ATTACH_FUNCTION:
			strOperator = this->temp_operator;
			strFunction = this->strVar;
			strVar = "";
			cout << "\n\n===============FUNC!!!!===========\n\n";
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

	} //switch(signal)

	// Creating new Expression instance
	SharedPtr<Expression> subExp = SharedPtr<Expression>(new Expression(strOperator, strFunction));
	SharedPtr<EssentialElement> subExpVal;

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

ReturnCode Expression::_closeSubExpression(Signal signal){

	cout << "C";
	openedBraces--;

	switch (signal) {

		case STORE_NUMBER:
			addElement(getNumber());
			//cout << "STORE_NUMBER!!";
		case SIGNAL_DEFAULT:
			//cout << "SUB_EXIT!!";
			return SUBEQUATION_EXIT;	

		default:
			return SIGNAL_ERROR;
	} // switch(signal)

}

ReturnCode Expression::_getNumber(Signal signal){

	cout << "N";
	
	switch (signal) {
	
		case SIGNAL_DEFAULT:
			strVar+=getChar();					// get character
			return OK;

		default:
			return SIGNAL_ERROR;

	} // switch(signal)
}

ReturnCode Expression::_getFunction(Signal signal){

	cout << "F";

	switch (signal) {

		case SIGNAL_DEFAULT:
			strVar+=getChar();
			return OK;

		default:
			return SIGNAL_ERROR;
	} //switch(signal)
}

ReturnCode Expression::_getOperator(Signal signal){

	cout << "O";

	switch (signal) {

		case STORE_NUMBER:
			addElement(getNumber());
		case SIGNAL_DEFAULT:
			temp_operator = getChar();
			return OK;

		default:
			return SIGNAL_ERROR;			

	} // switch (signal)
}

ReturnCode Expression::_null(Signal signal){

	cout << ".";

	switch (signal) {
	
		case STORE_NUMBER:
			addElement(getNumber());
			return OK;

		default:
			return SIGNAL_ERROR;	
	} // switch(signal)

}


//====================================================================================================

ReturnCode Expression::parse(){

	string s;

	cout << "EQUATION NO: " << this->getId() << endl;

	ReturnCode code = OK;

	temp_operator = "+";

	string::iterator end = str.end();		// Getting last position

	while (parsePos != end) {
	
	
		precedingCharType = currentCharType;
		currentCharType = getCharType();

		code = elementaryFunctionExec(currentCharType, precedingCharType);

		if (code == PARSE_ERROR) {
			cout << "P";
			return code;
		}
		else
		if (code == SUBEQUATION_EXIT) {
			cout << endl << "SUBEXIT CHAR: " << getChar() << endl;
			return OK;
		}
		else
			parsePos++;
	
	} // while(parsePos != end)

	//cout << "\n\nEND - CHAR: " << getChar() << "  TYPE: " << getCharType() << "\n";

	code = elementaryFunctionExec(type_unknown, currentCharType);

	if (code == PARSE_ERROR) {
		cout << "P";
		return code;
	}

	if (openedBraces) {
		cout << "SYNTAX ERROR" << endl;
		return PARSE_ERROR;
	}


	cout << endl;

	return OK;

}


SharedPtr<EssentialElement> Expression::calculate(){

	cout << "\n\n============CALCULATING===========\n\n";

	/*
	 * Everything has been parsed into elementary elements with preceding operators
	 * Now its time to perform all aritmethic operations
	 */

	SharedPtr<OperatorBindingClass> opBindClassPtr;
	SharedPtr<OperatorClass> opClassPtr;
	SharedPtr<EssentialElement> bindElemPtr;
	SharedPtr<EssentialElement> expElemPtr;
	SharedPtr<EssentialElement> obj;
	string allOperators;
	ListIterator<EssentialElement> it;
	bool highestPriority = false;
	Numeric a, b;

	first();
	do {

		expElemPtr = getCurrent();
        cout << expElemPtr->strOperator;
		cout << expElemPtr->value << " ";

	} while (next());


	cout << "\n\n======================================\n\n";


	operatorBindingClasses->first();

	do {

		if (operatorBindingClasses->getIterator() ==
			operatorBindingClasses->getIteratorLast()) {
			   cout << "LAST CLASS!\n";
			   highestPriority = true;
		}
		
		opBindClassPtr = operatorBindingClasses->getCurrent();
		opClassPtr = opBindClassPtr->getOperatorClass();
		cout << "OPERATOR CLASS: " << opClassPtr->getPriority() << endl;

		opClassPtr->first();
		allOperators = opClassPtr->getAll();			
		cout << "OPERATORS: " << allOperators << endl;

		opBindClassPtr->first();
		if (!opBindClassPtr->empty()) {
		
			//cout << "\n\nWESZLO!!               \n\n";

			do {
				bindElemPtr = opBindClassPtr->getCurrent();

				//cout << "++++++++++++++++++++++";

				cout << bindElemPtr->strOperator;
				cout << bindElemPtr->strFunction << "(";
				cout << bindElemPtr->value << ") ";
			
				//cout << "@@@@@@@@@@@@@@@2";
			
				it = find(bindElemPtr);

				//cout << "#";

				obj = *it;

				// checking if the element has function attached to it
				if (obj->strFunction != "" ) {
					cout << "\n\n+++++++++++++CALCULATING FUNCTION!+++++++++++\nVALUE BEFORE: " << obj->value;
					// assigning the value returned from the function 
					b = operatorMapping->operate(bindElemPtr->strFunction, obj->value);
					cout << "\nVALUE AFTER: " << b << "\n\n";
				}
				else
					// no function
					b = obj->value;						// Getting numeric value
			
				del(it);							// Deleting pointer from list
				obj.reset();						// Deleting ownership of EssentialElement

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
			 * Ownerships of EssentialElement are fully removed, so destructor's gonna be called
			 * No memory leaks. (I think so)
			 */


			} while (opBindClassPtr->next());	
		} // if (!opBindClassPtr->empty())
		cout << endl;

	} while (operatorBindingClasses->next());

	operatorBindingClasses.reset(); 		// Disposing OperatorBindingClasses instance


	cout << "\n\n========================================\n\n";

	cout << "Value EQ NO" << this->id << ": " << this->value << "\n\n";

	SharedPtr<EssentialElement> ptrElem = make_shared<EssentialElement>();
	ptrElem->value = this->value;
	ptrElem->strOperator = this->strOperator;
	ptrElem->strFunction = this->strFunction;

	cout << "F: " << ptrElem->strFunction << "\n";

	return ptrElem;

}
