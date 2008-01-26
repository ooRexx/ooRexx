/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* REXX Macros                                                                */
/*                                                                            */
/* Header file for REXX methods written in C.                                 */
/*                                                                            */
/******************************************************************************/
#ifndef RexxNativeAPI_H_INCLUDED
#define RexxNativeAPI_H_INCLUDED

#include "rexx.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/* New-style macros and functions                                             */
/******************************************************************************/
#define ooRexxArray(s)           REXX_ARRAY_NEW(s)
#define ooRexxArray1(v1)         REXX_ARRAY_NEW1(v1)
#define ooRexxArray2(v1,v2)      REXX_ARRAY_NEW2(v1,v2)
#define ooRexxBuffer(l)          REXX_BUFFER_NEW(l)
#define ooRexxInteger(v)         REXX_INTEGER_NEW(v)
#define ooRexxPointer(v)         REXX_POINTER_NEW(v)
#define ooRexxReceiver()         REXX_RECEIVER()
#define ooRexxSend(r,n,aa)       REXX_SEND(r,n,aa)
#define ooRexxSend0(r,n)         REXX_SEND(r,n,ooRexxArray(0))
#define ooRexxSend1(r,n,a1)      REXX_SEND(r,n,ooRexxArray1(a1))
#define ooRexxSend2(r,n,a1,a2)   REXX_SEND(r,n,ooRexxArray2(a1,a2))
#define ooRexxString(s)          REXX_STRING_NEW(s, strlen(s))
#define ooRexxStringUpper(s)     REXX_STRING_NEW_UPPER(s)
#define ooRexxStringD(d)         REXX_STRING_NEWD(d)
#define ooRexxStringL(s,l)       REXX_STRING_NEW(s,l)
#define ooRexxSuper(n,aa)        REXX_SUPER(n,aa)
#define ooRexxSuper0(n)          REXX_SUPER(n,ooRexxArray(0))
#define ooRexxSuper1(n,a1)       REXX_SUPER(n,ooRexxArray1(a1))
#define ooRexxSuper2(n,a1,a2)    REXX_SUPER(n,ooRexxArray2(a1,a2))
#define ooRexxTable()            REXX_TABLE_NEW()
#define ooRexxVarSet(n,v)        REXX_SETVAR(n,v)
#define ooRexxVarValue(n)        REXX_GETVAR(n)
#define ooRexxNil                REXX_NIL()
#define ooRexxTrue               REXX_TRUE()
#define ooRexxFalse              REXX_FALSE()
#define ooRexxEnvironment        REXX_ENVIRONMENT()
#define ooRexxLocal              REXX_LOCAL()
#define ooRexxRaiseCondition(c, d, a, r) REXX_RAISE(c, d, a, r)

#ifndef __cplusplus
#define RexxMethod0(r,n) r  n##_m (void); \
static char n##_t[] = {REXXD_##r,0};    \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m());} return n##_t;} \
r  n##_m (void)

#define RexxMethod1(r,n,t1,p1) r  n##_m (t1 p1); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1))));} return n##_t;} \
r  n##_m (t1 p1)

#define RexxMethod2(r,n,t1,p1,t2,p2) r  n##_m (t1 p1, t2 p2); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2)

#define RexxMethod3(r,n,t1,p1,t2,p2,t3,p3) r  n##_m (t1 p1, t2 p2, t3 p3); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3)

#define RexxMethod4(r,n,t1,p1,t2,p2,t3,p3,t4,p4) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4)

#define RexxMethod5(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5)

#define RexxMethod6(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5,t6,p6) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,REXXD_##t6,0}; \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5)),*((t6 *)*(a+6))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6)

#else

#define RexxMethod0(r,n) r  n##_m (void); \
static char n##_t[] = {REXXD_##r,0};    \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m());} return n##_t;} \
r  n##_m (void)

#define RexxMethod1(r,n,t1,p1) r  n##_m (t1 p1); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1))));} return n##_t;} \
r  n##_m (t1 p1)

