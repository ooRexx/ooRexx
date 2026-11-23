/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/* Object REXX Support                                         automaton.cpp  */
/*                                                                            */
/* Regular Expression Utility functions                                       */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "automaton.hpp"
#include "regexp.hpp"
//#define MYDEBUG

// constructor: initialize automaton
automaton::automaton() : ch(NULL), next1(NULL), next2(NULL), final(-1), regexp(NULL),
                         setArray(NULL), setSize(0), size(16), freeState(1), currentPos(0), minimal(false)
{
    int bytes = sizeof(int)*size;

    ch    = (int*) malloc(bytes);
    next1 = (int*) malloc(bytes);
    next2 = (int*) malloc(bytes);
}

// destructor: free memory
automaton::~automaton()
{
    if (size)
    {
        free(ch);
        free(next1);
        free(next2);
    }
    if (setSize)
    {
        for (int i=0;i<setSize;i++)
        {
            free(setArray[i]);
        }
        free(setArray);
    }

}


/*************************************************************/
/* automaton::setMinimal                                     */
/*                                                           */
/* make the automaton match minimal or maximal               */
/* this influences the state machine.                        */
/*************************************************************/
void automaton::setMinimal(bool f)
{
    if (f != this->minimal)
    {
        if (this->final != -1)
        {
            if (f == false)
            {
                setState(this->final, 0x00, this->final+1, this->final+1);
            }
            else
            {
                setState(this->final, EPSILON, EOP, EOP);
            }
        }
        this->minimal = f;
    }
}


/*************************************************************/
/* automaton::parse                                          */
/*                                                           */
/* creates the automaton from the regular expression. if an  */
/* error occurs during parsing, the error number is returned */
/* and the automaton will match any pattern.                 */
/* parsing is recursive.                                     */
/*************************************************************/

