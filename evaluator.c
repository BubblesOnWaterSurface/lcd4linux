/* $Id: evaluator.c,v 1.9 2004/01/15 07:47:02 reinelt Exp $
 *
 * expression evaluation
 *
 * based on EE (Expression Evaluator) which is 
 * (c) 1992 Mark Morley <morley@Camosun.BC.CA>
 * 
 * heavily modified 2003 by Michael Reinelt <reinelt@eunet.at>
 *
 * FIXME: GPL or not GPL????
 *
 * $Log: evaluator.c,v $
 * Revision 1.9  2004/01/15 07:47:02  reinelt
 * debian/ postinst and watch added (did CVS forget about them?)
 * evaluator: conditional expressions (a?b:c) added
 * text widget nearly finished
 *
 * Revision 1.8  2004/01/12 03:51:01  reinelt
 * evaluating the 'Variables' section in the config file
 *
 * Revision 1.7  2004/01/07 10:15:41  reinelt
 * small glitch in evaluator fixed
 * made config table sorted and access with bsearch(),
 * which should be much faster
 *
 * Revision 1.6  2004/01/06 23:01:37  reinelt
 * more copyright issues
 *
 * Revision 1.5  2004/01/06 17:33:45  reinelt
 * Evaluator: functions with variable argument lists
 * Evaluator: plugin_sample.c and README.Plugins added
 *
 * Revision 1.4  2004/01/06 15:19:12  reinelt
 * Evaluator rearrangements...
 *
 * Revision 1.3  2003/10/11 06:01:52  reinelt
 * renamed expression.{c,h} to client.{c,h}
 * added config file client
 * new functions 'AddNumericVariable()' and 'AddStringVariable()'
 * new parameter '-i' for interactive mode
 *
 * Revision 1.2  2003/10/06 05:47:27  reinelt
 * operators: ==, \!=, <=, >=
 *
 * Revision 1.1  2003/10/06 04:34:06  reinelt
 * expression evaluator added
 *
 */


/***************************************************************************
 **                                                                       **
 ** EE.C         Expression Evaluator                                     **
 **                                                                       **
 ** AUTHOR:      Mark Morley                                              **
 ** COPYRIGHT:   (c) 1992 by Mark Morley                                  **
 ** DATE:        December 1991                                            **
 ** HISTORY:     Jan 1992 - Made it squash all command line arguments     **
 **                         into one big long string.                     **
 **                       - It now can set/get VMS symbols as if they     **
 **                         were variables.                               **
 **                       - Changed max variable name length from 5 to 15 **
 **              Jun 1992 - Updated comments and docs                     **
 **                                                                       **
 ** You are free to incorporate this code into your own works, even if it **
 ** is a commercial application.  However, you may not charge anyone else **
 ** for the use of this code!  If you intend to distribute your code,     **
 ** I'd appreciate it if you left this message intact.  I'd like to       **
 ** receive credit wherever it is appropriate.  Thanks!                   **
 **                                                                       **
 ** I don't promise that this code does what you think it does...         **
 **                                                                       **
 ** Please mail any bug reports/fixes/enhancments to me at:               **
 **      morley@camosun.bc.ca                                             **
 ** or                                                                    **
 **      Mark Morley                                                      **
 **      3889 Mildred Street                                              **
 **      Victoria, BC  Canada                                             **
 **      V8Z 7G1                                                          **
 **      (604) 479-7861                                                   **
 **                                                                       **
 ***************************************************************************/


/* 
 * exported functions:
 *
 * void DelResult (RESULT *result)
 *   sets a result to none
 *   frees a probably allocated memory
 *
 * int SetVariable (char *name, RESULT *value)
 *   adds a generic variable to the evaluator
 *
 * int AddNumericVariable(char *name, double value)
 *   adds a numerical variable to the evaluator
 *
 * int AddStringVariable(char *name, char *value)
 *   adds a numerical variable to the evaluator
 *
 * int AddFunction (char *name, int args, void (*func)())
 *   adds a function to the evaluator
 *
 * RESULT* SetResult (RESULT **result, int type, void *value)
 *   initializes a result
 *
 * double R2N (RESULT *result)
 *   converts a result into a number
 *
 * char* R2S (RESULT *result)
 *   converts a result into a string
 *
 * int Eval (char* expression, RESULT *result)
 *   evaluates an expression
 *
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>

#include "debug.h"
#include "evaluator.h"


// Token types
#define T_DELIMITER 1
#define T_NUMBER    2
#define T_STRING    3
#define T_NAME      4


#define is_blank(c)  (c==' ' || c=='\t')
#define is_number(c) (isdigit(c) || c=='.')
#define is_name(c)   (isalnum(c) || c=='_')
#define is_delim(c)  (strchr("+-*/%^().,;:=<>?!&|", c)!=NULL)


