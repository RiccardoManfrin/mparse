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

#include "operator.h"

typedef float parser_val_t;

///Parser btree Node possible states
typedef enum{
	///Node holds an expression to be parsed
	EXPAND,
	///Node doesn't need further expansion and is to be calculated
	CALCULATE,
	///Node has the numerical result value so we can use it for the above node computation.
	CALCULATED
} state_t;

class parser_list_item_t;
class operator_t;
class parser_item_t;
class mparser_t;


///@brief Parser tokens list
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



///@brief Basic parser structure
class parser_item_t: public btree_item_t, public list_item_t{
	friend class mparser_t;
private:
	mparser_t *owner;
public:
	char *expr;
	op_id_t op;
	state_t st;
	parser_val_t val;
	
private:
	parser_val_t calculate();
	
public:
	///@brief Constructor
	parser_item_t(mparser_t *owner){
			expr=NULL;
// 			op=NOTOP;
			st=EXPAND;
			this->owner = owner;
	}
	///@brief Destructor
	virtual ~parser_item_t(){
			if(expr)free(expr);
	}
	///@brief Operator node allocator
	static parser_item_t* operator_node(mparser_t *owner, op_id_t optor)
	{
		parser_item_t *data=new parser_item_t(owner);
		data->st = CALCULATE;
		data->op = optor;
		//data->expr = NULL;
		return data;
	}
	///@brief Operator node allocator
	static parser_item_t* operand_node(mparser_t *owner, char* expr)
	{
		parser_item_t *data=new parser_item_t(owner);
		data->expr = expr;
		data->st = EXPAND;
		//data->op = NOTOP;
		return data;
	}
};




///@brief Operation information
class operator_t{
friend class mparser_t;
private:
	///@brief Shared information on the standard supported operators
	static operator_t operators[OPERATORS_NUM];
private:
	static parser_item_t * expand_std_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
	static parser_item_t * expand_alt_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
	static parser_item_t * expand_1op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
public:
	op_id_t  id;
	const char 	*op;
	parser_item_t *(* expand_fptr)(mparser_t *owner, op_id_t id, parser_item_t **btnodesarray, list_t *tk_indexes);
public:
	static void init();
};




///@brief Mathematical parser object
class mparser_t {
	friend class parser_item_t;
public:
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
	
private:
	///@brief List of variables used throughout the parsed expression
	list_t  *uservars;
	///@brief List of user defined functions used throughout the parsed expression
	list_t  *userfunctions;
	///@brief Binary tree used to expand the exprenssions 
	btree_t *tree;
	
private:
	///@brief Internal recursive btree expression expander
	parser_item_t * expand(parser_item_t *node);
	///@brief User function replace function
	char *userfunction_replace(char *func_instance);
	///@brief User function replace function
	parser_val_t uservariable_replace(char *var_instance);
public:
	///@brief Allocate parser
	mparser_t();
	///@brief Fetch all the standard operations available
	static void init();
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