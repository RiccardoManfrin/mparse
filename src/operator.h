/**
 * @file operator.h
 * 
 * In order to add a new standard function, you need to perform
 * 6 different actions:
 * 
 * 1) Add the function string expression "FUNC_OP" in std_functions.h
 * 2) Add the function ID "FUNC" in std_functions.h"
 * 3) Add the expand function in std_functions_expand.h (here) 
 * 	and define the corresponding macro
 * 4) Add the solver function in std_functions_solve.h"
 * 	and define the corresponding macro
 * 5) Add function initialization in init_func_bind function (here)
 * 6) Increase the "NOTOP" value by one in std_functions.h
 */
#ifndef __OPERATOR_H__
#define __OPERATOR_H__

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

static const char 	*SUM_OP = "+";
static const char 	*SUB_OP = "-";
static const char  	*EXP_OP = "^";
static const char 	*MULT_OP = "*";
static const char 	*DIV_OP = "/";
static const char 	*MOD_OP = "%";
static const char 	*COS_OP = "cos";
static const char 	*SIN_OP = "sin";
static const char 	*ATAN_OP = "atan";
static const char  	*TAN_OP = "tan";
static const char 	*NEP_OP = "exp";
static const char 	*LOGN_OP = "logn";
static const char 	*LOG2_OP = "log2";
static const char 	*LOG10_OP = "log10";

const char *optors_str[NOTOP] = {SUM_OP, SUB_OP, EXP_OP, MULT_OP, DIV_OP, MOD_OP, COS_OP, SIN_OP, ATAN_OP,
							TAN_OP, NEP_OP, LOGN_OP, LOG2_OP, LOG10_OP};


#endif