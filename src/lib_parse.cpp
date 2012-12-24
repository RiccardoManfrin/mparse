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


//************************************** LOCAL DEFINES AND MACROS ***********************************//


//************************************ STATIC VARIABLES DECLARARTION ********************************//


//************************************ STATIC FUNCTIONS DECLARATION *********************************//


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
static list_t* find_extern_brakets(parser_btree_item_t *node, bool *found_exception);

/**
 * INPUT Arguments:
 * brackets : list of the most extern brackets pairs
 * expr : mathematical expression matching the list of 
 * brackets
 * found_op : empty variable to store the operation 
 * processed by the function
 * found_exception : empty bool to store the result
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
static list_t* find_extern_highest_prio_op(list_t *brackets, char *expr, op_id_t *found_op, bool *found_exception);


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
static void parser_assign_var(parser_btree_item_t *btnode, list_t *vars);

/**
 * Recursive binary tree solver
 */
static parser_val_t parser_bt_calc(parser_btree_item_t *bt, list_t *vars);

/**
 * Takes a node and generate the subtree based on the out of
 * brackets highest priority operations found.
 */
static parser_btree_item_t *generate_subtree(parser_btree_item_t *bt, list_t *tk_indexes, op_id_t optor);
 
/**
 * This is the function that process the single node
 * of the binary tree. The function is dispatched by
 * the binary tree dispatcher to all of its sub nodes,
 * through calling the btree_dispatcher function.
 */
static parser_btree_item_t * expand_tree(list_t *userfunctions, parser_btree_item_t *node);

/**
 * Replace function arguments with argument fom real instance of the function
 */
static char * parser_func_replace(char *func_instance, list_t *userfunctions);

//************************************* STATIC FUNCTIONS DEFINITION *********************************//

static void parser_var_free(void *var_def){
		if(var_def)
		{
			if(((parser_var_def_t*)var_def)->name) free(((parser_var_def_t*)var_def)->name);
			free(var_def);
		}
		return;
}

static void parser_func_free(void *func_def){
		if(func_def)
		{
			if(((parser_func_def_t*)func_def)->name) free(((parser_func_def_t*)func_def)->name);
			if(((parser_func_def_t*)func_def)->def) free(((parser_func_def_t*)func_def)->def);
			free(func_def);
		}
		return;
}

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
			parser_idx_list_item_t *idx = new parser_idx_list_item_t();
			idx->idx = i;
			dbg_print(6, "Tocken found\n");
			result->push(idx);
			found = 1;
		}
		i++;
	}
	if( found == 1 ) return result;
	return NULL;
}

static list_t* find_extern_brakets(parser_btree_item_t *node, bool *found_exception){
	char *expr = node->expr;
	*found_exception = false;
	//[Check first is "("] && [Check last is ")"] && [Check "(" number == ")" number]
	//is equivalent to check that the counter never becomes negative and is left to zero at the end.
	bool first = false, last = false, nomiddle=true;
	int i = 0, prev_i=0, l=0;
	list_t *matched = new list_t();
	while(expr[l]){
		prev_i = i;
		if (expr[l] == '(')	{i++;}
		if (expr[l] == ')')	{i--;}
		if(i==1 && prev_i == 0){/*printf("found \"(\" @%i\n",l);*/matched->push(new parser_idx_list_item_t(l)); if(l==0) first=true;}
		if(i==0 && prev_i == 1){/*printf("found \")\" @%i\n",l);*/matched->push(new parser_idx_list_item_t(l)); if(!expr[l+1]) last=true; else nomiddle=false;}
		else if(i<0) { *found_exception = true; delete matched; return NULL; }
		l++;
	}
	if(i!=0) { *found_exception = true; delete matched; return NULL; }
	if(first==true && last==true && nomiddle==true)
	{ 
		delete matched;
// 		printf("expr: \"%s\"\n",expr);
		char *newexpr = (char *)malloc(strlen(expr)-1);
		memset(newexpr,0, strlen(expr)-1);
		memcpy(newexpr,&expr[1], strlen(expr)-2);
// 		printf("newexpr: \"%s\"\n",newexpr);
 		free(node->expr);
		node->expr = newexpr;
		return find_extern_brakets(node,found_exception);
	}
	if(matched->head == NULL) return NULL;
	matched->reverse();
	return matched;
}

