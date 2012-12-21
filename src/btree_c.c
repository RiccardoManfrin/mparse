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


typedef enum{
	STRING,
	INT
} print_type;


//***************************************************************************************************//

btree* btree_new(){
	btree* bt= malloc(sizeof(btree));
	bt->obj=NULL;
	bt->left=NULL;
	bt->right=NULL;
	return bt;
}

void btree_dispatcher_post(void(*fptr)(void *) ,btree* bt){
	if(bt != NULL)
	{
		if (bt->left!=NULL) btree_dispatcher_post(fptr, (btree *) bt->left);
		if (bt->right!=NULL) btree_dispatcher_post(fptr, (btree *) bt->right);
		//Having the operation as the last thing helps operations
		//such as free or pointers redefinition.
		(*fptr)(bt);
	}
	return;
}

void btree_dispatcher_pre(void(*fptr)(void *) ,btree* bt){
	if(bt != NULL)
	{
		(*fptr)(bt);
		if (bt->left!=NULL) btree_dispatcher_pre(fptr, (btree *) bt->left);
		if (bt->right!=NULL) btree_dispatcher_pre(fptr, (btree *) bt->right);
	}
	return;
}

void btree_free(btree* bt, void(*spec_obj_free)(void *)){
	if(bt != NULL)
	{
		btree * left = bt->left;
		btree * right = bt->right;
		if(spec_obj_free!=NULL && bt->obj!=NULL) (*spec_obj_free)(bt->obj);
		free(bt);
		dbg_print(6,"Clearing (free) bt node\n");
		btree_free(left,spec_obj_free);
		btree_free(right,spec_obj_free);
	}
	return;
}

void btree_print(btree* bt, void(*spec_obj_print)(void *)){
	if(bt != NULL)
	{
		if(spec_obj_print!=NULL && bt->obj!=NULL) (*spec_obj_print)(bt->obj);
    else return;
		if (bt->left!=NULL) btree_print((btree *) bt->left, spec_obj_print);
		if (bt->right!=NULL) btree_print((btree *) bt->right, spec_obj_print);
	}
	return;
}

void btree_fill(btree* bt, void *obj){
	if(bt!=NULL && obj!=NULL)
	{
		///(Re)Allocating the obj
		if(bt->obj!=NULL) 
		{
			excp(NON_NULL_PTR);
			free(bt->obj);
		}
		bt->obj=obj;
	}
	else
	{
		excp(NULL_EXCEPTION);
	}
	return;
}
