#ifndef EXCEPTION_H
#define EXCEPTION_H

///Here it's where we switch if to use exceptions or not by defining EXCEPTION
#include "defines.h" 

typedef enum{
	SYNTAX_ERROR,
	EMPTY_OBJ,
	MISSMATCHING_BRACKETS,
	EMPTY_BRACKETS_ARG,
	MISSING_OP_ARGUMENT,
	OP_NOT_RECOGNIZED,
	NULL_EXCEPTION,
	MISSING_PRINTING_FUNCTION,
	NOT_INITIALIZED,
	NON_NULL_PTR,
	OUT_OF_BOUND_INDEX
	//Add YOUR OWN exceptions here
} exception_type;

#ifdef EXCEPTION
void exception(char *filename, int line, exception_type x);

///Specialization of execption for the list
#define excp(x) (exception(__FILE__, __LINE__, x))

#else
#define excp(x) 

#endif


#endif
