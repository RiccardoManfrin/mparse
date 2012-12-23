/** 
* @author Riccardo Manfrin [namesurname at gmail dot com]
* @brief  Parser methods definition
*/
 
#ifndef __LIB_PARSE_H__
#define __LIB_PARSE_H__
#include "btree.h"
#include "list.h"
#include "std_functions.h"

class parser_btree_item_t;
class parser_string_list_item_t;
class parser_idx_list_item_t;

#define expr2val atof
#define parser_val_t float

struct operation_t {
	op_id_t  id;
	char 	*op;
	parser_btree_item_t *(* expand_fptr)(struct operation_t *me, parser_btree_item_t **btnodesarray, list_t *tk_indexes);
};

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
class parser_btree_item_t:public list_item_t, public btree_item_t {
public:
	char *expr;
	op_id_t op;
	state st;
	parser_val_t val;
public:
	parser_btree_item_t(){
			expr=NULL;
			op=NOTOP;
	}
	virtual ~parser_btree_item_t(){
			if(expr)free(expr);
	}
};

class parser_idx_list_item_t: public list_item_t{
public:
	int idx;
public:
	parser_idx_list_item_t(){};
	parser_idx_list_item_t(int idx){this->idx=idx;};
};

class parser_string_list_item_t: public list_item_t{
public:
	char *str;
public:
	parser_string_list_item_t(){str=NULL;}
	parser_string_list_item_t(char *str){this->str=str;}
	~parser_string_list_item_t(){if(str) free(str);}
};

class parser_func_def_t:public list_item_t, public btree_item_t{
public:
	char * name;
	char * def;
};

class parser_var_def_t:public list_item_t, public btree_item_t{
public:
	char * name;
	parser_val_t val;
};

class parser_data_t:public list_item_t, public btree_item_t{
public:
	char *expr;
	bool toupdate;
};


typedef struct{
	list_t  *vars;
	list_t  *lfunctions;
	btree_t *pstruct;
	void  *data;
} parser_t;

 /**
  * Here we define new functions to be used within the 
  * expression to parse.
  * Each new function must be in the form
  * 
  * 	"my_new_function(myvar1,var2,3)"
  *
  * while the explaination must be 
  * 
  * 	"myvar1+myvar2/3"
  *
  * Functions explaination can contain other functions 
  * as long as these are already defined.
  *
  * The function returns 0 upon successful completition
  * or -1 in case of errors
  */
int parser_func(parser_t *p, char *function, char *replacement_expr);

 /**
  * Here we define the variables that will be used within
  * the mathematical expression to solve.
  * Calling the function multiple times on the same 
  * variable will just update its value.
  *
  * The function returns 0 upon successful completition
  * or -1 in case of errors
  */
int parser_var(parser_t *p, char *var, parser_val_t value);


/**
 * This function is used to generate a new parser based on
 * the mathematical expression provided as argument.
 */
parser_t * parser_new();

 /**
  * This function must be called on the root and
  * processes the tree until it is complete;
  * The way it works is to expand the root and proceed
  * with the leaf nodes as long as the algorithm find
  * leaf nodes to expand.
  * This function makes use of the btree dispatcher and
  * of recursion to work things out.
  */
void parser_expand(parser_t *p, char *expr);

 /**
  * Calculate the result of the expression
  */
parser_val_t parser_calc(parser_t *p, char *expr);

 /**
  * Destroyer to invoke when done with parsing
  */
void parser_destroy(parser_t *p);

static struct operation_t operation[NOTOP];

#include "std_functions_expand.h"

#endif
