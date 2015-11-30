#include <stdlib.h>
#include <stdio.h>
#include "solverTempl.h"
#include "solverLib.h"
#include <iostream>
#include <sstream>
#include <math.h>
#include <map>
#include <vector>

using namespace std;



Numeric add(Numeric a, Numeric b) {

	return a + b;
}

Numeric subtract(Numeric a, Numeric b) {

	return a - b;
}

Numeric multiply(Numeric a, Numeric b) {

	return a * b;
}

Numeric divide(Numeric a, Numeric b) {

	return a / b;
}







void mapOperations(Mapping*& map) {

	map = &Mapping::Instance();

	map->add("+", 	add);
	map->add("-", 	subtract);
	map->add("*",	multiply);
	map->add("/",	divide);
	map->add("^",	pow);
	map->add("ln",	log);
	map->add("log",	log10);
	map->add("sin",	sin);
	map->add("cos",	cos);
	map->add("sqrt",sqrt);
	map->add("abs",	fabs);



}

void createFunctionsList(Functions*& list)
{
	list = &Functions::Instance();

	list->add(make_shared<Function>("ln"));
	list->add(make_shared<Function>("log"));
	list->add(make_shared<Function>("sin"));
	list->add(make_shared<Function>("cos"));
	list->add(make_shared<Function>("sqrt"));
	list->add(make_shared<Function>("abs"));

}

void createOperatorsList(OperatorClasses*& list)
{
	list = &OperatorClasses::Instance();					// Create operator classes list

	list->add(make_shared<OperatorClass>(0));						// Creating operator classes
	list->add(make_shared<OperatorClass>(1));
	list->add(make_shared<OperatorClass>(2));
	list->first();											// Iterator init
	// Class 0: + -
	list->getCurrent()->add(make_shared<Operator>("^"));				// Creating operators
	list->getCurrent()->updateList();
	list->next();
	// Class 1: * /
	list->getCurrent()->add(make_shared<Operator>("*"));
	list->getCurrent()->add(make_shared<Operator>("/"));
	list->getCurrent()->updateList();
	list->next();
	// Class 2: ^
	list->getCurrent()->add(make_shared<Operator>("+"));
	list->getCurrent()->add(make_shared<Operator>("-"));
	list->getCurrent()->updateList();
	list->updateList();
}

int main(int argc, char* argv[])
{
	Numeric value;
	ReturnCode code;

    Mapping* map;
	mapOperations(map);
	
	OperatorClasses* list;
	createOperatorsList(list);	
	
	string str = "sqrt(((-log(2(1+abs((3+2^(4-3)/5+6-1.2-8/10)-2(2-1)(45/9)-log(10)))+40/50))^3+2)*25)";

	cout << "\n\n===============EQUATION 1.0==================\n\n";
	cout << str << endl;
	Expression exp(str, list, map);

	code = exp.parse();

	if (code == OK) {					// Checking if parsing gone ok
		value = (exp.calculate())->value;	// Expression::calculate returns EssentialElement*
		cout << "\n\nVALUE: " << value << "\n\n";
	}

	cout << "ERROR CODE: " << code << endl;

	return 0;
}