typedef struct {
  char   *name;
  RESULT *value;
} VARIABLE;

static VARIABLE *Variable=NULL;
static int      nVariable=0;


typedef struct {
  char *name;
  int   args;
  void (*func)();
} FUNCTION;

static FUNCTION *Function=NULL;
static int      nFunction=0;
static int       FunctionSorted=0;

static char *Expression=NULL;
static char *Token=NULL;
static int   Type=0;


// error handling
#define ERROR(n) longjmp(jb,n)
jmp_buf jb;

char* ErrMsg[] = {
  "",
  "Syntax error",
  "Unbalanced parenthesis",
  "Division by zero",
  "Unknown variable",
  "Unrecognized function",
  "Wrong number of arguments",
  "Missing an argument",
  "Empty expression"
};



void DelResult (RESULT *result)
{
  result->type=0;
  result->number=0.0;
  if (result->string) {
    free (result->string);
    result->string=NULL;
  }
}


static void FreeResult (RESULT *result)
{
  if (result!=NULL) {
    DelResult(result);
    free (result);
  }
}


static RESULT* NewResult (void)
{
  RESULT *result = malloc(sizeof(RESULT));
  if (result==NULL) {
    error ("cannot allocate result: out of memory!");
    return NULL;
  }
  result->type=0;
  result->number=0.0;
  result->string=NULL;
  
  return result;
}


static RESULT* DupResult (RESULT *result)
{
  RESULT *result2;
  
  if ((result2=NewResult())==NULL) 
    return NULL;
  
  result2->type=result->type;
  result2->number=result->number;
  if (result->string!=NULL)
    result2->string=strdup(result->string);
  else    
    result2->string=NULL;
  
  return result2;
}


RESULT* SetResult (RESULT **result, int type, void *value)
{
  if (*result) {
    DelResult(*result);
  } else {
    if ((*result=NewResult())==NULL) 
      return NULL;
  }
  
  if (type==R_NUMBER) {
    (*result)->type=R_NUMBER;
    (*result)->number=*(double*)value;
    (*result)->string=NULL;
  } else if (type==R_STRING) {
    (*result)->type=R_STRING;
    (*result)->string=strdup(value);
  } else {
    error ("internal error: invalid result type %d", type); 
    return NULL;
  }
  
  return *result;
}


double R2N (RESULT *result)
{
  if (result==NULL) {
    error ("internal error: NULL result");
    return 0.0;
  }

  if (result->type & R_NUMBER) {
    return result->number;
  }
  
  if (result->type & R_STRING) {
    result->type |= R_NUMBER;
    result->number=atof(result->string);
    return result->number;
  }
  
  error ("internal error: invalid result type %d", result->type); 
  return 0.0;
}


char* R2S (RESULT *result)
{
  char buffer[16];
  
  if (result==NULL) {
    error ("internal error: NULL result");
    return NULL;
  }

  if (result->type & R_STRING) {
    return result->string;
  }
  
  if (result->type & R_NUMBER) {
    sprintf(buffer, "%g", result->number);
    result->type |= R_STRING;
    result->string=strdup(buffer);
    return result->string;
  }
  
  error ("internal error: invalid result type %d", result->type); 
  return NULL;
  
}


// bsearch compare function for variables 
static int v_lookup (const void *a, const void *b)
{
  char *name=(char*)a;
  VARIABLE *v=(VARIABLE*)b;

  return strcmp(name, v->name);
}


// qsort compare function for variables
static int v_sort (const void *a, const void *b)
{
  VARIABLE *va=(VARIABLE*)a;
  VARIABLE *vb=(VARIABLE*)b;

  return strcmp(va->name, vb->name);
}


static int DelVariable (char *name)
{
  VARIABLE *V;

  V=bsearch(name, Variable, nVariable, sizeof(VARIABLE), v_lookup);
  if (V!=NULL) {
    FreeResult (V->value);
    memmove (V, V+1, (nVariable-1)*sizeof(VARIABLE)-(V-Variable));
    nVariable--;
    Variable=realloc(Variable, nVariable*sizeof(VARIABLE));
    return 1;
  }
  return 0;
}


