/** 
* @file parser.h
* 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Parser methods definition
*/

#ifndef __LIB_PARSE_H__
#define __LIB_PARSE_H__
#include "btree.h"
#include "list.h"
#include "std_functions.h"


#define expr2val atof
typedef float parser_val_t;

class parser_list_item_t: public list_item_t{
public:
	union{
		char *str;
		int idx;
	};
public:
	parser_idx_list_item_t(){};
	parser_idx_list_item_t(int idx){this->idx=idx;};
	parser_string_list_item_t(){str=NULL;}
	parser_string_list_item_t(char *str){this->str=str;}
	~parser_string_list_item_t(){if(str) free(str);}
};

///@brief Mathematical parser object
class mparser_t {
private:
	///Parser btree Node possible states
	typedef enum{
		///Node holds an expression to be parsed
		EXPAND,
		///Node doesn't need further expansion and is to be calculated
		CALCULATE,
		///Node has the numerical result value so we can use it for the above node computation.
		CALCULATED
	} state_t;

	///@brief Function class definition
	class function_def_t:public list_item_t, public btree_item_t{
	public:
		char * name;
		char * def;
	public:
		parser_func_def_t(){
			name=NULL;
			def=NULL;
		}
	};

	///@brief Variable definition
	class variable_def_t:public list_item_t, public btree_item_t{
	public:
		char * name;
		parser_val_t val;
	public:
		parser_var_def_t(){
			name=NULL;
		}
	};
	
	///@brief Basic parser binary tree structure
	class parser_btree_item_t:public list_item_t, public btree_item_t {
	public:
		char *expr;
		op_id_t op;
		state_t st;
		parser_val_t val;
	public:
		parser_btree_item_t(){
				expr=NULL;
				op=NOTOP;
				st=EXPAND;
		}
		virtual ~parser_btree_item_t(){
				if(expr)free(expr);
		}
	};
	
	///@brief Operation information
	class operator_t{
	public:
		op_id_t  id;
		char 	*op;
		parser_btree_item_t *(* expand_fptr)(struct operation_t *me, parser_btree_item_t **btnodesarray, list_t *tk_indexes);
	};
	
private:
	///@brief List of variables used throughout the parsed expression
	list_t  *uservars;
	///@brief List of user defined functions used throughout the parsed expression
	list_t  *userfunctions;
	///@brief Binary tree used to expand the exprenssions 
	btree_t *tree;
public:
	///@brief Allocate parser
	mparser_t();
	///@brief Declare new variable
	void variable(char *name, parser_val_t value);
	///@brief Declare new function
	void function(char *name, char *expanded);
	///@brief Define parser expression 
	void expression();
	///@brief Calculate the result of the inserted expression
	parser_val_t calculate();
	///@brief Deallocate all parser structures
	~mparser_t();
};