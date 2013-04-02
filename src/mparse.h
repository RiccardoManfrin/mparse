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

///@brief Operator ID value enum
typedef enum {
	SUM			=0,
	SUB			=1,
	EXP			=2,
	MULT		=3,
	DIV			=4,
	MOD			=5,
	COS			=6,
	SIN			=7,
	ATAN		=8,
	TAN			=9,
	NEP			=10,
	LOGN		=11,
	LOG2		=12,
	LOG10		=13,
	NOTOP		=14,
} op_id_t;

const unsigned int OPERATORS_NUM = 14;

typedef float parser_val_t;

class mparser_t;

///@brief Basic parser structure
class parser_item_t: public btree_item_t, public list_item_t{
	friend class mparser_t;
private:
	///@brief Parser btree Node possible states
	typedef enum{
		///Node holds an expression to be parsed
		EXPAND,
		///Node doesn't need further expansion and is to be calculated
		CALCULATE,
		///Node has the numerical result value so we can use it for the above node computation.
		CALCULATED
	} state_t;
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



///@brief Mathematical parser object
class mparser_t {
	friend class parser_item_t; //Required to access to user vars and functions list
	friend class operator_t; //Required to access to parser_list_item_t class methods
private:
	///@brief Parser tokens list to support expression parsing
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

public:	
	///@brief Standard 2 operands operator expander function
	static parser_item_t * expand_std_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
	///@brief Alternate 2 operands operator expander function
	static parser_item_t * expand_alt_2op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
	///@brief Standard 1 operand operator expander function
	static parser_item_t * expand_1op_func(mparser_t *parser, op_id_t id, parser_item_t **bt, list_t *tokens);
	

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
		list_t *refs;
	public:
		variable_def_t(){
			name=NULL;
			refs = new list_t();
		}
		virtual bool compare(list_item_t* comparable){
			variable_def_t *compared = (variable_def_t *)comparable;
			if( compared->name == this->name ) return true;
			return false;
		}
		~variable_def_t(){
			delete refs;
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
	bool uservariable_replace(parser_item_t *node);
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