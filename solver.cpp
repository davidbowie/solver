/*
EXP-SOLVER - program calculating math expressions

solver.cpp -  main program 

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

#include "solverTempl.h"
#include "solverLib.h"
#include <iostream>
#include <math.h>

using namespace std;

// Elementary arithmetic functions

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

// Create list of operators hierarchy
void createOperatorsList(OperatorClasses*& list)
{
	list = &OperatorClasses::Instance();					// Create operator classes list

	list->add(make_shared<OperatorClass>(0));				// Creating operator classes
	list->add(make_shared<OperatorClass>(1));
	list->add(make_shared<OperatorClass>(2));
	list->first();											// Iterator init
	// Class 0: ^
	list->getCurrent()->add(make_shared<Operator>("^"));	// Creating operators
	list->getCurrent()->updateList();
	list->next();
	// Class 1: * /
	list->getCurrent()->add(make_shared<Operator>("*"));
	list->getCurrent()->add(make_shared<Operator>("/"));
	list->getCurrent()->updateList();
	list->next();
	// Class 2: + - 
	list->getCurrent()->add(make_shared<Operator>("+"));
	list->getCurrent()->add(make_shared<Operator>("-"));
	list->getCurrent()->updateList();
	list->updateList();
}

// Map function with its string representation
void mapOperations(Mapping*& map) {

	map = &Mapping::Instance();

	// Elementary functions
	map->add("+", 	add);
	map->add("-", 	subtract);
	map->add("*",	multiply);
	map->add("/",	divide);
	// math.h functions
	map->add("^",	pow);
	map->add("ln",	log);
	map->add("log",	log10);
	map->add("sin",	sin);
	map->add("cos",	cos);
	map->add("sqrt",sqrt);
	map->add("abs",	fabs);

}

int main(int argc, char* argv[])
{
	Numeric value;				// Final expression value
	ReturnCode code;			// Error code

    Mapping* map;				// Mapping functions and operators
	mapOperations(map);
	
	OperatorClasses* list;
	createOperatorsList(list);	// Creating operators hierarchy
	
	string str;

	//Reading from stdin
	cin >> str;

	// Main expression
	Expression exp(str, list, map);

	// Checking syntax
	code = exp.parse();

	if (code == OK) {								// Checking if parsing gone ok
		value = (exp.calculate())->value;			// Expression::calculate returns EssentialElement*
		cout << value << endl;
	}
	else
	cout << "ERROR: " << code << endl;

	return 0;
}