static int GetVariable (char *name, RESULT *value)
{
  VARIABLE *V;

  V=bsearch(name, Variable, nVariable, sizeof(VARIABLE), v_lookup);
  if (V!=NULL) {
    value->type=V->value->type;
    value->number=V->value->number;
    if (V->value->string!=NULL) value->string=strdup(V->value->string);
    else value->string=NULL;
    return 1;
  }
  
  DelResult (value);
  return 0;
}


int SetVariable (char *name, RESULT *value)
{
  VARIABLE *V;

  V=bsearch(name, Variable, nVariable, sizeof(VARIABLE), v_lookup);
  if (V!=NULL) {
    FreeResult (V->value);
    V->value=DupResult(value);
    return 1;
  }
  
  // we append the var at the end and re-sort
  // the whole array. As every SetVariable call
  // implies a bsearch(), this is ok and cannot
  // be optimized.
  nVariable++;
  Variable=realloc(Variable, nVariable*sizeof(VARIABLE));
  Variable[nVariable-1].name=strdup(name);
  Variable[nVariable-1].value=DupResult(value);
  qsort(Variable, nVariable, sizeof(VARIABLE), v_sort);

  return 0;
}


int AddNumericVariable (char *name, double value)
{
  RESULT result;
  
  result.type=R_NUMBER;
  result.number=value;
  result.string=NULL;
  
  return SetVariable (name, &result);
}


int AddStringVariable (char *name, char *value)
{
  RESULT result;
  
  result.type=R_STRING;
  result.number=0.0;
  result.string=strdup(value);
  
  return SetVariable (name, &result);
}


// bsearch compare function for functions 
static int f_lookup (const void *a, const void *b)
{
  char *name=(char*)a;
  FUNCTION *f=(FUNCTION*)b;
  return strcmp(name, f->name);
}


// qsort compare function for functions
static int f_sort (const void *a, const void *b)
{
  FUNCTION *va=(FUNCTION*)a;
  FUNCTION *vb=(FUNCTION*)b;
  return strcmp(va->name, vb->name);
}


static FUNCTION* GetFunction (char *name)
{
  FUNCTION *F;
  
  if (!FunctionSorted) {
    FunctionSorted=1;
    qsort(Function, nFunction, sizeof(FUNCTION), f_sort);
  }
  
  F=bsearch(name, Function, nFunction, sizeof(FUNCTION), f_lookup);
  
  return F;
}


int AddFunction (char *name, int args, void (*func)())
{
  // we append the func at the end, and flag
  // the function table as unsorted. It will be
  // sorted again on the next GetFunction call.
  FunctionSorted=0;
  nFunction++;
  Function=realloc(Function, nFunction*sizeof(FUNCTION));
  Function[nFunction-1].name=strdup(name);
  Function[nFunction-1].args=args;
  Function[nFunction-1].func=func;
  
  return 0;
}



// Prototypes
static void Level01 (RESULT *result);
static void Level02 (RESULT *result);
static void Level03 (RESULT *result);
static void Level04 (RESULT *result);
static void Level05 (RESULT *result);
static void Level06 (RESULT *result);
static void Level07 (RESULT *result);
static void Level08 (RESULT *result);
static void Level09 (RESULT *result);
static void Level10 (RESULT *result);
static void Level11 (RESULT *result);
static void Level12 (RESULT *result);



static void Parse (void)
{
  char *start;
  
  Type=0;
  if (Token) {
    free (Token);
    Token=NULL;
  }
  
  while (is_blank(*Expression)) Expression++;
  
  if (is_delim(*Expression)) {
    Type=T_DELIMITER;
    // special case for <=, >=, ==, !=
    if (strchr("<>=!", *Expression)!=NULL && *(Expression+1)=='=') {
      Token=strndup(Expression, 2);
      Expression+=2;
    } else {
      Token=strndup(Expression, 1);
      Expression++;
    }
  }
  
  else if (isdigit(*Expression)) {
    Type=T_NUMBER;
    start=Expression; 
    while (is_number(*Expression)) Expression++;
    Token=strndup(start, Expression-start);
  }
  
  else if (is_name(*Expression)) {
    Type=T_NAME;
    start=Expression;
    while (is_name(*Expression)) Expression++;
    Token=strndup(start, Expression-start);
  }
  
  else if (*Expression=='\'') {
    Type=T_STRING;
    start=++Expression;
    while (*Expression && *Expression!='\'') Expression++;
    Token=strndup(start, Expression-start);
    if (*Expression=='\'') Expression++;
  }
  
  else if (*Expression) {
    ERROR(E_SYNTAX);
  }
  
  while(is_blank(*Expression)) Expression++;

  // empty token
  if (Token==NULL) Token=strdup("");
}



