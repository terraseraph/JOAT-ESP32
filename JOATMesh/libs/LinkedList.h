/*
	NLinkedList.h - V1.1 - Generic NLinkedList implementation
	Works better with FIFO, because LIFO will need to
	search the entire List to find the last one;

	For instructions, go to https://github.com/ivanseidel/LinkedList

	Created by Ivan Seidel Gomes, March, 2013.
	Released into the public domain.
*/

#ifndef LinkedList_h
#define LinkedList_h

#include <stddef.h>

template <class T>
struct NListNode
{
	T data;
	NListNode<T> *next;
};

template <typename T>
class NLinkedList
{

  protected:
	int _size;
	NListNode<T> *root;
	NListNode<T> *last;

	// Helps "get" method, by saving last position
	NListNode<T> *lastNodeGot;
	int lastIndexGot;
	// isCached should be set to FALSE
	// everytime the list suffer changes
	bool isCached;

	NListNode<T> *getNode(int index);

  public:
	NLinkedList();
	~NLinkedList();

	/*
		Returns current size of NLinkedList
	*/
	virtual int size();
	/*
		Adds a T object in the specified index;
		Unlink and link the NLinkedList correcly;
		Increment _size
	*/
	virtual bool add(int index, T);
	/*
		Adds a T object in the end of the NLinkedList;
		Increment _size;
	*/
	virtual bool add(T);
	/*
		Adds a T object in the start of the NLinkedList;
		Increment _size;
	*/
	virtual bool unshift(T);
	/*
		Set the object at index, with T;
		Increment _size;
	*/
	virtual bool set(int index, T);
	/*
		Remove object at index;
		If index is not reachable, returns false;
		else, decrement _size
	*/
	virtual T remove(int index);
	/*
		Remove last object;
	*/
	virtual T pop();
	/*
		Remove first object;
	*/
	virtual T shift();
	/*
		Get the index'th element on the list;
		Return Element if accessible,
		else, return false;
	*/
	virtual T get(int index);

	/*
		Clear the entire array
	*/
	virtual void clear();
};

// Initialize NLinkedList with false values
template <typename T>
NLinkedList<T>::NLinkedList()
{
	root = NULL;
	last = NULL;
	_size = 0;

	lastNodeGot = root;
	lastIndexGot = 0;
	isCached = false;
}

// Clear Nodes and free Memory
template <typename T>
NLinkedList<T>::~NLinkedList()
{
	NListNode<T> *tmp;
	while (root != NULL)
	{
		tmp = root;
		root = root->next;
		delete tmp;
	}
	last = NULL;
	_size = 0;
	isCached = false;
}

/*
	Actualy "logic" coding
*/

template <typename T>
NListNode<T> *NLinkedList<T>::getNode(int index)
{

	int _pos = 0;
	NListNode<T> *current = root;

	// Check if the node trying to get is
	// immediatly AFTER the previous got one
	if (isCached && lastIndexGot <= index)
	{
		_pos = lastIndexGot;
		current = lastNodeGot;
	}

	while (_pos < index && current)
	{
		current = current->next;

		_pos++;
	}

	// Check if the object index got is the same as the required
	if (_pos == index)
	{
		isCached = true;
		lastIndexGot = index;
		lastNodeGot = current;

		return current;
	}

	return false;
}

template <typename T>
int NLinkedList<T>::size()
{
	return _size;
}

template <typename T>
bool NLinkedList<T>::add(int index, T _t)
{

	if (index >= _size)
		return add(_t);

	if (index == 0)
		return unshift(_t);

	NListNode<T> *tmp = new NListNode<T>(),
				 *_prev = getNode(index - 1);
	tmp->data = _t;
	tmp->next = _prev->next;
	_prev->next = tmp;

	_size++;
	isCached = false;

	return true;
}

template <typename T>
bool NLinkedList<T>::add(T _t)
{

	NListNode<T> *tmp = new NListNode<T>();
	tmp->data = _t;
	tmp->next = NULL;

	if (root)
	{
		// Already have elements inserted
		last->next = tmp;
		last = tmp;
	}
	else
	{
		// First element being inserted
		root = tmp;
		last = tmp;
	}

	_size++;
	isCached = false;

	return true;
}

template <typename T>
bool NLinkedList<T>::unshift(T _t)
{

	if (_size == 0)
		return add(_t);

	NListNode<T> *tmp = new NListNode<T>();
	tmp->next = root;
	tmp->data = _t;
	root = tmp;

	_size++;
	isCached = false;

	return true;
}

template <typename T>
bool NLinkedList<T>::set(int index, T _t)
{
	// Check if index position is in bounds
	if (index < 0 || index >= _size)
		return false;

	getNode(index)->data = _t;
	return true;
}

template <typename T>
T NLinkedList<T>::pop()
{
	if (_size <= 0)
		return T();

	isCached = false;

	if (_size >= 2)
	{
		NListNode<T> *tmp = getNode(_size - 2);
		T ret = tmp->next->data;
		delete (tmp->next);
		tmp->next = NULL;
		last = tmp;
		_size--;
		return ret;
	}
	else
	{
		// Only one element left on the list
		T ret = root->data;
		delete (root);
		root = NULL;
		last = NULL;
		_size = 0;
		return ret;
	}
}

template <typename T>
T NLinkedList<T>::shift()
{
	if (_size <= 0)
		return T();

	if (_size > 1)
	{
		NListNode<T> *_next = root->next;
		T ret = root->data;
		delete (root);
		root = _next;
		_size--;
		isCached = false;

		return ret;
	}
	else
	{
		// Only one left, then pop()
		return pop();
	}
}

template <typename T>
T NLinkedList<T>::remove(int index)
{
	if (index < 0 || index >= _size)
	{
		return T();
	}

	if (index == 0)
		return shift();

	if (index == _size - 1)
	{
		return pop();
	}

	NListNode<T> *tmp = getNode(index - 1);
	NListNode<T> *toDelete = tmp->next;
	T ret = toDelete->data;
	tmp->next = tmp->next->next;
	delete (toDelete);
	_size--;
	isCached = false;
	return ret;
}

template <typename T>
T NLinkedList<T>::get(int index)
{
	NListNode<T> *tmp = getNode(index);

	return (tmp ? tmp->data : T());
}

template <typename T>
void NLinkedList<T>::clear()
{
	while (size() > 0)
		shift();
}

#endif
