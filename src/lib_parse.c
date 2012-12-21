/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* @brief  Parser methods
* @Note We don't need to implement the search operation, therefore 
* we don't care about keeping the binary tree balanced, we want to go 
* through all of its nodes every time and hence having it balanced
* or not makes no difference.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lib_parse.h"
#include "exception.h"
#include "debug.h"

#include "std_functions.h"

//************************************** LOCAL DEFINES AND MACROS ***********************************//

///Node possible states
typedef enum{
	///Node holds an expression to be parsed
	EXPAND,
	///Node doesn't need further expansion and is to be calculated
	CALCULATE,
	///Node has the numerical result value so we can use it for the above node computation.
	CALCULATED
} state;

///Basic parser structure (embedded in each btree node).
typedef struct{
	void *expr;
	op_id op;
	state st;
	parser_val_t val;
} parser_bt_node;

typedef struct{
	char * name;
	char * def;
} parser_func_def;

typedef struct{
	char * name;
	parser_val_t val;
} parser_var_def;

typedef unsigned char boolean;
#define TRUE 	1
#define FALSE 	0

typedef struct{
	char *expr;
	boolean toupdate;
} parser_data;

//************************************ STATIC VARIABLES DECLARARTION ********************************//


//************************************ STATIC FUNCTIONS DECLARATION *********************************//

/**
 * Parser binary tree node free function
 */
static void parser_bt_node_free(void *pn);

/**
 * Parser variables free function
 */
static void parser_var_free(void *var_def);

/**
 * Initializations of all the static data structures
 */
static void parser_init();

/**
 * Returns the lentgh of a string 
 */
static int lngth(char *c);

/**
 * Allocates the list result to a INTEGER list and 
 * returns all the indexes of the matches found in string
 * tc (to check) for string tf (to find)
 */
static list_t* find(char *tf, char *tc, list_t* result);

/**
 * Matches all the brackets of an algebric expression
 * and returns a pointer to the list of the most
 * external brackets opened and closed (can be more 
 * than a single couple)
 */
static list_t* find_extern_brakets(btree *node, boolean *found_exception);

/**
 * INPUT Arguments:
 * brackets : list of the most extern brackets pairs
 * expr : mathematical expression matching the list of 
 * brackets
 * found_op : empty variable to store the operation 
 * processed by the function
 * found_exception : empty boolean to store the result
 * of the operation.
 * 
 * OUTPUT :
 * A list with the indexes of the highest priority operations
 * found outside the most extern brackets.
 * 
 * Notes: first item to watch for is found_exception, in
 * order to know if we succeeded with the operation.
 * Indeed we can have 3 possible outcomes:
 * 	- Succed and operation found.
 * 	- Failure: different events:
 * 		-  A variable was found
 * 		-  An error occurred in the expression grammar/syntax
 *
 * Note that this function only takes care of math functions.
 * A subsequent function should be used to check on variables.
 */
static list_t* find_extern_highest_prio_op(list_t *brackets, char *expr, op_id *found_op, boolean *found_exception);

/**
 * Helper functions for generating subtree (generate_subtree
 * function).
 */
static parser_bt_node * fill_operator_node(op_id optor);
static parser_bt_node * fill_operand_node(char * expr);

/**
 * Free the complex parser_bt_node structure
 */
static void parser_bt_node_free(void *pn);

/**
 * Free a variable definition
 */
static void parser_var_free(void *var_def);

/**
 * Free a function definition
 */
static void parser_func_free(void *func_def);

/**
 * take btnode variable name and look it up on var list to find and assign its corresponding value
 */
static void parser_assign_var(parser_bt_node *btnode, list_t *vars);

/**
 * Recursive binary tree solver
 */
static parser_val_t parser_bt_calc(btree *bt, list_t *vars);

/**
 * Takes a node and generate the subtree based on the out of
 * brackets highest priority operations found.
 */
static btree *generate_subtree(btree *bt, list_t *tk_indexes, op_id optor);
 