/*
We parse a regular expression pattern to create a finite-state
machine (FSM) that later match() can use to efficiently determine
whether a string matches the pattern or not.

The FSM's main part are three arrays, where the array index is the
state number.
  ch[]    holds one of the following:
          a specific character which this state must consume
          EPSILON, indicating that this state doesn't consume a character
          ANY, indicating that this state consumes any character
          SET, indicating that this state consumes a character from a set
  next1[] the state to transition to for the next step
  next2[] if next2[] is not equal to next1[], it specifies an alternative
          state to transition to

  freeState keeps track of the first yet unused state.  
  setState() is used to set a new or modify an existing state.


Parsing is done in a single pass.

Here's an example.  After parsing the pattern "ABC", the FSM will look
like this:
state   ch      next1   next2
0       eps     1
1       A       2
2       B       3
3       C       4

Starting in state 0, we transition to state 1 without consuming anything.
In state 1 we consume "A" and transition to state 2.
In state 2 we consume "B" and transition to state 3.
In state 3 we consume the final "C", and transition to the terminal state 4.

Every part of the FSM that the parser builds (an expression, a term, a
factor) follows the same rule: its states are contiguous, it is entered
through its first state, and it is left by going to the state that follows
its last one.  A part can then simply be followed by another one, with no
need to patch the states already built.

Now let's see what happens if we change the pattern to e. g. "ABC*".
Having parsed "ABC", the FSM is as shown above.
When the parser sees the asterisk, it will modify the FSM as follows
(see factor() function, block regexp[currentPos] == '*'): the states of
the atom "C" move up by one (insertStateAt), and two epsilon states are
added: one before the atom, the factor's entry, that goes to a loop state
after it; and the loop state, that either leaves the factor (to state 6)
or matches the atom once more (state 4):
state   ch      next1   next2
0       eps     1
1       A       2
2       B       3
3       eps     5
4       C       5
5       eps     6       4

Following the state transitions, we find that this will - as expected -
allow AB, ABC, ABCC, ABCCC.. to be successfully matched.

An "|" is built in the same way: for "AB|C" the states of the term "AB"
move up by one, to make room for an entry state that goes to either term,
and an epsilon state after "AB" jumps over the "C" to the end:
state   ch      next1   next2
0       eps     1
1       eps     2       5
2       A       3
3       B       4
4       eps     6
5       C       6

Parsing is done (roughly) according to this regular expression BNF:

   expression ::= term | term "|" expression

   term ::= factor | factor term

   factor ::= atom | atom metacharacter

   atom ::= "?" | "[" set "]" | "(" expression ")" | character 

   metacharacter ::= "*" | "+" | "{" times "}"

*/
int automaton::parse(const char *regexp)
{
    int temp;
    this->regexp = regexp;
    currentPos = 0;
    freeState  = 1;
    memset(ch,    0x00, sizeof(int)*size);
    memset(next1, 0x00, sizeof(int)*size);
    memset(next2, 0x00, sizeof(int)*size);
    if (setSize)
    {
        for (int i=0;i<setSize;i++)
        {
            free(setArray[i]);
        }
        free(setArray);
        setSize = 0;
        setArray = NULL;
    }

    try
    {
        // parse expression recursively by splitting it up
        temp = expression();
        // using a temporary place to store the start state
        // is needed since next1 may change during reallocation
        next1[0] = temp;
    }
    catch (RE_ERROR err)
    {
        this->regexp = NULL;
        // an error occured!
        setState(0, EPSILON, 0, 0);  // make automaton match anything
        return(int) err;
    }
    // set start state
    setState(0, EPSILON, next1[0], next1[0]);
    this->final = freeState;
    if (minimal == false)
    {
        // zero-terminate the expression...
        setState(freeState, 0x00, freeState+1, freeState+1);
        freeState++;
    }
    else
    {
        // this ends the pattern, but we need a dummy
        // show the regexp be switched between min and max matching
        setState(freeState, EPSILON, EOP, EOP);
        freeState++;
    }
    // ...and set epsilon transition to end state
    setState(freeState, EPSILON, EOP, EOP);

// in debug mode, print out the automaton
#ifdef MYDEBUG
    printf("state\tch\tnext1\tnext2\n");
    for (int i=0;i<size && next1[i] != EOP;i++)
    {
        if ((unsigned int) next1[i] < 32000)
        {
            printf("%d\t",i);
            switch (ch[i] & SCAN)
            {
                case ANY:
                    printf("any\t");
                    break;
                case SET:
                    printf("set %d\t",(ch[i] & 0x0fff0000) >> 16);
                    break;
                case SET|NOT:
                    printf("nset %d\t",(ch[i] & 0x0fff0000) >> 16);
                    break;
                case EPSILON:
                    printf("eps\t");
                    break;
                default:
                    printf("%c\t",ch[i]);
                    break;
            }
            if (next1[i] != next2[i])
            {
                printf("%d\t%d\n",next1[i],next2[i]);
            }
            else
            {
                printf("%d\n",next1[i]);
            }
        }
    }
#endif

    this->regexp = NULL;  // contents only guaranteed during
                          // runtime of this method
    return 0;
}

/***********************************************************/
/* automaton::letter                                       */
/*                                                         */
/* check if the given character is a letter or a operator. */
/***********************************************************/
int automaton::letter(int c)
{
  int r;

  switch (c) {
  case '(':
  case ')':
  case '{':
  case '}':
  case '|':
  case '*':
  case '+':
  case '[':
  case ']':
  //case '?':
  case 0x00:
    r=0;
    break;
  default:
    r=1;
  }
  return r;
}

/**************************************************************/
/* automaton::expression                                      */
/*                                                            */
/* expression ::= term | term "|" expression                  */
/*                                                            */
/* dissect expression into smaller parts (expression => term) */
/* and insert fitting states into the automaton.              */
/**************************************************************/
int automaton::expression()
{
    // every part of the automaton (expression, term, factor) is built the
    // same way: its states are contiguous, it is entered through its first
    // state, and it is left by going to the state that follows its last
    // one.  So a part needs no patching when another one follows it.
    int start = freeState;

    term();
    // is the current character an 'or' operator?
    if (regexp[currentPos] == '|')
    {
        currentPos++;
        // the left term moves up one state, to make room for the entry
        // state, which goes to either term
        insertStateAt(start);
        // after the left term, jump over the right expression
        int jump = freeState++;
        int right = freeState;
        expression();
        setState(start, EPSILON, start + 1, right);
        setState(jump, EPSILON, freeState, freeState);
    }
    return start;
}

