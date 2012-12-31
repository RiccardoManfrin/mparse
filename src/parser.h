/** 
* @file parser.h
* 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* 
* @brief  Parser methods definition
*/

#ifndef __PARSER_H__
#define __PARSER_H__
#include "btree.h"
#include "list.h"
#include "parser_operators.h"


#define expr2val atof
typedef float parser_val_t;

class parser_list_item_t: public list_item_t{
public:
	char *str;
public:
	parser_list_item_t(){str=NULL;}
	parser_list_item_t(char *str){this->str=str;}
	~parser_list_item_t(){if(str) free(str);}
	virtual bool compare(list_item_t* comparable){
		return ( ((parser_list_item_t *)comparable)->str == str);
	}
};

///@brief Mathematical parser object
class mparser_t {
	
public:
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
		function_def_t(){
			name=NULL;
			def=NULL;
		}
		virtual bool compare(list_item_t* comparable){
			function_def_t *compared = (function_def_t *)comparable;
			if( compared->name == this->name ) return true;
			return false;
		}
	};

	///@brief Variable definition
	class variable_def_t:public list_item_t, public btree_item_t{
	public:
		char * name;
		parser_val_t val;
	public:
		variable_def_t(){
			name=NULL;
		}
		virtual bool compare(list_item_t* comparable){
			variable_def_t *compared = (variable_def_t *)comparable;
			if( compared->name == this->name ) return true;
			return false;
		}
	};
	
	///@brief Basic parser binary tree structure
	class parser_btree_item_t: public btree_item_t, public list_item_t{
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
		const char 	*op;
		parser_btree_item_t *(* expand_fptr)(parser_btree_item_t **btnodesarray, list_t *tk_indexes);
	};
	
private:
	///@brief List of variables used throughout the parsed expression
	list_t  *uservars;
	///@brief List of user defined functions used throughout the parsed expression
	list_t  *userfunctions;
	///@brief Binary tree used to expand the exprenssions 
	btree_t *tree;
	///@brief Information on the standard operators
	static operator_t operators[NOTOP];
	
private:
	///@brief Internal recursive btree expression expander
	void expand(parser_btree_item_t *node);
	///@brief User function replace function
	char *userfunction_replace(char *func_instance);
	
public:
	///@brief Allocate parser
	mparser_t();
	///@brief Declare new variable
	void variable(char *name, parser_val_t value);
	///@brief Declare new function
	void function(char *name, char *expanded);
	///@brief Define parser expression 
	void expression(char *expr);
	///@brief Calculate the result of the inserted expression
	parser_val_t calculate();
	///@brief Deallocate all parser structures
	~mparser_t();
};

#endif