/**
 * This is the function that process the single node
 * of the binary tree. The function is dispatched by
 * the binary tree dispatcher to all of its sub nodes,
 * through calling the btree_dispatcher function.
 */
static btree * expand_tree(list_t *userfunctions, btree *node);

/**
 * Replace function arguments with argument fom real instance of the function
 */
static char * parser_func_replace(char *func_instance, list_t *userfunctions);

//************************************* STATIC FUNCTIONS DEFINITION *********************************//


static void parser_bt_node_free(void *pn) {
	if(pn)
	{
		if( ((parser_bt_node*)pn)->expr ) free( ((parser_bt_node*)pn)->expr );
		free(pn);
	}
	return;
}

static void parser_var_free(void *var_def){
		if(var_def)
		{
			if(((parser_var_def*)var_def)->name) free(((parser_var_def*)var_def)->name);
			free(var_def);
		}
		return;
}

static void parser_func_free(void *func_def){
		if(func_def)
		{
			if(((parser_func_def*)func_def)->name) free(((parser_func_def*)func_def)->name);
			if(((parser_func_def*)func_def)->def) free(((parser_func_def*)func_def)->def);
			free(func_def);
		}
		return;
}

#include "std_functions_expand.h"

static void parser_init(){
	init_func_bind();
	return;
}

static int lngth(char *c){
	int s=0;
	while(c[s]!=0)
	{
		s++;
	}
	return s;
}

static list_t* find(char *tf, char *tc, list_t* result){
	char found = 0;
	int i=0,l;
	while(i<=lngth(tc)-lngth(tf))
	{
		l=0;
		while(tf[l]==tc[i+l])
		{
			l++;
		}
		if(l>=lngth(tf)) 
		{
			dbg_print(6, "Tocken found\n");
			list_push(result, &i, sizeof(int));
			found = 1;
		}
		i++;
	}
	if( found == 1 ) return result;
	return NULL;
}

static list_t* find_extern_brakets(btree *node, boolean *found_exception){
	char *expr = (char*)(((parser_bt_node*)(node->obj))->expr);
	*found_exception = FALSE;
	//[Check first is "("] && [Check last is ")"] && [Check "(" number == ")" number]
	//is equivalent to check that the counter never becomes negative and is left to zero at the end.
	boolean first = FALSE, last = FALSE, nomiddle=TRUE;
	int i = 0, prev_i=0, l=0;
	list_t *matched = list_new();
	while(expr[l]){
		prev_i = i;
		if (expr[l] == '(')	{i++;}
		if (expr[l] == ')')	{i--;}
		if(i==1 && prev_i == 0){/*printf("found \"(\" @%i\n",l);*/list_push(matched, &l, sizeof(int)); if(l==0) first=TRUE;}
		if(i==0 && prev_i == 1){/*printf("found \")\" @%i\n",l);*/list_push(matched, &l, sizeof(int)); if(!expr[l+1]) last=TRUE; else nomiddle=FALSE;}
		else if(i<0) { *found_exception = TRUE; list_free(matched,free); return NULL; }
		l++;
	}
	if(i!=0) { *found_exception = TRUE; list_free(matched,free); return NULL; }
	if(first==TRUE && last==TRUE && nomiddle==TRUE)
	{ 
		list_free(matched, free);
// 		printf("expr: \"%s\"\n",expr);
		char *newexpr = malloc(strlen(expr)-1);
		memset(newexpr,0, strlen(expr)-1);
		memcpy(newexpr,&expr[1], strlen(expr)-2);
// 		printf("newexpr: \"%s\"\n",newexpr);
 		free(((parser_bt_node*)(node->obj))->expr);
		((parser_bt_node*)(node->obj))->expr = newexpr;
		return find_extern_brakets(node,found_exception);
	}
	if(matched->el == NULL) return NULL;
	list_reverse(matched);
	return matched;
}