/*********************************/
/* automaton::term               */
/*                               */
/* term ::= factor | factor term */
/*                               */
/* parse a term.                 */
/*********************************/
int automaton::term()
{
    int start = freeState;

    factor();

    // another factor following?  each factor is entered through its first
    // state, which is the state the previous one goes to when it is done
    while ( (regexp[currentPos] == '(') ||
            (regexp[currentPos] == '[') ||
            letter(regexp[currentPos]))
    {
        factor();
    }

    return start;
}

/***************************************************************/
/* automaton::factor                                           */
/*                                                             */
/* factor ::= atom | atom metacharacter                        */
/*                                                             */
/* atom ::= "?" | "[" set "]" | "(" expression ")" | character */
/*                                                             */
/* metacharacter ::= "*" | "+" | "{" times "}"                 */
/*                                                             */
/* create new states for the automaton according to meaning of */
/* single characters.                                          */
/* known operators are '?', '\', '[', '(', '{', '*', '+'       */
/*                                                             */
/* returns the start state for this factor                     */
/***************************************************************/
int automaton::factor()
{
    int t1;

    t1 = freeState;
    switch (regexp[currentPos])
    {
        // match any single character
        case '?':
            // consume any character and transition to the following state
            setState(freeState, ANY, freeState+1, freeState+1);
            freeState++;
            currentPos++;
            break;
            // escape operator, use next character literally
        case '\\':
            currentPos++;
            if (regexp[currentPos] == 0x00) throw E_UNEXPECTED_EOP;
            // consume the current character and transition to the following state
            setState(freeState, regexp[currentPos], freeState+1, freeState+1);
            freeState++;
            currentPos++;
            break;
            // set definition
        case '[':
            currentPos++;
            set();
            if (regexp[currentPos] == ']') currentPos++;
            else throw E_ILLEGAL_SET;
            break;
            // an expression in parenthesis
        case '(':
            currentPos++;              // step over "("
            // an expression is entered through its first state and left
            // by going to the state that follows it, as any factor
            expression();
            if (regexp[currentPos] == ')') currentPos++;
            else throw E_MISSING_PAREN_CLOSE;
            break;
        default:
            // only a letter is valid at this position
            if (letter(regexp[currentPos]))
            {
                // consume the letter and transition to the following state
                setState(freeState, regexp[currentPos], freeState+1, freeState+1);
                freeState++;
                currentPos++;
            }
            else
            {
                throw E_UNEXPECTED_SYMBOL;
            }
    }

    // the next statements are left out of the switch-block because
    // they are all postfix operators.
    // if they are used without operands, E_UNEXPECTED_SYMBOL is
    // caused in the switch-block

    // match previous expression zero or more times
    if (regexp[currentPos] == '*')
    {
        // the atom moves up one state, to make room for the entry state,
        // which goes to the loop state that follows the atom
        insertStateAt(t1);
        int loop = freeState++;
        // the loop state leaves, or matches the atom once more
        setState(loop, EPSILON, freeState, t1 + 1);
        setState(t1, EPSILON, loop, loop);
        currentPos++;
    }
    // match previous expression one or more times
    else if (regexp[currentPos] == '+')
    {
        // after the atom, match it once more or leave
        setState(freeState, EPSILON, t1, freeState+1);
        freeState++;
        currentPos++;
    }
    // match previous expression exactly n times
    else if (regexp[currentPos] == '{')
    {
        char stringBuffer[64];
        int tempPos = currentPos+1;
        int i = 0;
        int j,k, size;

        // get n
        while (regexp[tempPos] && regexp[tempPos] != '}')
        {
            stringBuffer[i] = regexp[tempPos];
            tempPos++;
            if (i<62) i++;
        }

        if (regexp[tempPos] == 0x00)
        {
            throw E_UNEXPECTED_EOP;
        }
        stringBuffer[i] = 0x00;
        i = atoi(stringBuffer) - 1;
        if (i <= 0)
        {
           throw E_ILLEGAL_NUMBER;
        }

        // the previous expression will be copied and insert
        // n times into the automaton: each copy is entered from the end of
        // the one before it, and goes to the state that follows it
        size = freeState - t1;
        while (i--)
        {
            k = freeState;
            for (j=t1;j<k;j++,freeState++)
            {
                setState(freeState, ch[j], next1[j]+size, next2[j]+size);
            }
            t1 += size;
        }

        currentPos = tempPos + 1;
    }
    return t1;
}