// expression lists
static void Level01 (RESULT *result)
{
  do {
    while (Type==T_DELIMITER && *Token==';') Parse();
    Level02(result);
  } while (Type==T_DELIMITER && *Token==';');
}


// variable assignments
static void Level02 (RESULT *result)
{
  char *name;
  
  if (Type==T_NAME) {
    if (Expression[0]=='=' && Expression[1]!='=') {
      name=strdup(Token);
      Parse();
      Parse();
      if (*Token && (Type!=T_DELIMITER || *Token!=';')) {
	Level03(result);
	SetVariable(name, result);
      } else {
	DelVariable(name);
      }
      free (name);
      return;
    }
  }
  Level03(result);
}


// conditional expression a?b:c
static void Level03 (RESULT *result)
{
  RESULT r_then = {0, 0.0, NULL};
  RESULT r_else = {0, 0.0, NULL};
  
  Level04(result);
  
  while(Type==T_DELIMITER && *Token=='?') {
    Parse();
    Level01 (&r_then);
    if (Type==T_DELIMITER && *Token==':') {
      Parse();
      Level01 (&r_else);
    } else {
      ERROR(E_SYNTAX);
    }
    if (R2N(result)!=0.0) {
      DelResult(result);
      DelResult(&r_else);
      *result=r_then;
    } else {
      DelResult(result);
      DelResult(&r_then);
      *result=r_else;
    }
  }
}


// logical 'or'
static void Level04 (RESULT *result)
{
  RESULT operand = {0, 0.0, NULL};
  double value;
  
  Level05(result);
  
  while(Type==T_DELIMITER && *Token=='|') {
    Parse();
    Level05 (&operand);
    value = (R2N(result)!=0.0) || (R2N(&operand)!=0.0);
    SetResult(&result, R_NUMBER, &value); 
  }
}


// logical 'and'
static void Level05 (RESULT *result)
{
  RESULT operand;
  double value;
  
  Level06(result);
  
  while(Type==T_DELIMITER && *Token=='&') {
    Parse();
    Level06 (&operand);
    value = (R2N(result)!=0.0) && (R2N(&operand)!=0.0);
    SetResult(&result, R_NUMBER, &value); 
  }
}


// equal, not equal
static void Level06 (RESULT *result)
{
  char operator;
  RESULT operand = {0, 0.0, NULL};
  double value;
  
  Level07 (result);
  
  if (Type==T_DELIMITER && ((operator=Token[0])=='=' || operator=='!') && Token[1]=='=') {
    Parse();
    Level07 (&operand);
    if (operator=='=')
      value = (R2N(result) == R2N(&operand));
    else
      value = (R2N(result) != R2N(&operand));
    SetResult(&result, R_NUMBER, &value); 
  }
}


// relational operators
static void Level07 (RESULT *result)
{
  char operator[2];
  RESULT operand = {0, 0.0, NULL};
  double value;
  
  Level08 (result);
  
  if (Type==T_DELIMITER && (*Token=='<' || *Token=='>')) {
    operator[0]=Token[0];
    operator[1]=Token[1];
    Parse();
    Level08 (&operand);
    if (operator[0]=='<')
      if (operator[1]=='=')
	value = (R2N(result) <= R2N(&operand));
      else
	value = (R2N(result) <  R2N(&operand));
    else
      if (operator[1]=='=')
	value = (R2N(result) >= R2N(&operand));
      else
	value = (R2N(result) >  R2N(&operand));
    SetResult(&result, R_NUMBER, &value); 
  }
}


// addition, subtraction, concatenation
static void Level08 (RESULT *result)
{
  char operator;
  RESULT operand = {0, 0.0, NULL};
  double value;
  
  Level09(result);
  
  while(Type==T_DELIMITER && ((operator=*Token)=='+' || operator=='-' || operator=='.')) {
    Parse();
    Level09 (&operand);
    if (operator=='+') {
      value = (R2N(result) + R2N(&operand));
      SetResult(&result, R_NUMBER, &value); 
    } else if (operator=='-') {
      value = (R2N(result) - R2N(&operand));
      SetResult(&result, R_NUMBER, &value); 
    } else {
      char *s1=R2S(result);
      char *s2=R2S(&operand);
      char *s3=malloc(strlen(s1)+strlen(s2)+1);
      strcpy (s3, s1);
      strcat (s3, s2);
      SetResult (&result, R_STRING, s3);
      free (s3);
    }
  }
}


