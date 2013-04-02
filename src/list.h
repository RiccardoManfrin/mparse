/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Linked list data structure
*/

#ifndef LIST_H
#define LIST_H
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

class list_item_t{
public:
	list_item_t *next;
public:
	list_item_t();
	virtual void print(){printf("%08lx\n", (uint64_t) this);};
	virtual bool compare(list_item_t *comparable){return (comparable==this);}
};
	
class list_t {
public:
	list_item_t *head;
public:
	class iterator_t{
	private:
		list_item_t *curr;
	public:
		iterator_t(list_t *l);
		bool next();
		list_item_t *get();
	};
private:
	void swap(uint32_t a, uint32_t b);
public:
	list_t();
	uint32_t count();
	void print();
	void push(list_item_t *item);
	list_item_t *pop(uint32_t index);
	list_item_t *pop(list_item_t *comparable);
	list_item_t *find(list_item_t *comparable);
	void reverse();
	~list_t();
};

#endif