/*
EXP-SOLVER - program calculating math expressions

solverTempl.h - Template library header file

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

#ifndef SOLVERTEMPL_H
#define SOLVERTEMPL_H

#include <list>
#include <algorithm>
#include <string>
#include <memory>

// Shared pointer type
template <class T>
using SharedPtr = typename std::shared_ptr<T>;

// List consisting shared pointers
template <class T>
using ListType = typename std::list<SharedPtr<T>>;

// Iterator type
template <class T>
using ListIterator = typename ListType<T>::iterator;

/*
 * Class templates
 */
template <class T>
class List {
protected:
	ListType<T> objects;
	ListIterator<T> it;

public:

	void add(SharedPtr<T> ob) {objects.push_back(ob);}	// Adds new object
	void del(ListIterator<T> pos) {objects.erase(pos);}	// Deletes object
	bool empty() {return objects.empty();}				// Indicates if list is empty
	bool next();
	bool prev();
	void first();
	SharedPtr<T> getCurrent();
	
	ListIterator<T> getIterator() {return it;}			// Returns iterator
	
	ListIterator<T> getIteratorLast() {					// Returns iterator pointing at the last element
		ListIterator<T> tmp = objects.end();
		return --tmp;
	}

	ListIterator<T> getIteratorFirst() {return objects.begin();}	// Return iterator pointing at first element

	ListIterator<T> find(SharedPtr<T> value) {
	
		return std::find(objects.begin(), objects.end(), value);	// Finds element
	}
};

/*
 * Class template methods
 */

// Moves iterator forwards
template <class T>
bool List<T>::next() {

	if (++it != objects.end()) {
		return 1;
	}
	else 
		return 0;

}

// Moves iterator backwards
template <class T>
bool List<T>::prev() {

	if (--it != objects.begin()) {
		return 1;
	}
	else 
		return 0;
}

// Moves iterator to the beginning
template <class T>
void List<T>::first(){

	it = objects.begin();
}

// Returns object pointed by iterator
template <class T>
SharedPtr<T> List<T>::getCurrent() {
	return *it;
}

#endif // SOLVERTEMPL_H


