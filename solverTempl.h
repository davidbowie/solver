// solverTempl.h

#ifndef SOLVERTEMPL_H
#define SOLVERTEMPL_H

#include <list>
#include <algorithm>
#include <string>
#include <memory>

template <class T>
using SharedPtr = typename std::shared_ptr<T>;

template <class T>
using ListType = typename std::list<SharedPtr<T>>;

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

	void add(SharedPtr<T> ob) {objects.push_back(ob);}
	void del(ListIterator<T> pos) {objects.erase(pos);}
	bool empty() {return objects.empty();}
	bool next();
	bool prev();
	void first();
	SharedPtr<T> getCurrent();
	
	ListIterator<T> getIterator() {return it;}
	
	ListIterator<T> getIteratorLast() {
		ListIterator<T> tmp = objects.end();
		return --tmp;
	}

	ListIterator<T> getIteratorFirst() {return objects.begin();}

	ListIterator<T> find(SharedPtr<T> value) {
	
		return std::find(objects.begin(), objects.end(), value);
	}
};

template <class T>
class SingletonList : public List<T> {

	SingletonList(){};
	SingletonList(SingletonList const&) = delete;
	SingletonList& operator=(SingletonList const&) = delete;
	
public:
	static SingletonList& Instance();


};


/*
 * Class template methods
 */
template <class T>
bool List<T>::next() {

	if (++it != objects.end()) {
		return 1;
	}
	else 
		return 0;

}

template <class T>
bool List<T>::prev() {

	if (--it != objects.begin()) {
		return 1;
	}
	else 
		return 0;
}

template <class T>
void List<T>::first(){

	it = objects.begin();
}


template <class T>
SharedPtr<T> List<T>::getCurrent() {
	return *it;
}

template <class T>
SingletonList<T>& SingletonList<T>::Instance() {

	static SingletonList<T> instance;
	return instance;
}


#endif // SOLVERTEMPL_H


