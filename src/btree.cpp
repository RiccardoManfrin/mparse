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
	left=NULL;
	right=NULL;
}

btree_item_t::~btree_item_t(){
	if(left) left->~btree_item_t();
	if(right) right->~btree_item_t();
	delete this;
}

//***************************************************************************************************//

btree_t::btree_t(){
	root = NULL;
}

btree_t::~btree_t(){
	destroy(root);
	delete this;
}

void btree_t::destroy(btree_item_t* item){
	if(root) delete root;
	destroy(root->left);
	destroy(root->right);
}

void btree_t::print(){
	print(root);
}

void btree_t::print(btree_item_t* item){
	if(item) item->print();
	if(item->left) print(item->left);
	if(item->right) print(item->right);
}