static list_t *find_extern_highest_prio_op(list_t *brackets, char *expr, op_id* found_op, boolean *found_exception){
	/** 
	* HERE WE MUST USE A WHILE TO CHECK THE HIGHEST AVAILABLE OPERATOR 
	* AND WE MUST DISTINGUISH BETWEEN 20 AND 10 OPERATORS
	*/
	int i = 0;
	*found_op = operation[i].id;
	char *to_find;
	list_t *found = list_new();
	*found_exception = TRUE;
new_op_check:
	while(i<NOTOP)
	{
		to_find = operation[i].op;
		find(to_find,expr,found);
		if(!list_count(found))
		{
			i++;
			*found_op = operation[i].id;
			goto new_op_check;
		}
		else
		{
			///Check on missing operator arguments
			if(*(int*)(found->el->object) == strlen(expr)-1)
			{
				excp(MISSING_OP_ARGUMENT);
				*found_exception = TRUE;
				list_free(found, free);
				return NULL;
			}
			list_reverse(found);

			if(brackets!=NULL)
			{
				elem *itr = found->el;
				int index = 0;
				elem *l = brackets->el;
				elem *r = l->next;
				while(itr!=NULL)
				{
					//The token is enclosed in brackets
					if(*((int*)itr->object) > *((int*)l->object) && *((int*)itr->object) < *((int*)r->object))
					{
						itr = itr->next;
						//No need to increase the index as we pop an element
						int *ptr = list_pop_index(found, index);
						free(ptr);
						if (list_count(found)==0)
						{
							i++;
							*found_op = operation[i].id;
							goto new_op_check;
						}
					}
					//The token is after the brackets
					else if(*((int*)itr->object) > *((int*)r->object))
					{
						if(r->next == NULL)
						{
							*found_exception = FALSE;
							dbg_print(6, "Op is outside brackets\n");
							return found;
						}
						else { l = r->next; r = l->next;}
					}
					//The token is before the brackets (so it's valid and we don't remove it)
					else
					{
						dbg_print(6, "Op is outside brackets\n");
						index++;
						itr = itr->next;
					}
				}
			}
			*found_exception = FALSE;
			return found;
		}
		i++;
	}
	//If we land here it means that something went wrong, or there's a variable or user
	//function to take care of
	*found_exception = FALSE;
	list_free(found, free);
	return NULL;
}

static parser_bt_node * fill_operator_node(op_id optor){
	
	parser_bt_node *data=NULL;
	data=malloc(sizeof(parser_bt_node));
	memset(data, 0, sizeof(parser_bt_node));
	
	//***************************//
	data->expr = malloc(strlen(operation[optor].op)+1);
	memset(data->expr, 0, strlen(operation[optor].op)+1);
	memcpy(  data->expr,operation[optor].op,  strlen(operation[optor].op)  );
	data->st = CALCULATE;
	data->op = optor;
	//***************************//
	dbg_print(8, "operator data : %s\n", data->expr);
	return data;
}

static parser_bt_node * fill_operand_node(char * expr){
	
	parser_bt_node *data=NULL;
	data=malloc(sizeof(parser_bt_node));
	memset(data, 0, sizeof(parser_bt_node));
	//***************************//
	data->expr = expr;
	data->st = EXPAND;
	data->op = NOTOP;
	//***************************//
	dbg_print(8, "operand data : %s\n", data->expr);
	return data;
}

