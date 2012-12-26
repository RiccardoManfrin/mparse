/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Binary tree data structure
*/

#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>

class btree_t;
class btree_item_t;

class btree_item_t{
	friend class btree_t;
public:
	btree_item_t *parent;
	btree_item_t *left;
	btree_item_t *right;
public:
	btree_item_t();
	virtual void print(){printf("%08lx\n", (uint64_t) this);};
	virtual bool compare(btree_item_t *comparable){return (comparable==this);}
	virtual ~btree_item_t(){}
};

class btree_t{
public:
	btree_item_t *root;
private:
	void destroy(btree_item_t *item);
	void print(btree_item_t *item);
public:
	/** 
	* Creates a new btree, allocates it and returns a pointer to that.
	* All the btree must be initialized through this method, otherwise
	* we don't protect the code from NULL btree pointers.
	*/
	btree_t();

	/**
	* Printing function requires to specify stored object
	* printing functions. Printing is in preorder.
	* e.g.: Consider the following tree structure:
	* 
	*     ____a____
	*  __b__      c
	* d	   e
	* 
	* Sequence printing order follows: a,b,d,e,c
	*/
	void print();

	~btree_t();
};
#endif
