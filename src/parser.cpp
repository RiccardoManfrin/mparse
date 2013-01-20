/**
* @file parser.cpp
*
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Parser implementation
*
* @Note We don't need to implement the search operation, therefore 
* we don't care about keeping the binary tree balanced, we want to go 
* through all of its nodes every time and hence having it balanced
* or not makes no difference.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "parser.h"

mparser_t::operator_t mparser_t::operator_t::operators[OPERATORS_NUM];

mparser_t::mparser_t()
{
	uservars= new list_t();
	userfunctions= new list_t();
	tree=NULL;
}
void mparser_t::init()
{
	operator_t::init();
}

void mparser_t::operator_t::init()
{
	unsigned int optors_idx;
	for(optors_idx=0; optors_idx<OPERATORS_NUM; optors_idx++){
		operator_t::operators[optors_idx].id = (op_id_t) optors_idx;
		operator_t::operators[optors_idx].op = optors_str[optors_idx];
		//TODO methods expand.. should belong to operator as private and have a pointer target right one!!!
		operator_t::operators[optors_idx].expand_fptr = operator_t::expand_std_2op_func;
	}
}

mparser_t::~mparser_t()
{
	if(uservars) delete uservars;
	if(userfunctions) delete userfunctions;
	if(tree) delete tree;
}

void mparser_t::variable(char* name, parser_val_t value)
{
	variable_def_t *var = new variable_def_t();
	var->name = name;
	var->val = value;
	variable_def_t * found = (variable_def_t *)uservars->find(var);
	if(found) {
		found->val = value;
		delete var;
	}else{
		uservars->push(var);
	}
}

void mparser_t::function(char* name, char* expanded)
{
	function_def_t * func = new function_def_t();
	func->name=name;
	func->def=expanded;
	function_def_t *found = (function_def_t *)userfunctions->find(func);
	if(found){
		found->def = expanded;
		delete func;
	}else{
		userfunctions->push(func);
	}
}

void mparser_t::expression(char * expr)
{
	if(tree) delete tree; //FREE
	tree = new btree_t(); //MALLOC
	parser_item_t * root= new parser_item_t(this); //MALLOC
	unsigned int len = strlen(expr)+1;
	char *rootexpr=(char *) malloc(len);//MALLOC
	memcpy(rootexpr,expr, len-1);
	rootexpr[len-1]=0;
	root->expr = rootexpr;
	root->st = parser_item_t::EXPAND;
	tree->root = expand(root);
}

parser_item_t* mparser_t::expand(parser_item_t* node)
{
	if(node->st == parser_item_t::EXPAND)
	{
		char *expr = node->expr;
		//Phase 1 : finding external brackets

		//Error check conditions:
		// - Number of opened brackets matches number of closed brackets,
		//   equivalent to ensure that counter final value is zero. 
		// - Number of closed brackets never exceeds number of opened brackets
		//   equivalent to ensure that counter never becomes negative.
		//
		//Counter is incremented upon detecting an opened bracket and decremented
		//when a closed one is found.
		//
		//We also make sure that the only external found brackets are not the most external,
		//as in that case we need to remove the wrapping brackets and rework the 
		//internal expression.
		
		list_t *brackets = NULL;
		while(!brackets){
			bool first = false, last = false, nomiddle=true;
			int i = 0, prev_i=0, l=0;
			brackets=new list_t();
			while(expr[l]){
				prev_i = i;
				if (expr[l] == '(')	{i++;}
				if (expr[l] == ')')	{i--;}
				if(i==1 && prev_i == 0) {
					brackets->push(new parser_list_item_t(&expr[l]));
					if(l==0) first=true;
				}
				if(i==0 && prev_i == 1) {
					brackets->push(new parser_list_item_t(&expr[l]));
					if(!expr[l+1]) last=true; 
					else nomiddle=false;
				}
				else if(i<0) {
					//error: We found more closed brackets than the opened ones.
					delete brackets;
					return NULL;
				}
				l++;
			}
			if(i!=0)
			{
				//error: opened/closed brackets final sum mismatch.
				delete brackets;
				return NULL;
			}
			if(first==true && last==true && nomiddle==true)
			{
				//brackets wrap the whole expression: unwrap and restart looking for brackets
				delete brackets;
				brackets=NULL;
				
				unsigned int unwrapped_len = strlen(expr)-2;
				char *unwrapped_expr = (char *)malloc(unwrapped_len+1);
				unwrapped_expr[unwrapped_len]=0;
				memcpy(unwrapped_expr, &expr[1], unwrapped_len);
				free(node->expr);
				node->expr = unwrapped_expr;
				expr=node->expr;
			}
		}
		if(brackets->count()){
			brackets->reverse(); // Last inserted is the first in the list, so reverse it.
		}else{ 
			delete brackets;
			brackets=NULL;
		}
		
		//Phase 2 : finding Highest Priority Operator Outside External Brackets
		list_t *hpooeb = new list_t();
		unsigned int op_idx;
		unsigned int op_len;
		for (op_idx=0; op_idx <OPERATORS_NUM ; op_idx++){
			char *tf = (char *)operator_t::operators[op_idx].op; //Token to find
			op_len=strlen(tf);
			unsigned int expr_len = strlen(expr);
			unsigned int expr_idx;
			if(expr_len<op_len) continue;
			for(expr_idx=0; expr_idx < (expr_len-op_len); expr_idx++){
				unsigned int tf_idx=0;
				while(tf[tf_idx] ==expr[expr_idx+tf_idx]){
					tf_idx++;
					if(tf_idx == op_len){
						//found!
						hpooeb->push(new parser_list_item_t(&expr[expr_idx]));
						break;
					}
				}
			}
			if(!hpooeb->count()){
				continue;
			}else{
				//Check on missing operand after the operator
				if( ((parser_list_item_t *) hpooeb->head)->str == &expr[expr_len-1] ){
					delete hpooeb;
					return NULL;
				}
				hpooeb->reverse();
				if(brackets){
					parser_list_item_t *l = (parser_list_item_t *)brackets->head;
					parser_list_item_t *r = (parser_list_item_t *)l->next;
					
					while(r){
						parser_list_item_t *itr =  (parser_list_item_t *)hpooeb->head;
						while(itr){
							if(itr->str > l->str && itr->str < r->str) {
								//The operator is within brakets and must be removed
								delete hpooeb->pop(itr);
							}
							itr=(parser_list_item_t *)itr->next;
						}
						l = (parser_list_item_t *)r->next;
						if(!l) break;
						r = (parser_list_item_t *)l->next;
					}
				}//EOF if (found brackets && check op is outside)
				if (hpooeb->count()) break;
			}//EOF else (found operators)
		}//EOF op_idx FOR loop
		
		//Phase 3 : composing the tree of operations
		if (!hpooeb->count()){
			//Loop terminated witout finding a valid operation outside external brackets
			if(brackets){
				delete brackets;
				/* If operator was not found outside brakets, it can mean
				* two things:
				* 1) the brackets are wrapping the whole expression
				* 2) the operator is not defined in std
				* As we already got rid of case 1) (as we always 
				* trim most extern brackets) only two is left;
				*/
				char *c = userfunction_replace(node->expr);
				free(node->expr);
				node->expr = c;
				node = expand(node);
			}else{
				node->op =NOTOP;
				node->st =parser_item_t::CALCULATE;
				//It is either a variable or constant value
				//TODO: distinguish variable and create a bindings to them.
			}
		}else{
			//geerate_subtree
			unsigned int operations = hpooeb->count();
			int n_internal_nodes = operations;
			int n_external_nodes = n_internal_nodes+1;
			
			list_t *tokens = new list_t();
			char *token;
			parser_list_item_t *ptr = (parser_list_item_t *)hpooeb->head;
			char *start = expr;
			char *end = ptr->str;
			unsigned int token_len;
			//Creating list of tokens
			while(ptr!=NULL)
			{
				token_len= end-start+1;
				if(token_len>0)
				{
					//We allocate an extra character 
					//to end the token with the char "\n" == 0;
					token = (char *)malloc(token_len);
					memcpy(token, start,token_len);
					token[token_len-1]=0;
					tokens->push(new parser_list_item_t(token));
				} else {
					//This is a syntax error (it means we found 2 
					//optors with nothing in the middle)
					delete tokens;
					return NULL;
				}
				ptr = (parser_list_item_t *)ptr->next;
				if(ptr){
					start = end+op_len;
					end = ptr->str;
				}else{
					start = end+op_len;
					end = expr+strlen(expr);
				}
			}
			
			//at this point we have the last token (end should be greater than start)
			token_len= end-start+1;
			if(token_len>0)
			{
				//We allocate an extra character 
				//to end the token with the char "\n" == 0;
				token = (char *)malloc(token_len);
				memcpy(token, start,token_len);
				token[token_len-1]=0;
				tokens->push(new parser_list_item_t(token));
			} else {
				//This is a syntax error (it means last operator
				//has no argument)
				delete tokens;
				return NULL;
			}
			tokens->reverse();
			
			parser_item_t **bt = (parser_item_t **)malloc(sizeof(parser_item_t*)*(n_internal_nodes+n_external_nodes));
			if(op_idx< OPERATORS_NUM){
				operator_t::operators[op_idx].expand_fptr(this, operator_t::operators[op_idx].id, bt, tokens);
			}else{
				delete tokens;
				return NULL;
			}
			delete node;
			node = bt[0];
			free(bt);
			delete tokens;
		}
		delete hpooeb;
	} //EOF IF st ==EXPAND
		
	//Phase 4 : Recursive iteration on children
	if(node->left)
		node->left = expand((parser_item_t *) node->left);
	if(node->right)
		node->right = expand((parser_item_t *) node->right);
	return node;
}