static btree *generate_subtree(btree *rootbt, list_t *tk_indexes, op_id optor){
	char *expr = ((parser_bt_node *)(rootbt->obj))->expr;
	int operations = list_count(tk_indexes);
	
	if(operations>0)
	{	
		int n_internal_nodes = operations;
		int n_external_nodes = n_internal_nodes+1;
		unsigned int count = 0;
		/** ** FUNCTION trim_expr ******************************
		* Creates a pointer to a new list of string tokens
		* by trimming the expr accordingly to the
		* list of indexes.
		*/
		list_t *tokens = NULL;
		if (tk_indexes != NULL && tk_indexes->el != NULL)
		{
			int oplen = strlen(operation[optor].op);
			tokens = list_new();
			char *token;
			elem *ptr = tk_indexes->el;
			int start = - 1;
			int end = *((int*)ptr->object);
			int i, l=0;
			while(ptr!=NULL)
			{
				if(end-start>0)
				{
					//We allocate an extra character 
					//to end the token with the char "\n" == 0;
					token = malloc(end - start);
					l = 0;
					for(i=start+1;i<end;i++)
					{
						token[l] = expr[i];
						l++;
					}
					token[end-start-1]=0;
					list_push(tokens, token, 0);
				}
				else
				{
					//This is a syntax error (it means we found 2 
					//optors with nothing in the middle)
					excp(SYNTAX_ERROR);
					break; 
				}
				if(ptr->next!=NULL)
				{
					ptr = ptr->next;
				}
				else
				{
					break;
				}
				start = end-1+oplen;
				end = *((int*)ptr->object);
			}
			if (lngth(expr)-1>end){
				start = end+oplen;
				end = lngth(expr)-1;
				//We allocate an extra character 
				//to end the token with the char "\0" == 0;
				token = malloc(end - start + 2);
				l = 0;
				for(i=start;i<=end;i++)
				{
					token[l] = expr[i];
					l++;
				}
				token[end-start+1]=0;
				list_push(tokens, token, 0);
			}
			list_reverse(tokens);
		}
		else
		{
			tokens = NULL;
		}
		/** * END FUNCTION trim_expr ***/
		btree_free(rootbt,free); //Freing btree and object (not expression)
		
		btree **bt = malloc(sizeof(btree*)*(n_internal_nodes+n_external_nodes));
		while(count < (n_internal_nodes+n_external_nodes))
		{
			bt[count] = btree_new();
			memset(bt[count],0,sizeof(btree));
			count++;
		}
		count = 0;
		switch(optor)
		{
			case (SUM):
			case (MULT):
			case (SUB):
			case (EXP):
			case (DIV):
			case (MOD):
			case (COS):
			case (SIN):
			case (TAN):
			case (ATAN):
			case (NEP):
			case (LOGN):
			case (LOG2):
			case (LOG10):
			{
				return operation[optor].expand_fptr(&operation[optor], bt, tokens);
			}
			default:
			{
				excp(OP_NOT_RECOGNIZED);
				break;
			}
		}
		rootbt = bt[0];
		return rootbt;
	}
	/**
	 * If we don't have any operations in our expression, we are dealing with a single operand
	 * so we must update the node state to "CALCULATE";
	 */
	dbg_print(7,"Nothing to expand... updating state to CALCULATE\n");
	((parser_bt_node *)(rootbt->obj))->st=CALCULATE;
	return rootbt;
}