// multiplication, division, modulo
static void Level09 (RESULT *result)
{
  char operator;
  RESULT operand = {0, 0.0, NULL};
  double value;
  
  Level10 (result);
  
  while(Type==T_DELIMITER && ((operator=*Token)=='*' || operator=='/' || operator=='%')) {
    Parse();
    Level10(&operand);
    if (operator == '*') {
      value = (R2N(result) * R2N(&operand));
    } else if (operator == '/') {
      if (R2N(&operand) == 0.0) ERROR (E_DIVZERO);
      value = (R2N(result) / R2N(&operand));
    } else {
      if (R2N(&operand) == 0.0) ERROR (E_DIVZERO);
      value = fmod(R2N(result), R2N(&operand));
    }
    SetResult(&result, R_NUMBER, &value); 
  }
}


// x^y
static void Level10 (RESULT *result)
{
  RESULT exponent = {0, 0.0, NULL};
  double value;

  Level11 (result);
  
  if (Type==T_DELIMITER && *Token == '^') {
    Parse();
    Level11 (&exponent);
    value = pow(R2N(result), R2N(&exponent));
    SetResult(&result, R_NUMBER, &value); 
  }
}


// unary + or - signs or logical 'not'
static void Level11 (RESULT *result)
{
  char sign=0;
  double value;
  
  if (Type==T_DELIMITER && (*Token=='+' || *Token=='-' || *Token=='!')) {
    sign=*Token;
    Parse();
  }

  Level12 (result);
  
  if (sign == '-') {
    value = -R2N(result);
    SetResult(&result, R_NUMBER, &value); 
  }
  else if (sign == '!') {
    value = (R2N(result)==0.0);
    SetResult(&result, R_NUMBER, &value); 
  }    
}


// literal numbers, variables, functions
static void Level12 (RESULT *result)
{
  RESULT *param[10];
  
  if (*Token == '(') {
    
    Parse();
    if (*Token == ')') ERROR (E_NOARG);
    Level01(result);
    if (*Token != ')') ERROR (E_UNBALAN);
    Parse();
    
  } else {
    
    if (Type == T_NUMBER) {
      double value=atof(Token);
      SetResult(&result, R_NUMBER, &value);
      Parse();
      
    } else if (Type == T_STRING) {
      SetResult(&result, R_STRING, Token);
      Parse();
      
    } else if (Type == T_NAME) {

      if (*Expression == '(') {
	FUNCTION *F=GetFunction(Token);
	if (F!=NULL) {
	  int n=0;
	  Parse(); // read opening brace
	  do { 
	    Parse(); // read argument
	    if (*Token == ',') ERROR (E_NOARG);
	    if (*Token == ')') {
	      // immediately closed when no args
	      if (F->args>0 || n>0) ERROR (E_NOARG);
	    } else {
	      param[n]=NewResult();
	      Level01(param[n]);
	      n++;
	    }
	  } while (n < 10 && *Token == ',');
	  Parse(); // read closing brace
	  if (F->args<0) {
	    // Function with variable argument list: 
	    // pass number of arguments as first parameter
	    F->func(result, n, &param); 
	  } else {
	    if (n != F->args) ERROR (E_NUMARGS);
	    F->func(result, 
		    param[0], param[1], param[2], param[3], param[4], 
		    param[5], param[6], param[7], param[8], param[9]);
	  }
	  // free parameter list
	  while(n-->0) {
	    FreeResult(param[n]);
	  }
	  
	  return;
	  
	} else {
	  ERROR(E_BADFUNC);
	}
	
      } else {
	if (!GetVariable(Token, result)) 
	  ERROR(E_UNKNOWN);
      }
      Parse();
      
    } else {
      ERROR(E_SYNTAX);
    }
  }
}


int Eval (char* expression, RESULT *result)
{
  int i, err;
  
  if ((err=setjmp(jb))) {
    error ("Evaluator: %s in expression <%s>", ErrMsg[err], expression);
    return -1;
  }
  
  // maybe sort function table
  if (!FunctionSorted) {
    FunctionSorted=1;
    qsort(Function, nFunction, sizeof(FUNCTION), f_sort);
    // sanity check: two functions with the same name?
    for (i=1; i<nFunction; i++) {
      if (strcmp(Function[i].name, Function[i-1].name)==0) {
	error ("Evaluator: internal error: Function '%s' defined twice!", Function[i].name);
      }
    }
  }
  
  Expression=expression;
  DelResult (result);
  Parse();
  if (*Token=='\0') ERROR (E_EMPTY);
  Level01(result);
  if (*Token!='\0') ERROR (E_SYNTAX);
  return 0;
}