#define RexxMethod2(r,n,t1,p1,t2,p2) r  n##_m (t1 p1, t2 p2); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2)

#define RexxMethod3(r,n,t1,p1,t2,p2,t3,p3) r  n##_m (t1 p1, t2 p2, t3 p3); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3)

#define RexxMethod4(r,n,t1,p1,t2,p2,t3,p3,t4,p4) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4)

#define RexxMethod5(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5)

#define RexxMethod6(r,n,t1,p1,t2,p2,t3,p3,t4,p4,t5,p5,t6,p6) r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6); \
static char n##_t[] = {REXXD_##r,REXXD_##t1,REXXD_##t2,REXXD_##t3,REXXD_##t4,REXXD_##t5,REXXD_##t6,0}; \
extern "C" char * REXXENTRY n(void **a);           \
char * REXXENTRY n (void **a) {if (a != 0) {REXX_ret_##r(n##_m(*((t1 *)*(a+1)),*((t2 *)*(a+2)),*((t3 *)*(a+3)),*((t4 *)*(a+4)),*((t5 *)*(a+5)),*((t6 *)*(a+6))));} return n##_t;} \
r  n##_m (t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6)

#endif


#define _cstring(r)              REXX_STRING(r)
#define _double(r)               REXX_DOUBLE(r)
#define _integer(r)              REXX_INTEGER(r)
#define _unsigned_integer(r)     REXX_UNSIGNED_INTEGER(r)
#define _isdouble(r)             REXX_ISDOUBLE(r)
#define _isdirectory(r)          REXX_ISDIRECTORY(r)
#define _isinteger(r)            REXX_ISINTEGER(r)
#define _isstring(r)             REXX_ISSTRING(r)
#define _string(r)               REXX_OBJECT_STRING(r)

#define area_get(r,s,b,l)        REXX_BUFFER_GETDATA(r,s,(CSTRING)b,l)
#define area_put(r,s,b,l)        REXX_BUFFER_COPYDATA(r,s,(CSTRING)b,l)

#define array_at(r,i)            REXX_ARRAY_AT(r,i)
#define array_hasindex(r,i)      REXX_ARRAY_HASINDEX(r,i)
#define array_put(r,v,i)         REXX_ARRAY_PUT(r,v,i)
#define array_size(r)            REXX_ARRAY_SIZE(r)

#define buffer_address(r)        REXX_BUFFER_ADDRESS(r)
#define buffer_length(r)         REXX_BUFFER_LENGTH(r)
#define buffer_extend(r, l)      REXX_BUFFER_EXTEND(r, l)
#define buffer_get(r,s,b,l)      REXX_BUFFER_GETDATA(r,s,(CSTRING)b,l)
#define buffer_put(r,s,b,l)      REXX_BUFFER_COPYDATA(r,s,(CSTRING)b,l)

#define integer_value(r)         REXX_INTEGER_VALUE(r)
#define pointer_value(r)         REXX_POINTER_VALUE(r)

#define string_get(r,s,b,l)      REXX_STRING_GET(r,s,b,l)
#define string_length(r)         REXX_STRING_LENGTH(r)
#define string_put(r,s,b,l)      REXX_STRING_PUT(r,s,b,l)
#define string_rput(r,s,b,l)     REXX_STRING_RPUT(r,s,b,l)
#define string_set(r,s,c,n)      REXX_STRING_SET(r,s,c,n)
#define string_data(r)           REXX_STRING_DATA(r)

#define table_add(r,v,i)         REXX_TABLE_ADD(r,v,i)
#define table_at(r,i)            REXX_TABLE_GET(r,i)
#define table_remove(r,i)        REXX_TABLE_REMOVE(r,i)

#define rexx_condition(c,d,a)    REXX_CONDITION(c,d,a)
#define rexx_exception(m)        REXX_EXCEPT(m, NULLOBJECT)
#define rexx_exception_with(m, a) REXX_EXCEPT(m, a)
#define rexx_exception1(m, v1)   REXX_EXCEPT1(m, v1)
#define rexx_exception2(m, v1, v2)  REXX_EXCEPT2(m, v1, v2)



/******************************************************************************/
/* Old-style macros and internal definitions follow..... don't use!!          */
/******************************************************************************/

/******************************************************************************/
/* Types (used in macro expansions and function prototypes)                   */
/******************************************************************************/
#define ONULL   REXXOBJECT
#define OSELF   REXXOBJECT
#define CSELF   void *
#define BUFFER  void *
#define ARGLIST REXXOBJECT
#define SCOPE   REXXOBJECT
#define MSGNAME REXXOBJECT

/******************************************************************************/
/* Primitive Message Names (for general use except where indicated)           */
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


REXXOBJECT REXXENTRY REXX_ARRAY_AT(REXXOBJECT, size_t);
bool       REXXENTRY REXX_ARRAY_HASINDEX(REXXOBJECT, size_t);
void       REXXENTRY REXX_ARRAY_PUT(REXXOBJECT, REXXOBJECT, size_t);
size_t     REXXENTRY REXX_ARRAY_SIZE(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW(size_t);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW1(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_ARRAY_NEW2(REXXOBJECT, REXXOBJECT);

char *     REXXENTRY REXX_BUFFER_ADDRESS(REXXOBJECT);
size_t     REXXENTRY REXX_BUFFER_LENGTH(REXXOBJECT);
size_t     REXXENTRY REXX_BUFFER_GETDATA(REXXOBJECT, size_t, void *, size_t);
size_t     REXXENTRY REXX_BUFFER_COPYDATA(REXXOBJECT, size_t, void *, size_t);
REXXOBJECT REXXENTRY REXX_BUFFER_NEW(size_t);
REXXOBJECT REXXENTRY REXX_BUFFER_EXTEND(REXXOBJECT, size_t);

wholenumber_t REXXENTRY REXX_INTEGER_VALUE(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_INTEGER_NEW(wholenumber_t);

void      *REXXENTRY REXX_POINTER_VALUE(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_POINTER_NEW(void *);

double     REXXENTRY REXX_DOUBLE(REXXOBJECT);
bool       REXXENTRY REXX_ISDOUBLE(REXXOBJECT);
bool       REXXENTRY REXX_ISSTRING(REXXOBJECT);
bool       REXXENTRY REXX_ISDIRECTORY(REXXOBJECT);
bool       REXXENTRY REXX_ISINSTANCE(REXXOBJECT, REXXOBJECT);
void       REXXENTRY REXX_EXCEPT(int, REXXOBJECT);
void       REXXENTRY REXX_EXCEPT1(int, REXXOBJECT);
void       REXXENTRY REXX_EXCEPT2(int, REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_CONDITION(REXXOBJECT, REXXOBJECT, REXXOBJECT);
void       REXXENTRY REXX_RAISE(CSTRING, REXXOBJECT, REXXOBJECT, REXXOBJECT);
wholenumber_t REXXENTRY REXX_INTEGER(REXXOBJECT);
size_t     REXXENTRY REXX_UNSIGNED_INTEGER(REXXOBJECT);
bool       REXXENTRY REXX_ISINTEGER(REXXOBJECT);
CSTRING    REXXENTRY REXX_STRING(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_MSGNAME(void);
REXXOBJECT REXXENTRY REXX_RECEIVER(void);
bool       REXXENTRY REXX_PTYPE(REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SEND(REXXOBJECT, CSTRING, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_DISPATCH(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SUPER(CSTRING, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_SETVAR(CSTRING, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_GETFUNCTIONNAMES(char ***, size_t*);
REXXOBJECT REXXENTRY REXX_GETVAR(CSTRING);
int        REXXENTRY REXX_VARIABLEPOOL(void *);
int        REXXENTRY REXX_STEMSORT(CSTRING, int, int, size_t, size_t, size_t, size_t);
void       REXXENTRY REXX_PUSH_ENVIRONMENT(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_POP_ENVIRONMENT(void);
REXXOBJECT REXXENTRY REXX_NIL(void);
REXXOBJECT REXXENTRY REXX_TRUE(void);
REXXOBJECT REXXENTRY REXX_FALSE(void);
REXXOBJECT REXXENTRY REXX_ENVIRONMENT(void);
REXXOBJECT REXXENTRY REXX_LOCAL(void);
void       REXXENTRY REXX_GUARD_ON(void);
void       REXXENTRY REXX_GUARD_OFF(void);


REXXOBJECT REXXENTRY REXX_OBJECT_METHOD(REXXOBJECT, REXXOBJECT);

REXXOBJECT REXXENTRY REXX_OBJECT_NEW(REXXOBJECT);

size_t     REXXENTRY REXX_STRING_GET(REXXOBJECT, size_t, char *, size_t);
void       REXXENTRY REXX_STRING_PUT(REXXOBJECT, size_t, CSTRING, size_t);
void       REXXENTRY REXX_STRING_SET(REXXOBJECT, size_t, char, size_t);
size_t     REXXENTRY REXX_STRING_LENGTH(REXXOBJECT);
CSTRING    REXXENTRY REXX_STRING_DATA(REXXOBJECT);
REXXOBJECT REXXENTRY REXX_STRING_NEW(CSTRING, size_t);
REXXOBJECT REXXENTRY REXX_STRING_NEW_UPPER(CSTRING);
REXXOBJECT REXXENTRY REXX_STRING_NEWD(double);

REXXOBJECT REXXENTRY REXX_TABLE_ADD(REXXOBJECT, REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_TABLE_GET(REXXOBJECT, REXXOBJECT);
REXXOBJECT REXXENTRY REXX_TABLE_REMOVE(REXXOBJECT, REXXOBJECT);

int        REXXENTRY RexxQuery (void);
int        REXXENTRY RexxTerminate (void);
int        REXXENTRY RexxInitialize (void);

#ifdef __cplusplus
}
#endif

/******************************************************************************/
/* Interface Datatypes (used in macro expansions)                             */
/******************************************************************************/
#define REXXD_void          1
#define REXXD_OBJECT        2
#define REXXD_REXXOBJECT    REXXD_OBJECT
#define REXXD_int           3
#define REXXD_size_t        4
#define REXXD_ssize_t       5
#define REXXD_stringsize_t  6
#define REXXD_wholenumber_t 7
#define REXXD_double        8
#define REXXD_CSTRING       9
#define REXXD_OSELF         10
#define REXXD_ARGLIST       11
#define REXXD_MSGNAME       12
#define REXXD_SCOPE         13
#define REXXD_POINTER       14
#define REXXD_CSELF         15
#define REXXD_STRING        16
#define REXXD_REXXSTRING    REXXD_STRING
#define REXXD_BUFFER        17

/******************************************************************************/
/* Internal Macros (used in macro expansions)                                 */
/******************************************************************************/
#define REXX_ret_void(v)           v
#define REXX_ret_REXXOBJECT(v)    *((REXXOBJECT *)*a) = v
#define REXX_ret_REXXSTRING(v)    *((REXXOBJECT *)*a) = v
#define REXX_ret_int(v)           *((int *)*a) = v
#define REXX_ret_size_t(v)        *((size_t *)*a) = v
#define REXX_ret_ssize_t(v)       *((ssize_t *)*a) = v
#define REXX_ret_stringsize_t(v)  *((stringsize_t *)*a) = v
#define REXX_ret_wholenumber_t(v) *((wholenumber_t *)*a) = v
#define REXX_ret_double(v)        *((double *)*a) = v
#define REXX_ret_CSTRING(v)       *((CSTRING *)*a) = v
#define REXX_ret_POINTER(v)       *((void  **)*a) = v
#ifdef __cplusplus
}
#endif

#endif