static btree * expand_tree(list_t *userfunctions, btree *node){
	dbg_print(8, "_____________new expand_tree______________\n");
	if ( ((parser_bt_node *)node->obj)->st == EXPAND )
	{
		///Phase 1 : finding external brackets
		boolean found_exception;
		list_t *brackets = find_extern_brakets(node, &found_exception);
// 		printf(">>>>>> extern brackets: ");
// 		list_print(brackets,print_int);
// 		printf("\n");
		///Phase 2 : finding highest priority out of brackets operators
		list_t *hpob_op;
		op_id found_op;
		
		if(!found_exception)
		{
			//highest priority outside brackets operators list
			hpob_op = find_extern_highest_prio_op(
				brackets, 
				(char*)(((parser_bt_node*)(node->obj))->expr), 
				&found_op,
				&found_exception);
		}
	
		///Phase 3 : composing the tree of operations
		if(!found_exception)
		{
			if(hpob_op)
			{
				node = generate_subtree(node, hpob_op, found_op);
				list_free(hpob_op,free);
			}
			else {
				//Checking brackets existence (if so it's a user function)
				if(brackets){
					list_free(brackets,free);
					//Handling user defined functions:
					char *c = parser_func_replace((char*)((parser_bt_node *)node->obj)->expr,userfunctions);
					free(((parser_bt_node *)node->obj)->expr);
					((parser_bt_node *)node->obj)->expr = c;
					return expand_tree(userfunctions, node);
				}
				else
				{
					//In this case it can be either a function or a number: therefore it's not an OP
					((parser_bt_node *)node->obj)->op =NOTOP;
				}
			}
			
		}
	}
	else
	{
		/// We didn't find the node to require expanding.
		dbg_print(7, "NOT Expanding\n");
	}
	
	///Phase 5 : Recursive iteration
	if(node)
	{
		if(node->left) if(node->left->obj) if(((parser_bt_node*)(node->left->obj))->expr)
		{
			dbg_print(7,"Expanding left node expr: \"%s\"\n", ((parser_bt_node*)(node->left->obj))->expr);
			node->left = expand_tree(userfunctions, node->left);
		}
		if(node->right) if(node->right->obj) if(((parser_bt_node*)(node->right->obj))->expr)
		{
			dbg_print(7,"Expanding right node expr: \"%s\"\n", ((parser_bt_node*)(node->right->obj))->expr);
			node->right = expand_tree(userfunctions, node->right);
		}
	}
	
	return node;
}

static void parser_assign_var(parser_bt_node *btnode, list_t *vars){
	list_item_t *l = vars->head;
	while(l){
		if(memcmp( ((parser_var_def *)l->object)->name, btnode->expr, strlen(btnode->expr)) == 0)
		{
			btnode->val = ((parser_var_def *)l->object)->val;
			break;
		}
		l = l->next;
	}
	return;
}

