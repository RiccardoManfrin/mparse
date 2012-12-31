#ifndef __OPERATORS_H__
#define __OPERATORS_H__

#include "parser.h"

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
	//This confines the number of operators
	NOTOP		=14,
} op_id_t;

//			NAME		OP (string)
const char 	*SUM_OP = "+";
const char 	*SUB_OP = "-";
const char  *EXP_OP = "^";
const char 	*MULT_OP = "*";
const char 	*DIV_OP = "/";
const char 	*MOD_OP = "%";
const char 	*COS_OP = "cos";
const char 	*SIN_OP = "sin";
const char 	*ATAN_OP = "atan";
const char  *TAN_OP = "tan";
const char 	*NEP_OP = "exp";
const char 	*LOGN_OP = "logn";
const char 	*LOG2_OP = "log2";
const char 	*LOG10_OP = "log10";

const char *optors_str[NOTOP] = {SUM_OP, SUB_OP, EXP_OP, MULT_OP, DIV_OP, MOD_OP, COS_OP, SIN_OP, ATAN_OP,
							TAN_OP, NEP_OP, LOGN_OP, LOG2_OP, LOG10_OP};

#endif