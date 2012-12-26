/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Binary tree data structure
*/

#include <stdlib.h>
#include <stdio.h>
#include "btree.h"
#include "exception.h"
#include "debug.h"


btree_item_t::btree_item_t(){
	parent=NULL;
	left=NULL;
	right=NULL;
}

//***************************************************************************************************//

btree_t::btree_t(){
	root = NULL;
}

btree_t::~btree_t(){
	destroy(root);
}

void btree_t::destroy(btree_item_t* item){
	if(item) {
		if(item->left) destroy(item->left);
		if(item->right) destroy(item->right);
		delete item;
	}
}

void btree_t::print(){
	print(root);
}

void btree_t::print(btree_item_t* item){
	if(item) item->print();
	if(item->left) print(item->left);
	if(item->right) print(item->right);
}