/*******************************************************************/
/* automaton::insertStateAt                                        */
/*                                                                 */
/* make room for one state at position, moving the states from it  */
/* to the last one up by one.  The states moved are those of the   */
/* part being built, which only go to states inside it or to the   */
/* one that follows it: those move up by one too.  The states      */
/* before position that go to it now go to the new state there.    */
/*******************************************************************/
void automaton::insertStateAt(int position)
{
    for (int i = freeState - 1; i >= position; i--)
    {
        setState(i + 1, ch[i],
                 next1[i] >= position ? next1[i] + 1 : next1[i],
                 next2[i] >= position ? next2[i] + 1 : next2[i]);
    }
    freeState++;
}

/**********************************************************/
/* automaton::set                                         */
/*                                                        */
/* process a set definition.                              */
/* this parses the the definition and creates a character */
/* array that contains all specified characters.          */
/* if this is an exclusive match, the NOT bit of the SET  */
/* transition will be set.                                */
/**********************************************************/
int automaton::set()
{
  int i=0;
  int transition = SET;
  int length = 257;                 // pre-allocation length is 257 bytes
                                    // if more are need, reallocation is used
  const char *ptr = regexp+currentPos;
  char *range = (char*) malloc(sizeof(char)*length);
#ifdef WIN32
  int (__cdecl *func)(int) = NULL;     // function pointer for symbolic names
#else
  int (*func)(int) = NULL;
#endif
  // is this an exclusive definition?
  if (*ptr == '^') {
    transition |= NOT;
    ptr++;
  }

  // parse the set definition
  while (*ptr && *ptr != ']') {
    // '-' operator: go from last specified character
    // to next specified character (fill in all in-between)
    // (a '-' at the beginning is interpreted as a simple character)
    if (*ptr == '-' && i) {
      // the range is of byte values, 00 to FF: with a signed char, a
      // range ending at 7F never ended and one above 7F was the wrong range
      unsigned char b=(unsigned char)ptr[-1], e=(unsigned char)ptr[1];
      if (e == 0x00 || e == ']') throw E_ILLEGAL_SET;
      ptr+=2;
      // the start character is already in the set; the end one is not
      int first=b+1;
      // start character > end character? then swap them, and the new start
      // (the end character) must be filled in too
      if (b>e) {
        unsigned char t=b;
        b=e; e=t;
        first=b;
      }
      // fill up the range
      for (int c=first;c<=e;c++) {
        if (checkRange(range,i,(char)c)) range[i++] = (char)c;
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
      }
    }
    // escape operator
    else if (*ptr == '\\') {
      ptr++;
      if (*ptr == 0x00) throw E_ILLEGAL_SET;
      if (checkRange(range,i,*ptr)) range[i++] = *ptr;
      if (i == length) {
        range = (char*) realloc(range,length*2);
        length *= 2;
      }
      ptr++;
    }
    // symbolic name?
    else if (*ptr == ':') {
      int len=0;
      char buffer[16];
      ptr++;
      while (len<15 && *ptr && *ptr != ':')
        buffer[len++] = *ptr++;
      buffer[len] = 0x00;
      if (*ptr != ':') throw E_ILLEGAL_SYMBOLIC_NAME;
      ptr++;
      // make symbolic name uppercase
      while (len>=0) {
        buffer[len] = toupper((int) buffer[len]);
        len--;
      }
      // keyword ALPHA
      if (!strcmp(buffer,"ALPHA"))
        func = isalpha;
      // keyword LOWER
      else if (!strcmp(buffer,"LOWER"))
        func = islower;
      // keyword UPPER
      else if (!strcmp(buffer,"UPPER"))
        func = isupper;
      // keyword ALNUM
      else if (!strcmp(buffer,"ALNUM"))
        func = isalnum;
      // keyword DIGIT
      else if (!strcmp(buffer,"DIGIT"))
        func = isdigit;
      // keyword XDIGIT
      else if (!strcmp(buffer,"XDIGIT"))
        func = isxdigit;
      // keyword BLANK
      else if (!strcmp(buffer,"BLANK")) {
        // whitespace...
        if (checkRange(range,i,' ')) range[i++] = ' ';
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
        // ...or tab
        if (checkRange(range,i,0x09)) range[i++] = 0x09;
        if (i == length) {
          range = (char*) realloc(range,length*2);
          length *= 2;
        }
      }
      // keyword SPACE
      else if (!strcmp(buffer,"SPACE"))
        func = isspace;
      // keyword CNTRL
      else if (!strcmp(buffer,"CNTRL"))
        func = iscntrl;
      // keyword PRINT
      else if (!strcmp(buffer,"PRINT"))
        func = isprint;
      // keyword PUNCT
      else if (!strcmp(buffer,"PUNCT"))
        func = ispunct;
      // keyword GRAPH
      else if (!strcmp(buffer,"GRAPH"))
        func = isgraph;
      // unknown symbolic name?
      else
        throw E_ILLEGAL_SYMBOLIC_NAME;

      if (func) {
        int j;
        // go over all ASCII characters and use the specified
        // function...
        for (j=0;j<256;j++)
          if ( (*func)(j) ) {
            if (checkRange(range,i,j)) range[i++] = j;
            if (i == length) {
              range = (char*) realloc(range,length*2);
              length *= 2;
            }
          }
      }

    }
    // simple character
    else {
      if (checkRange(range,i,*ptr)) range[i++] = *ptr;
      ptr++;
      if (i == length) {
        range = (char*) realloc(range,length*2);
        length *= 2;
      }
    }
  }

  // empty sets are not allowed
  if (i == 0) throw E_ILLEGAL_SET;
  else currentPos+=(int)(ptr-(regexp+currentPos));

  // set SET transition (bits 16 to 27 contain the set number)
  // that is created by insertSet
  // consume a character from/not from the set and transition to the following state
  setState(freeState, transition | (insertSet(range, i)<<16), freeState+1, freeState+1);
  i = freeState;
  freeState++;

  free(range);

  return i;
}