char * mparser_t::userfunction_replace(char* func_instance)
{
	//Finding function name
	function_def_t *func = (function_def_t *) userfunctions->head;
	if(func){
		while(func){
			char *funcname =func->name;
			unsigned int fneidx = 0;
			while(funcname[fneidx]!='(') {fneidx++;}
			if(memcmp(func_instance,funcname,fneidx)==0)
			{
				//Found
				break;
			}
			func=(function_def_t *)func->next;
		}
	}else{
		//Error: we expected a user function but did not find any suitable one
		return NULL;
	}

 	//printf("FUNC INSTANCE: %s\nFUNC NAME:%s\nFUNC DEF:%s\n",func_instance, fdef->name, fdef->def);
	char *funcname = func->name;
	char *funcdef = func->def;
	char buffA[strlen(func->def)+strlen(func_instance)];
	char buffB[strlen(func->def)+strlen(func_instance)];
	memset(buffA,0, strlen(func->def)+strlen(func_instance));
	memset(buffB,0, strlen(func->def)+strlen(func_instance));
	memcpy(buffA,funcdef, strlen(funcdef));
	char *ptrcurr =buffA, *ptrtarget =buffB;
	unsigned int varlen = 0;
	unsigned int offset = 1, offset1=1;
	// function_def_t:
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
			unsigned int i;
			
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
				for(i=0;i<OPERATORS_NUM;i++)
				{
					if(memcmp(op,operator_t::operators[i].op,strlen(operator_t::operators[i].op))==0) //Beginning of an operator
					{
						beginOk=true;
						op+=strlen(operator_t::operators[i].op);
						memcpy(&ptrtarget[offsettarget], &ptrcurr[offsetcurr], strlen(operator_t::operators[i].op));
						offsettarget+=strlen(operator_t::operators[i].op);
						offsetcurr+=strlen(operator_t::operators[i].op);
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
						
						for(i=0;i<OPERATORS_NUM;i++)
						{
							if(memcmp(op,operator_t::operators[i].op,strlen(operator_t::operators[i].op))==0){found=true;}
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
	char *res= (char *)malloc(strlen(func->def)+strlen(func_instance));
	memcpy(res,ptrcurr, strlen(func->def)+strlen(func_instance));
	//now ptrcurr stores the translation.
	return res;
}

parser_val_t mparser_t::uservariable_replace(char* var_instance)
{
	variable_def_t *v = (variable_def_t *)uservars->head;
	while(v){
		if(memcmp( v->name, var_instance, strlen(var_instance)) == 0)
		{
			return v->val;
		}
	}
	//Error
	return 0;
}

parser_val_t mparser_t::calculate(){
	return ((parser_item_t *)this->tree->root)->calculate();
}

parser_val_t parser_item_t::calculate()
{
	///Every node is in state "CALCULATE" or only some are if calculate was 
	///already invoked and we only did update some branches of the tree
	///(eg: by chainging the value of a variable)
	if(st == CALCULATE)
	{
		switch(op)
		{
			case(NOTOP):
			{
				unsigned int i;
				bool isvar=false;
				for(i=0;i<strlen(expr);i++)
				{
					//Check if it's a variable or a number by detecting characters
					if(expr[i] > 'A') isvar=true;
				}
				if(isvar==false){
					//must be a number
					sscanf(expr, "%f", &val);
				}
				else
				{
					val =  owner->uservariable_replace(expr);
				}
				break;
			}
			case(SUM):
				val = ((parser_item_t *)left)->calculate() + ((parser_item_t *)right)->calculate();
				break;
			case(SUB):
				val = ((parser_item_t *)left)->calculate() - ((parser_item_t *)right)->calculate();
				break;
			case(EXP):
				val = pow(((parser_item_t *)left)->calculate(),((parser_item_t *)right)->calculate());
				break;
			case(MULT):
				val = ((parser_item_t *)left)->calculate() * ((parser_item_t *)right)->calculate();
				break;
			case(DIV):
				val = ((parser_item_t *)left)->calculate() / ((parser_item_t *)right)->calculate();
				break;
			case(MOD):
				val = (int) ((parser_item_t *)left)->calculate() % (int) ((parser_item_t *)right)->calculate();
				break;
			case(COS):
				val = cos(((parser_item_t *)right)->calculate());
				break;
			case(SIN):
				val = sin(((parser_item_t *)right)->calculate());
				break;
			case(ATAN):
				val = atan(((parser_item_t *)right)->calculate());
				break;
			case(TAN):
				val = tan(((parser_item_t *)right)->calculate());
				break;
			case(NEP):
				val = exp(((parser_item_t *)right)->calculate());
				break;
			case(LOGN):
				val = log(((parser_item_t *)right)->calculate());
				break;
			case(LOG2):
				val = log2(((parser_item_t *)right)->calculate());
				break;
			case(LOG10):
				val = log10(((parser_item_t *)right)->calculate());
				break;
			default:
				goto error;
				break;
		}
		st = CALCULATED;
	}
	return val;	
error:
	return (parser_val_t) 0;
}

/***************************** Parser expand functions **********************************/
parser_item_t * mparser_t::operator_t::expand_std_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens){
	int operations = tokens->count()-1;
	if(operations>0)
	{
		unsigned int n_internal_nodes = operations;
		unsigned int n_external_nodes = n_internal_nodes+1;
		unsigned int count = 0;
		unsigned int to_append = 1;
		unsigned int i = 1;
		
		///ROOT Filling
		bt[count] = parser_item_t::operator_node(parser, id);
		
		int operands_start_index=0, operands_upperlayer=0;
		///Nodes left to fill
		while (to_append < n_internal_nodes + n_external_nodes)
		{
			///Node is an operator
			if(to_append < n_internal_nodes)
			{
				///Node to fill is a leftmost node
				if ( to_append == i )
				{
					bt[to_append] = parser_item_t::operator_node(parser, id);
					i = ((i + 1) * 2) - 1;
					operands_start_index = (n_internal_nodes + n_external_nodes - i) ;
					operands_upperlayer = (n_external_nodes > operands_start_index) ? n_external_nodes - operands_start_index : 0;
				}
				else //If it's not the leftmost node we could need to reverse the operation
				{
					if(id==DIV) bt[to_append]=parser_item_t::operator_node(parser, MULT);
					else if (id==SUB) bt[to_append] = parser_item_t::operator_node(parser, SUM);
					else bt[to_append] = parser_item_t::operator_node(parser, id);
				}
			}
			///LEFT Filling (to_append > internal && external > to_append) ==> (operand) 
			else
			{
				bt[to_append] = parser_item_t::operand_node(parser, ((mparser_t::parser_list_item_t *)(tokens->pop(((operands_upperlayer--)>0)?(operands_start_index):(0))))->str);
			}
			///Connecting
			bt[count]->left = bt[to_append];
			///Incrementing [ next is RIGHT ]
			to_append++;
			
			///RIGHT Filling (operator)
			if(to_append < n_internal_nodes)
			{
				if(id==DIV) bt[to_append] = parser_item_t::operator_node(parser, MULT);	
				else if (id==SUB) bt[to_append] = parser_item_t::operator_node(parser, SUM);
				else bt[to_append] = parser_item_t::operator_node(parser, id);
			}
			///RIGHT Filling (to_append > internal && external> to_append) ==> (operand)
			else
			{
				bt[to_append] = parser_item_t::operand_node(parser, ((mparser_t::parser_list_item_t *)tokens->pop((((operands_upperlayer--))>0)?(operands_start_index):(0)))->str);
			}
			///Connecting
			bt[count]->right = bt[to_append];
			///Incrementing [next is LEFT]
			to_append++;
			///Incrementing node to attach new nodes to
			count++;
		}
	}
	return bt[0];
}

parser_item_t * mparser_t::operator_t::expand_alt_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens){
	int operations = tokens->count()-1;
	if(operations>0)
	{	
		unsigned int n_internal_nodes = operations;
		unsigned int n_external_nodes = n_internal_nodes+1;
		unsigned int count = 0;
		unsigned int to_append = 1;
		///ROOT Filling
		bt[count] = parser_item_t::operator_node(parser, id);
		///Tokens must be inserted in swapped (list_reverse) order.
		tokens->reverse();
		while (to_append < n_internal_nodes + n_external_nodes)
		{
			bt[to_append] = parser_item_t::operand_node(parser, ((parser_item_t *) tokens->pop((uint32_t)0))->expr);
			bt[count]->right = bt[to_append];
			///Incrementing node to attach
			to_append++;
			if(to_append == n_internal_nodes + n_external_nodes - 1)
			{
				///We got to the leftmost node (it's an operand!)
				bt[to_append] = parser_item_t::operand_node(parser, ((parser_item_t *) tokens->pop((uint32_t)0))->expr);
				bt[count]->left = bt[to_append];
				break;
			}
			else
			{
				///Filling left operator
				bt[to_append] = parser_item_t::operator_node(parser, id);
				bt[count]->left = bt[to_append];
			}
			///Incrementing node to attach
			to_append++;
			///Incrementing node to attach new nodes to (by 2 to go left)
			count+=2;
		}
	}
	return bt[0];
}

parser_item_t * mparser_t::operator_t::expand_1op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens){
	///ROOT Filling
	bt[0] = parser_item_t::operator_node(parser, id);
	
	///RIGHT Filling (operand)
	bt[1] = parser_item_t::operand_node(parser, (((mparser_t::parser_list_item_t *) tokens->pop(1)))->str);
	
	///Connecting
	bt[0]->right = bt[1];
	bt[0]->left = NULL;
	return bt[0];
}