static char * parser_func_replace(char *func_instance, list_t *userfunctions){
	parser_func_def *fdef = NULL;
	//Finding function name
	elem *func = userfunctions->el;
	while(func){
		char *funcname =((parser_func_def *)func->object)->name;
		unsigned int fneidx = 0;
		while(funcname[fneidx]!='(') {fneidx++;}
		if(memcmp(func_instance,funcname,fneidx)==0)
		{
			fdef=(parser_func_def *)func->object;
			break;
		}
		func=func->next;
	}

 	//printf("FUNC INSTANCE: %s\nFUNC NAME:%s\nFUNC DEF:%s\n",func_instance, fdef->name, fdef->def);
	char *funcname = fdef->name;
	char *funcdef = fdef->def;
	char buffA[strlen(fdef->def)+strlen(func_instance)];
	char buffB[strlen(fdef->def)+strlen(func_instance)];
	memset(buffA,0, strlen(fdef->def)+strlen(func_instance));
	memset(buffB,0, strlen(fdef->def)+strlen(func_instance));
	memcpy(buffA,funcdef, strlen(funcdef));
	char *ptrcurr =buffA, *ptrtarget =buffB;
	unsigned int varlen = 0;
	unsigned int offset = 1, offset1=1;
	// parser_func_def:
	//                _____offset
	//               |  ___offset+varlen+1
	//               | |  
	// f(x,ciao,come,va) = sin(x)*cos(ciao)
	// |___|____|____|_|
	//
	// istance:
	// f(2,1,4,5) --> sin(2)*cos(1)
	int innerbrackets=-1,innerbrackets1=-1;
	while( offset + varlen + 1 < strlen(funcname) ){
		unsigned int varlen1=0;
		//Finding next function definition argument
		while(	((funcname[offset - 1]!='(') && 
				(funcname[offset - 1]!=',')) ||
				(innerbrackets!=0) )
		{
			offset++;
			if(funcname[offset - 1]=='(') innerbrackets++;
			if(funcname[offset - 1]==')') innerbrackets--;
		}
		while(	((funcname[offset + varlen] != ',') && 
				(funcname[offset + varlen] != ')')) ||
				(innerbrackets!=0) )
		{
			if(funcname[offset + varlen]=='(') innerbrackets++;
			if(funcname[offset + varlen]==')') innerbrackets--;
			varlen++;
		}
		
		char arg[varlen+1];arg[varlen]=0;
		memcpy(arg, funcname+offset, varlen);
		offset+=varlen+1; varlen=0;
		
		//Finding next function instance real argument to replace with
		
		while(	((func_instance[offset1 - 1]!='(') &&
				(func_instance[offset1 - 1]!=',')) ||
				(innerbrackets1!=0) )
		{
			offset1++;
			if(func_instance[offset1 - 1]=='(') innerbrackets1++;
			if(func_instance[offset1 - 1]==')') innerbrackets1--;
		}
		while(	((func_instance[offset1 + varlen1] != ',') &&
				(func_instance[offset1 + varlen1] != ')')) ||
				(innerbrackets1!=0) )
		{
			if(func_instance[offset1 + varlen1]=='(') innerbrackets1++;
			if(func_instance[offset1 + varlen1]==')') innerbrackets1--;
			varlen1++;
		}
		char argreplacement[varlen1+1];argreplacement[varlen1]=0;
		memcpy(argreplacement, &func_instance[offset1], varlen1);
		offset1+=varlen1+1;
		
		//Now find occurrance of arg in funcdef and replace them with argreplacement
		unsigned int offsetcurr=0, offsettarget=0;
		while(offsetcurr<strlen(ptrcurr))
		{
			char * op=&ptrcurr[offsetcurr];
			boolean found=FALSE, beginOk=FALSE;
			int i;
			
			if(offsetcurr==0) {beginOk=TRUE;}//beginning of expression
			else if(*op=='(')//beginning of subexpression
			{
				beginOk=TRUE;
				op++;
				ptrtarget[offsettarget] = ptrcurr[offsetcurr];
				offsettarget++;
				offsetcurr++;
			}
			else if( memcmp(&ptrcurr[offsetcurr],arg,varlen+1) == 0 )
			{
				beginOk=TRUE;
			}
			else 
			{
				for(i=0;i<NOTOP;i++)
				{
					if(memcmp(op,operation[i].op,strlen(operation[i].op))==0) //Beginning of an operator
					{
						beginOk=TRUE;
						op+=strlen(operation[i].op);
						memcpy(&ptrtarget[offsettarget], &ptrcurr[offsetcurr], strlen(operation[i].op));
						offsettarget+=strlen(operation[i].op);
						offsetcurr+=strlen(operation[i].op);
						break;
					}
				}
			}
// 			printf("%s - %s\n",&ptrcurr[offsetcurr],arg);
			if(beginOk)
			{
				if( memcmp(&ptrcurr[offsetcurr],arg,varlen+1) == 0 )
				{
					op=&ptrcurr[offsetcurr+varlen+1];
					//How to distinguish between vars and other function name tockens?
					//Each variable is preceeded/followed by 
					//	A) an operator, 
					//	B) a open/closed bracket,
					//	C) or nothing.
					//Search matches these cryteria.
					if(*op==0) {found=TRUE;}
					else if(*op==')'){found=TRUE;}
					else
					{
						
						for(i=0;i<NOTOP;i++)
						{
							if(memcmp(op,operation[i].op,strlen(operation[i].op))==0){found=TRUE;}
						}
					}
					if(found==TRUE)
					{
						//FOUND variable
						memcpy(&ptrtarget[offsettarget], argreplacement, varlen1);
						offsetcurr+=varlen;
						offsettarget+=varlen1-1;
					}
					else ptrtarget[offsettarget] = ptrcurr[offsetcurr];
				}
				else ptrtarget[offsettarget] = ptrcurr[offsetcurr];
			}
			else ptrtarget[offsettarget] = ptrcurr[offsetcurr];
			offsettarget++;
			offsetcurr++;
		}
		if(ptrcurr==buffA){ ptrtarget=buffA; ptrcurr=buffB; }
		else { ptrtarget=buffB; ptrcurr=buffA;}
	}
	char *res= malloc(strlen(fdef->def)+strlen(func_instance));
	memcpy(res,ptrcurr, strlen(fdef->def)+strlen(func_instance));
	//now ptrcurr stores the translation.
	return res;
}