/******************************************************************/
/* automaton::setState                                            */
/*                                                                */
/* insert a state into the automaton.                             */
/* if the given position is too large for the currently allocated */
/* arrays, reallocation will occur.                               */
/******************************************************************/
void automaton::setState(int position, int transition, int state1, int state2)
{
  // insert an element not yet settable?
  // yes, then get enough memory for that
  while (size <= position) {
    size<<=1;

    ch    = (int*) realloc(ch,size*sizeof(int));
    next1 = (int*) realloc(next1,size*sizeof(int));
    next2 = (int*) realloc(next2,size*sizeof(int));
  }
  ch[position]    = transition;
  next1[position] = state1;
  next2[position] = state2;
}

/************************************************************/
/* automaton::checkRange                                    */
/*                                                          */
/* small helper function to check if a character is already */
/* in the range (avoid multiple entries).                   */
/* returns 1 if character not in range, zero otherwise.     */
/************************************************************/
int automaton::checkRange(char *range, int length, char c)
{
  int rc = 1;
  for (int i=0;i<length;i++)
    if (range[i] == c) {
      rc = 0;
      break;
    }
  return rc;
}

/***************************************************************/
/* automaton::insertSet                                        */
/*                                                             */
/* create a set and return the set number.                     */
/* the set is an array of characters, the first element of the */
/* set specifies the number of elements in the array.          */
/* the setArray containing pointers to the sets will be re-    */
/* allocated on each call. it is assumed that performance is   */
/* secondary because parsing takes place only once and usually */
/* there are only a few set definitions at all.                */
/***************************************************************/
int automaton::insertSet(char *range, int rangeSize)
{
  setSize++;
  // enlarge setArray
  setArray = (int**) realloc(setArray,setSize*sizeof(int*));
  // get memory for set array
  setArray[setSize-1] = (int*) malloc((rangeSize+1)*sizeof(int));

  // fill in elements
  for ( int i=0; i<rangeSize; i++)
    setArray[setSize-1][i+1] = (int) range[i];

  setArray[setSize-1][0] = rangeSize;  // set length
  return setSize-1;
}

