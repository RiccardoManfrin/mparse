#include "list.h"

list_item_t::list_item_t()
{
	next = NULL;
}

list_t::list_t()
{
	head = NULL;
}

list_t::~list_t()
{
	list_item_t * curr;
	while(head){
		curr=head->next;
		delete head;
		head=curr;
	}
}

void list_t::swap(uint32_t a, uint32_t b)
{
	unsigned int max;
	if		(a<b) 	max = b;
	else if	(a>b) 	max = a;
	else 			return;
	if ( max<count() )
	{
		list_item_t *prev_a=NULL;
		list_item_t *prev_b=NULL;

		///Setting pointers
		if(a>0){
			a--;
			prev_a=head;
		}
		while(a>0){
			a--;
			prev_a=prev_a->next;
		};
		
		if(b>0){
			b--;
			prev_b=head;
		}
		while(b>0){
			b--;
			prev_b=prev_b->next;
		};
		list_item_t *item_a= (prev_a)?(prev_a->next):(head);
		list_item_t *item_b= (prev_b)?(prev_b->next):(head);
		
		(prev_a?prev_a->next:head)=item_b;
		(prev_b?prev_b->next:head)=item_a;
		list_item_t *tmp= item_b->next;
		(prev_a?prev_a->next:head)->next = item_a->next;
		(prev_b?prev_b->next:head)->next = tmp;
	}
	return;
}


void list_t::push(list_item_t* item)
{
	if(item){
		item->next=head;
		head=item;
	}else{
#ifdef EXCEPTION
		excp(NULL_EXCEPTION);
#endif
	}
	return;
}

list_item_t * list_t::pop(uint32_t index)
{
	if(index<count()){
		list_item_t *found=NULL;
		if(index==0){
			found= head;
			head = head->next;
			found->next=NULL;
			return found;
		}
		list_item_t *ptr = head;
		while(index>1){
			ptr = ptr->next;
			index--;
		}
		found = ptr->next;
		ptr->next = ptr->next->next;
		found->next = NULL;
		return found;
	}else{
#ifdef EXCEPTION
		excp(OUT_OF_BOUND_INDEX);
#endif
	}
	return NULL;
}

list_item_t* list_t::pop(list_item_t *comparable)
{
	if(head){
		list_item_t *comparison_term = comparable;
		list_item_t *prev=NULL, *curr=head;
		
		if( comparison_term->compare(curr) )
		{
			head = head->next;
			return curr;
		}
		
		while (curr!=NULL)
		{
			prev = curr; 
			curr = curr->next;
			if( comparison_term->compare(curr) )
			{
				prev->next = curr->next;
				return curr;
			}
		}
	}
	return NULL;
}

uint32_t list_t::count()
{
	int i=0;
	list_item_t *curr = head;
	while (curr){
		i++;
		curr=curr->next;
	}
	return i;
}

list_item_t* list_t::find(list_item_t *comparable)
{
	if(head){
		list_item_t *curr = head;
		while (curr!=NULL)
		{
			if( comparable->compare(curr) ) return curr;
			curr=curr->next;
		}
	}
	return NULL;
}

void list_t::print()
{
	if(head != NULL)
	{
		list_item_t *item = head;
		while(item!=NULL)
		{
			item->print();
		}
		printf("\n");
	}else{
#ifdef EXCEPTION
		excp(NOT_INITIALIZED);
#endif
	}
	return;
}

void list_t::reverse()
{
	//TODO: do it better with pop from head and push onto a new list
	uint32_t last = count()-1;
	
	uint32_t first = 0;
	while(first<last){
		swap(first, last);
		last--;first++;
	}
	return;
}