static parser_val_t parser_bt_calc(btree *bt, list_t *vars){
	parser_bt_node *btnode = (parser_bt_node*)bt->obj;
	if(btnode->st == CALCULATE)
	{
		op_id id = btnode->op;
		switch(id)
		{
			case(NOTOP):
			{
				char *expr = btnode->expr;
				int i;
				boolean isvar=FALSE;
				//here  we can play smart with ascii encoding!!!
				for(i=0;i<strlen(expr);i++)
				{
					if(expr[i] > 'A') isvar=TRUE;
				}
				if(isvar==FALSE){
					//must ben number
					btnode->val = expr2val(btnode->expr);
				}
				else
				{
					parser_assign_var(btnode, vars);
				}
				break;
			}
			case(SUM):
				btnode->val = parser_bt_calc((btree *)bt->left, vars) + parser_bt_calc((btree *)bt->right, vars);
				break;
			case(SUB):
				btnode->val = parser_bt_calc((btree *)bt->left, vars) - parser_bt_calc((btree *)bt->right, vars);
				break;
			case(EXP):
				btnode->val = pow(parser_bt_calc((btree *)bt->left, vars),parser_bt_calc((btree *)bt->right, vars));
				break;
			case(MULT):
				btnode->val = parser_bt_calc((btree *)bt->left, vars) * parser_bt_calc((btree *)bt->right, vars);
				break;
			case(DIV):
				btnode->val = parser_bt_calc((btree *)bt->left, vars) / parser_bt_calc((btree *)bt->right, vars);
				break;
			case(MOD):
				btnode->val = (int) parser_bt_calc((btree *)bt->left, vars) % (int) parser_bt_calc((btree *)bt->right, vars);
				break;
			case(COS):
				btnode->val = cos(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(SIN):
				btnode->val = sin(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(ATAN):
				btnode->val = atan(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(TAN):
				btnode->val = tan(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(NEP):
				btnode->val = exp(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(LOGN):
				btnode->val = log(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(LOG2):
				btnode->val = log2(parser_bt_calc((btree *)bt->right, vars));
				break;
			case(LOG10):
				btnode->val = log10(parser_bt_calc((btree *)bt->right, vars));
				break;
			default:
				goto error;
				break;
		}
		btnode->st = CALCULATED;
	}
	return btnode->val;	
error:
	excp(OP_NOT_RECOGNIZED);
	return (parser_val_t) 0;
}

static void parser_clean(btree *root){
	if(root)((parser_bt_node*)root->obj)->st=CALCULATE;
	if(root->left)parser_clean(root->left);
	if(root->right)parser_clean(root->right);
	return;
}

//***************************************************************************************************//

int parser_func(parser_t *p, char *function, char *replacement_expr){
	((parser_data*)p->data)->toupdate = TRUE;
	//TODO Check if function is already defined
	//Else create new one
	parser_func_def *newfunc = malloc(sizeof(parser_func_def));
	newfunc->name = malloc(strlen(function));
	newfunc->def = malloc(strlen(replacement_expr));
	memcpy(newfunc->name, function, strlen(function));
	memcpy(newfunc->def, replacement_expr, strlen(replacement_expr));
	list_push(p->lfunctions, newfunc, sizeof(parser_func_def));
	return 0;
}

int parser_var(parser_t *p, char *var, parser_val_t value){
	// Check if variable is already defined
	elem *l = p->vars->el;
	int varlen=strlen(var);
	while(l){
		if(strlen(((parser_var_def *)l->object)->name) == varlen)
		{
			if(memcmp( ((parser_var_def *)l->object)->name, var, varlen) == 0)
			{
				((parser_var_def *)l->object)->val = value;
				return 0;
			}
		}
		l = l->next;
	}
	//Else create new one
	parser_var_def *newvar = malloc(sizeof(parser_var_def));
	newvar->name = malloc(strlen(var)+1);
	memcpy(newvar->name, var, strlen(var));
	newvar->name[strlen(var)]=0;
	newvar->val = value;
	list_push(p->vars, newvar, 0);
	return 0;
}

parser_t * parser_new(){
	//Common intialization stuff can be put here [DONE ONCE AND FOREVER]
	parser_init();
	
	parser_data *data =(parser_data*)malloc(sizeof(parser_data));
	data->expr=NULL;
	data->toupdate=FALSE;
	
	//Root generation [DONE ONCE AND FOREVER]
	btree *bt = btree_new();
	parser_bt_node *p = malloc(sizeof(parser_bt_node));
	p->st = EXPAND;
	p->expr = NULL;
	//Filling the root with the expression [DONE ONCE AND FOREVER]
	btree_fill(bt, p);

	parser_t *newparser = malloc(sizeof(parser_t));
	newparser->vars = list_new();
	newparser->lfunctions = list_new();
	newparser->pstruct = bt;
	newparser->data = data;
	
	return newparser;
}

void parser_expand(parser_t *p, char *expr){
	//Explode btree
	if ( ((parser_bt_node *) p->pstruct->obj)->expr==NULL )
	{
		((parser_bt_node *) p->pstruct->obj)->expr = (char *)malloc(strlen(expr)+1);
	}
	else
	{
		btree_free(p->pstruct, parser_bt_node_free);
		p->pstruct = btree_new();
		parser_bt_node *pn = malloc(sizeof(parser_bt_node));
		pn->st = EXPAND;
		pn->expr = malloc(strlen(expr)+1);
		//Filling the root with the expression [DONE ONCE AND FOREVER]
		btree_fill(p->pstruct, pn);
	}
	memcpy(((parser_bt_node *) p->pstruct->obj)->expr,expr,strlen(expr)+1);//Avoid free issues.
	p->pstruct = expand_tree(p->lfunctions, p->pstruct);
	return;
}

parser_val_t parser_calc(parser_t *p,char *expr){
	//We can do better than plain clean by
	//updating only branches with modified variable's
	//values; for we use parser_clean
	if( ((parser_data*)p->data)->toupdate == TRUE )
	{
		if( ((parser_data*)p->data)->expr != NULL )
		{
			free( ((parser_data*)p->data)->expr );
		}
		((parser_data*)p->data)->expr = (char *) malloc(strlen(expr)+1);
	}
	else if(((parser_data*)p->data)->expr == NULL)
	{
		((parser_data*)p->data)->expr = (char *) malloc(strlen(expr)+1);
		((parser_data*)p->data)->toupdate = TRUE;
	}
	else if (strlen(((parser_data*)p->data)->expr)!= strlen(expr))
	{
		free( ((parser_data*)p->data)->expr );
		((parser_data*)p->data)->expr = (char *) malloc(strlen(expr)+1);
		((parser_data*)p->data)->toupdate = TRUE;
	}
	else if(memcmp(((parser_data*)p->data)->expr, expr, strlen(expr))!=0)
	{
		((parser_data*)p->data)->toupdate = TRUE;
	}
	
	if ( ((parser_data*)p->data)->toupdate == TRUE )
	{	
		memcpy( ((parser_data*)p->data)->expr, expr, strlen(expr)+1 );
		((parser_data*)p->data)->toupdate = FALSE;
		parser_expand(p,expr);
	}
	parser_clean(p->pstruct);
	return parser_bt_calc(p->pstruct, p->vars);
}

void parser_destroy(parser_t *p){
	if(p->vars) list_free(p->vars, parser_var_free);
	if(p->lfunctions) list_free(p->lfunctions, parser_func_free);
	if(p->pstruct) btree_free(p->pstruct, parser_bt_node_free);
	if(p) free(p); p=NULL;
	return;
}