/*************************************************/
/* automaton::match                              */
/*                                               */
/* try to match a string with the automaton.     */
/* returns 1 on success and 0 on failure.        */
/* matching is non-recursively done with a queue */
/* that has a push, put and pop method.          */
/*************************************************/
/*************************************************************/
/* queueNext                                                 */
/*                                                           */
/* queue a state for the next position in the string, unless */
/* it is already queued for it.                              */
/*************************************************************/
static void queueNext(doubleQueue &dq, int *queuedNext, int state, int position)
{
  if (queuedNext[state] != position + 1) {
    queuedNext[state] = position + 1;
    dq.put(state);
  }
}

int automaton::match(const char *a, int N)  // string length passed in
                                      // instead of strlen
{
  int n1, n2;
  int j = 0;
  int state = next1[0];  // get start state
  doubleQueue dq(64);    // create a double queue
                         // one SCAN symbol will be put into the queue
  int i;
  int set;
  int len;
  bool found;
  // a state is queued at most once for each position in the string: an
  // automaton may have a cycle of epsilon transitions (as in "(a*)*"), and
  // following one again and again never ended.  queuedNow[s] is the position
  // s was last queued for with push (this position), queuedNext[s] the one
  // it was last queued for with put (the next position).
  int states = freeState + 1;
  int *queuedNow = (int*) malloc(sizeof(int)*states);
  int *queuedNext = (int*) malloc(sizeof(int)*states);
  for (i=0; i<states; i++) {
    queuedNow[i] = -1;
    queuedNext[i] = -1;
  }

  // terminates when end state (==0) is reached
  while (state) {
#ifdef MYDEBUG
    printf("          ");
    dq.dump();
#endif
    // go to next character
    if (state == SCAN) {
      if (minimal == true && j == N) break; // for minimal match
#ifdef MYDEBUG
      printf("consume %2d %c\n",j,a[j]);
#endif
      j++;
      dq.put(SCAN);
    }
    else
    switch (ch[state] & SCAN) {
    case EPSILON:        // epsilon transition
      n1 = next1[state];
      n2 = next2[state];
      if (queuedNow[n1] != j) {
        queuedNow[n1] = j;
        dq.push(n1);
      }
      if (queuedNow[n2] != j) {
        queuedNow[n2] = j;
        dq.push(n2);
      }
      break;
    case SET:            // inclusive set
    case SET|NOT:        // exclusive set
      set = (ch[state] & 0x0fff0000)>>16;   // get set number
      len = setArray[set][0];               // get number of elements in set
      found = (ch[state]&NOT)?true:false;   // set default value

      for (i=1; i<=len; i++) {
        if (setArray[set][i] == (int) a[j]) {
          found = !found;
          break;
        }
      }
      if (found) {
        queueNext(dq, queuedNext, next1[state], j);
      }
      break;
    case ANY:            // just match any character
      queueNext(dq, queuedNext, next1[state], j);
      break;
    default:
      if (j < N) { // freeState: we can't read past end of string
        // normal character?
        if (ch[state] == (int) a[j]) {
          queueNext(dq, queuedNext, next1[state], j);
        }
      } else if (j == N) {  // simulate zero-terminated string in any case
        if (ch[state] == (int) 0) {
          queueNext(dq, queuedNext, next1[state], j);
        }
      }
      break;
    }
#ifdef MYDEBUG
    printf("%s",dq.isEmpty()?"    empty ":"NOT empty ");
    dq.dump();
#endif
    // break out if we've reached the end of the string
    // or no more states are available
    if (dq.isEmpty() || (j>(N+1)) ) break;
    state = dq.pop();
  }

  free(queuedNow);
  free(queuedNext);

  currentPos = j;

  if (currentPos > N) currentPos = N;

  // return 1 if end state (EOP) has been reached
  return state==EOP?1:0;
}
