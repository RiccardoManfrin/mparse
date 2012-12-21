/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Linked list data structure
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "exception.h"

///Swap list elements of indexes a, b [indexes start from 0]
static int list_swap(list *l, int a, int b){
	unsigned int max;
	if(a<b) max = b;
	else if(a>b) max = a;
	else return 0;
	if ( max<list_count(l) )
	{
		void *obj;
		elem *el_a=l->el;
		elem *el_b=l->el;

		///Setting pointers
		if( a==0 ) //&& b>0
		{
			while( b>0 )
			{
				el_b = el_b->next;
				b--;
			}
		}
		else if( b==0 ) //&& a>0
		{
			while(a>0)
			{
				el_a = el_a->next;
				a--;
			}
		}
		else //Either a and b are > 0
		{
			while( a>0 )
			{
				el_a = el_a->next;
				a--;
			}
			while( b>0 )
			{
				el_b = el_b->next;
				b--;
			}
		}
		obj = el_b->object;
		el_b->object = el_a->object;
		el_a->object = obj;
		return 0;
			
	}
	return -1;
}

//***************************************************************************************************//

list* list_new(){
	list* lst = malloc(sizeof(list));
	lst->el = NULL;
	return lst;
}

void list_free(list* l, void(*spec_obj_free)(void *)){
	if(l!=NULL){
		elem* e = l->el;
		elem* tmp;
		while(e!=NULL)
		{
			tmp = e;
			e = e->next;
			if(tmp->object!=NULL && spec_obj_free!=NULL) (*spec_obj_free)(tmp->object);
			free(tmp);
		}
		free(l);
	}
	return;
}

unsigned int list_count(list *l){
	int i=0;
	if(l!=NULL)
	{
		elem *el = l->el;
		while (el!=NULL){
			i++;
			el=el->next;
		}
	}
	return i;
}

void list_print(list *lst, void(*spec_obj_print)(void *)){
	if(lst!=NULL)
	{
		if(lst->el != NULL)
		{
			elem *elm = lst->el;
			while(elm!=NULL)
			{
        if (spec_obj_print != NULL)
        {
				  (*spec_obj_print)(elm->object);
				  elm = elm->next;
        }
        else
        {
#ifdef EXCEPTION
          excp(MISSING_PRINTING_FUNCTION);
          return;
#endif
        }
			}
			printf("\n");
		}
		else
		{
#ifdef EXCEPTION
		excp(NOT_INITIALIZED);
#endif
		}
	}
	return;
}

void list_push(list* lst, void* obj, unsigned int size){
	if(lst==NULL)
	{
#ifdef EXCEPTION
		excp(NOT_INITIALIZED);
#endif
		return;
	}
	if(obj!=NULL)
	{
		elem *new_el = malloc(sizeof(elem));
		if(size==0)
		{
			new_el->object= obj;
		}
		else
		{
			new_el->object= malloc(size);
			memcpy(new_el->object, obj, size);
		}
		new_el->next=lst->el;
		lst->el=new_el;
	}
	else
	{
#ifdef EXCEPTION
		excp(NULL_EXCEPTION);
#endif
	}
	return;
}

void *list_pop_index(list* lst, unsigned int index){
	if(index<list_count(lst) && lst != NULL)
	{
		elem *e = lst->el;
		void *objptr=NULL;
		if(index==0)
		{
			e = e->next;
			objptr= lst->el->object;
			free(lst->el);
			lst->el = e;
			return objptr;
		}
		while(index>1)
		{
			e = e->next;
			index--;
		}
		elem *to_delete = e->next;
		e->next = e->next->next;
		objptr= to_delete->object;
		free(to_delete);
		return objptr;
	}
	return NULL;
}

void *list_find(list* lst, compare_result (*compare) (void *, void *),void * comparison_ref){
	if(list_count(lst)>0 && lst != NULL)
	{
		char found = 0;
		compare_result res;
		void *comparison_term = comparison_ref;
		elem *e = lst->el;
		if (e == NULL) goto fail;
		while (e!=NULL)
		{
			res = compare(e->object, comparison_term);
			if( res == FOUND) return e->object;
			else if ( res == SEARCHAGAIN )
			{
				found = 1;
				comparison_term = e->object;
			}
			e=e->next;
		}
		if(found) return comparison_term;
	}
fail:
	return NULL;
}

void *list_pop(list* lst, compare_result (*compare) (void *, void *),void * comparison_ref){
	if(list_count(lst)>0 && lst != NULL)
	{
		char found = 0; char foundfirst = 0;
		void *comparison_term = comparison_ref;
		elem *prev=NULL, *curr=lst->el, *prev_placeholder=NULL, *curr_placeholder=NULL;
		if (curr == NULL) goto fail;
		compare_result res = compare(curr->object, comparison_term);
		if( res == FOUND)
		{
			void *obj = curr->object;
			lst->el = lst->el->next;
			free(curr);
			return obj;
		}
		else if (res == SEARCHAGAIN)
		{
			found = 1;
			foundfirst = 1;
			prev_placeholder = lst->el;
			curr_placeholder = lst->el;
			comparison_term = curr->object;
		}
		while (curr->next!=NULL)
		{
			prev = curr; 
			curr = curr->next;
			res = compare(curr->object, comparison_term);
			if( res == FOUND)
			{
				prev->next = curr->next;
				void *obj = curr->object;
				free(curr);
				return obj;
			}
			else if (res == SEARCHAGAIN)
			{
				found = 1;
				prev_placeholder = prev;
				curr_placeholder = curr;
				comparison_term = curr->object;
			}
		}
		if(found && !foundfirst) 
		{
			prev_placeholder->next = curr_placeholder->next;
			free(curr_placeholder);
			return comparison_term;
		}
		else if(found && foundfirst)
		{
			prev_placeholder = curr_placeholder->next;
			free(curr_placeholder);
			return comparison_term;
		}
	}
fail:
	return NULL;
}

list* list_reverse(list * lst){
	
	int last = list_count(lst)-1;
	
	int first = 0;
	while(first<last){
		list_swap(lst, first, last);
		last--;first++;
	}
	return lst;
}