static list_t *find_extern_highest_prio_op(list_t *brackets, char *expr, op_id_t* found_op, bool *found_exception){
	/** 
	* HERE WE MUST USE A WHILE TO CHECK THE HIGHEST AVAILABLE OPERATOR 
	* AND WE MUST DISTINGUISH BETWEEN 20 AND 10 OPERATORS
	*/
	int i = 0;
	*found_op = operation[i].id;
	char *to_find;
	list_t *found = new list_t();
	*found_exception = true;
new_op_check:
	while(i<NOTOP)
	{
		to_find = operation[i].op;
		find(to_find,expr,found);
		if(!found->count())
		{
			i++;
			*found_op = operation[i].id;
			goto new_op_check;
		}
		else
		{
			///Check on missing operator arguments
			if( ((parser_idx_list_item_t *)found->head)->idx == strlen(expr)-1 )
			{
				excp(MISSING_OP_ARGUMENT);
				*found_exception = true;
				delete found;
				return NULL;
			}
			found->reverse();

			if(brackets!=NULL)
			{
				parser_idx_list_item_t *itr = (parser_idx_list_item_t *)found->head;
				int index = 0;
				parser_idx_list_item_t *l = brackets->head;
				parser_idx_list_item_t *r = l->next;
				while(itr!=NULL)
				{
					//The token is enclosed in brackets
					if(itr->idx > l->idx && itr->idx < r->idx)
					{
						itr = itr->next;
						//No need to increase the index as we pop an element
						delete found->pop(index);
						if (found->count()==0)
						{
							i++;
							*found_op = operation[i].id;
							goto new_op_check;
						}
					}
					//The token is after the brackets
					else if(itr->idx > r->idx)
					{
						if(r->next == NULL)
						{
							*found_exception = false;
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
			*found_exception = false;
			return found;
		}
		i++;
	}
	//If we land here it means that something went wrong, or there's a variable or user
	//function to take care of
	*found_exception = false;
	delete found;
	return NULL;
}

parser_btree_item_t * fill_operator_node(op_id_t optor){
	
	parser_btree_item_t *data=new parser_btree_item_t();
	//***************************//
	data->expr = malloc(strlen(operation[optor].op)+1);
	memset(  data->expr, 0, strlen(operation[optor].op)+1  );
	memcpy(  data->expr, operation[optor].op,  strlen(operation[optor].op)  );
	data->st = CALCULATE;
	data->op = optor;
	//***************************//
	dbg_print(8, "operator data : %s\n", data->expr);
	return data;
}

parser_btree_item_t * fill_operand_node(char * expr){
	
	parser_btree_item_t *data= new parser_btree_item_t();
	//***************************//
	data->expr = expr;
	data->st = EXPAND;
	data->op = NOTOP;
	//***************************//
	dbg_print(8, "operand data : %s\n", data->expr);
	return data;
}

static parser_btree_item_t *generate_subtree(parser_btree_item_t *rootbt, list_t *tk_indexes, op_id_t optor){
	char *expr = ((parser_btree_item_t *)rootbt)->expr;
	int operations = tk_indexes->count();
	
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
		if (tk_indexes != NULL && tk_indexes->head != NULL)
		{
			int oplen = strlen(operation[optor].op);
			tokens = new list_t();
			char *token;
			parser_idx_list_item_t *ptr = tk_indexes->head;
			int start = - 1;
			int end = ptr->idx;
			int i, l=0;
			while(ptr!=NULL)
			{
				if(end-start>0)
				{
					//We allocate an extra character 
					//to end the token with the char "\n" == 0;
					token = (char *)malloc(end - start);
					l = 0;
					for(i=start+1;i<end;i++)
					{
						token[l] = expr[i];
						l++;
					}
					token[end-start-1]=0;
					tokens->push(new parser_string_list_item_t(token));
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
				end = ptr->idx;
			}
			if (lngth(expr)-1>end){
				start = end+oplen;
				end = lngth(expr)-1;
				//We allocate an extra character 
				//to end the token with the char "\0" == 0;
				token = (char *) malloc(end - start + 2);
				l = 0;
				for(i=start;i<=end;i++)
				{
					token[l] = expr[i];
					l++;
				}
				token[end-start+1]=0;
				tokens->push(new parser_string_list_item_t(token));
			}
			tokens->reverse();
		}
		else
		{
			tokens = NULL;
		}
		/** * END FUNCTION trim_expr ***/
		delete rootbt; //Freing btree and object (not expression)
		
		parser_btree_item_t **bt = malloc(sizeof(parser_btree_item_t*)*(n_internal_nodes+n_external_nodes));
		while(count < (n_internal_nodes+n_external_nodes))
		{
			bt[count] = new parser_btree_item_t();
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
	rootbt->st=CALCULATE;
	return rootbt;
}


static parser_btree_item_t * expand_tree(list_t *userfunctions, parser_btree_item_t *node){
	dbg_print(8, "_____________new expand_tree______________\n");
	if ( node->st == EXPAND )
	{
		///Phase 1 : finding external brackets
		bool found_exception;
		list_t *brackets = find_extern_brakets(node, &found_exception);
// 		printf(">>>>>> extern brackets: ");
// 		list_print(brackets,print_int);
// 		printf("\n");
		///Phase 2 : finding highest priority out of brackets operators
		list_t *hpob_op;
		op_id_t found_op;
		
		if(!found_exception)
		{
			//highest priority outside brackets operators list
			hpob_op = find_extern_highest_prio_op(
				brackets, 
				node->expr, 
				&found_op,
				&found_exception);
		}
	
		///Phase 3 : composing the tree of operations
		if(!found_exception)
		{
			if(hpob_op)
			{
				node = generate_subtree(node, hpob_op, found_op);
				delete hpob_op;
			}
			else {
				//Checking brackets existence (if so it's a user function)
				if(brackets){
					delete brackets;
					//Handling user defined functions:
					char *c = parser_func_replace(node->expr,userfunctions);
					free(node->expr);
					node->expr = c;
					return expand_tree(userfunctions, node);
				}
				else
				{
					//In this case it can be either a function or a number: therefore it's not an OP
					node->op =NOTOP;
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
		if( (node->left) && (((parser_btree_item_t*)(node->left))->expr) )
		{
			dbg_print(7,"Expanding left node expr: \"%s\"\n", ((parser_btree_item_t*)(node->left))->expr);
			node->left = expand_tree(userfunctions, node->left);
		}
		if( (node->right) && (((parser_btree_item_t*)(node->right))->expr) )
		{
			dbg_print(7,"Expanding right node expr: \"%s\"\n", ((parser_btree_item_t*)(node->right))->expr);
			node->right = expand_tree(userfunctions, node->right);
		}
	}
	
	return node;
}

static void parser_assign_var(parser_btree_item_t *btnode, list_t *vars){
	parser_var_def_t *l = vars->head;
	while(l){
		if(memcmp( l->name, btnode->expr, strlen(btnode->expr)) == 0)
		{
			btnode->val = l->val;
			break;
		}
		l = l->next;
	}
	return;
}

static char * parser_func_replace(char *func_instance, list_t *userfunctions){
	parser_func_def_t *fdef = NULL;
	//Finding function name
	parser_func_def_t *func = userfunctions->head;
	while(func){
		char *funcname =func->name;
		unsigned int fneidx = 0;
		while(funcname[fneidx]!='(') {fneidx++;}
		if(memcmp(func_instance,funcname,fneidx)==0)
		{
			fdef=func;
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
	// parser_func_def_t:
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
			bool found=false, beginOk=false;
			int i;
			
			if(offsetcurr==0) {beginOk=true;}//beginning of expression
			else if(*op=='(')//beginning of subexpression
			{
				beginOk=true;
				op++;
				ptrtarget[offsettarget] = ptrcurr[offsetcurr];
				offsettarget++;
				offsetcurr++;
			}
			else if( memcmp(&ptrcurr[offsetcurr],arg,varlen+1) == 0 )
			{
				beginOk=true;
			}
			else 
			{
				for(i=0;i<NOTOP;i++)
				{
					if(memcmp(op,operation[i].op,strlen(operation[i].op))==0) //Beginning of an operator
					{
						beginOk=true;
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
					if(*op==0) {found=true;}
					else if(*op==')'){found=true;}
					else
					{
						
						for(i=0;i<NOTOP;i++)
						{
							if(memcmp(op,operation[i].op,strlen(operation[i].op))==0){found=true;}
						}
					}
					if(found==true)
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

static parser_val_t parser_bt_calc(parser_btree_item_t *bt, list_t *vars){
	parser_btree_item_t *btnode = bt;
	if(btnode->st == CALCULATE)
	{
		op_id_t id = btnode->op;
		switch(id)
		{
			case(NOTOP):
			{
				char *expr = btnode->expr;
				int i;
				bool isvar=false;
				//here  we can play smart with ascii encoding!!!
				for(i=0;i<strlen(expr);i++)
				{
					if(expr[i] > 'A') isvar=true;
				}
				if(isvar==false){
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
				btnode->val = parser_bt_calc((parser_btree_item_t *)bt->left, vars) + parser_bt_calc((parser_btree_item_t *)bt->right, vars);
				break;
			case(SUB):
				btnode->val = parser_bt_calc((parser_btree_item_t *)bt->left, vars) - parser_bt_calc((parser_btree_item_t *)bt->right, vars);
				break;
			case(EXP):
				btnode->val = pow(parser_bt_calc((parser_btree_item_t *)bt->left, vars),parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(MULT):
				btnode->val = parser_bt_calc((parser_btree_item_t *)bt->left, vars) * parser_bt_calc((parser_btree_item_t *)bt->right, vars);
				break;
			case(DIV):
				btnode->val = parser_bt_calc((parser_btree_item_t *)bt->left, vars) / parser_bt_calc((parser_btree_item_t *)bt->right, vars);
				break;
			case(MOD):
				btnode->val = (int) parser_bt_calc((parser_btree_item_t *)bt->left, vars) % (int) parser_bt_calc((parser_btree_item_t *)bt->right, vars);
				break;
			case(COS):
				btnode->val = cos(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(SIN):
				btnode->val = sin(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(ATAN):
				btnode->val = atan(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(TAN):
				btnode->val = tan(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(NEP):
				btnode->val = exp(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(LOGN):
				btnode->val = log(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(LOG2):
				btnode->val = log2(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
				break;
			case(LOG10):
				btnode->val = log10(parser_bt_calc((parser_btree_item_t *)bt->right, vars));
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

static void parser_clean(parser_btree_item_t *root){
	if(root) root->st=CALCULATE;
	if(root->left)parser_clean(root->left);
	if(root->right)parser_clean(root->right);
	return;
}

//***************************************************************************************************//

int parser_func(parser_t *p, char *function, char *replacement_expr){
	p->data->toupdate = true;
	//TODO Check if function is already defined
	//Else create new one
	parser_func_def_t *newfunc = new parser_func_def_t();
	newfunc->name = malloc(strlen(function));
	newfunc->def = malloc(strlen(replacement_expr));
	memcpy(newfunc->name, function, strlen(function));
	memcpy(newfunc->def, replacement_expr, strlen(replacement_expr));
	p->lfunctions->push(newfunc);
	return 0;
}

int parser_var(parser_t *p, char *var, parser_val_t value){
	// Check if variable is already defined
	parser_var_def_t *l = p->vars->head;
	int varlen=strlen(var);
	while(l){
		if(strlen(l->name) == varlen)
		{
			if(memcmp( l->name, var, varlen) == 0)
			{
				l->val = value;
				return 0;
			}
		}
		l = l->next;
	}
	//Else create new one
	parser_var_def_t *newvar = new parser_var_def_t();
	newvar->name = (char *) malloc(strlen(var)+1);
	memcpy(newvar->name, var, strlen(var));
	newvar->name[strlen(var)]=0;
	newvar->val = value;
	p->vars->push(newvar);
	return 0;
}

parser_t * parser_new(){
	//Common intialization stuff can be put here [DONE ONCE AND FOREVER]
	parser_init();
	
	parser_data_t *data =new parser_data_t();
	
	//Root generation [DONE ONCE AND FOREVER]
	parser_btree_item_t *p =  new parser_btree_item_t();
	//Filling the root with the expression [DONE ONCE AND FOREVER]
	btree_t *bt = new btree_t();
	bt->root = p;

	parser_t *newparser = malloc(sizeof(parser_t));
	newparser->vars = new list_t();
	newparser->lfunctions = new list_t();
	newparser->pstruct = bt;
	newparser->data = data;
	
	return newparser;
}

void parser_expand(parser_t *p, char *expr){
	btree_t *bt = p->pstruct;
	//Explode btree
	if ( ((parser_btree_item_t *) bt->root)->expr==NULL )
	{
		((parser_btree_item_t *) bt->root)->expr = (char *)malloc(strlen(expr)+1);
	}
	else
	{
		delete bt;
		bt = new btree_t();
		parser_btree_item_t *pn =  new parser_btree_item_t();
		pn->st = EXPAND;
		pn->expr =(char *) malloc(strlen(expr)+1);
		//Filling the root with the expression [DONE ONCE AND FOREVER]
		bt->root=pn;
	}
	memcpy(((parser_btree_item_t* )bt->root)->expr, expr,strlen(expr)+1);//Avoid free issues.
	p->pstruct->root = expand_tree(p->lfunctions, (parser_btree_item_t* )(bt->root));
	return;
}

parser_val_t parser_config(parser_t *p,char *expr){
	//We can do better than plain clean by
	//updating only branches with modified variable's
	//values; for we use parser_clean
	if( p->data->toupdate == true )
	{
		if( p->data->expr != NULL )
		{
			free( p->data->expr );
		}
		p->data->expr = (char *) malloc(strlen(expr)+1);
	}
	else if(p->data->expr == NULL)
	{
		p->data->expr = (char *) malloc(strlen(expr)+1);
		p->data->toupdate = true;
	}
	else if (strlen(p->data->expr)!= strlen(expr))
	{
		free( p->data->expr );
		p->data->expr = (char *) malloc(strlen(expr)+1);
		p->data->toupdate = true;
	}
	else if(memcmp(p->data->expr, expr, strlen(expr))!=0)
	{
		p->data->toupdate = true;
	}
	
	if ( p->data->toupdate == true )
	{	
		memcpy( p->data->expr, expr, strlen(expr)+1 );
		p->data->toupdate = false;
		parser_expand(p,expr);
	}
}

parser_val_t parser_calc(parser_t *p){
	parser_clean((parser_btree_item_t *) p->pstruct->root);
	return parser_bt_calc((parser_btree_item_t *) p->pstruct->root, p->vars);
}

void parser_destroy(parser_t *p){
	if(p->vars) delete p->vars; //parser_var_free needs to be invoked on items
	if(p->lfunctions) delete p->lfunctions;//parser_func_free needs to be invoked on items
	if(p->pstruct) delete p->pstruct;
	if(p) free(p); p=NULL;
	return;
}
