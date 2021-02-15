#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/*  guess.rex           Open Object Rexx Samples                              */
/*                                                                            */
/*  An animal guessing game                                                   */
/*                                                                            */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This samples creates a simple node class and uses it to create a logic    */
/*  tree.  The logic tree is filled in by playing a simple guessing game.     */
/******************************************************************************/
topic = 'animal'

firstNode = .node~new                       /* Create a new instance of node  */
                                            /* to start our tree.             */
firstNode~setGuess('bear')

say 'Think of' article(topic) 'for me to guess!'

do until userQuits
  firstNode~interact                        /* Invoke the interact method on  */
                                            /* the node class to handle user  */
                                            /* interaction.                   */
  userQuits = \prompt('Wanna try another' topic'?')
end

say ''
say 'Here is the decision tree I built:'
firstNode~dump(1)

say 'Thanks for playing!'
return

/* An example of using the ::routine directive to define a subroutine used by */
/* several of the following methods.                                          */
::routine prompt
  use arg question
  say question '(Y/N)'
  pull reply
  return reply = 'Y'

/* An example of using the ::routine directive to define a subroutine used by */
/* several of the following methods.                                          */
::routine article
  use arg noun
  if noun~caselessMatchchar(1, 'AEIOU') then
     return 'an' noun
  else
     return 'a' noun

/* Define a new class 'node' and make it visible outside of this program      */
::class node public

/* Define the init method on the node class.  This method initializes the     */
/* state data for an instance of the node class.                              */
::method init
  expose yesNode noNode myGuess myQuestion
  yesNode = .nil
  noNode = .nil
  myGuess = .nil
  myQuestion = .nil
  return self

/* Define the dump method on the node class.  This method prints out the      */
/* decision tree built by the program.                                        */
::method dump
  expose yesNode noNode myGuess myQuestion
  use arg tab
  if myGuess = .nil then do
    yesNode~dump(tab + 5)
    say left('',tab) myQuestion
    noNode~dump(tab + 5)
  end
  else do
    say left('',tab) '<'myGuess'>'
  end

/* Define the setGuess method on the node class.  This method gets the next   */
/* guess from the user.                                                       */
::method setGuess
  expose myGuess
  use arg myGuess

/* Define the interact method on the node class.  This method handles the     */
/* interaction between the program and the user.  Note the use of 'self' to   */
/* execute methods on this instance of node.                                  */
::method interact
  expose myGuess
  if myGuess = .nil then
    self~askQuestion
  else
    self~guess

/* Define the guess method on the node class.  This method tries to guess the */
/* user's article and asks the user for more information if it fails.         */
::method guess
  expose myGuess
  say "I bet you're thinking of" article(myGuess)'.'
  if prompt('Right?') then
    say 'Cool!  I guessed your secret!'
  else
    self~learnNew

/* Define the learnNew method on the node class.  This method gets additional */
/* information about the user's article.                                      */
::method learnNew
  expose myGuess myQuestion yesNode noNode
  say 'What were you thinking of?'
  parse pull newGuess
  say 'Oops!  I never heard of that.'
  say 'Please enter a question with a yes or no answer that would help me'
  say 'tell' article(myGuess) 'from' article(newGuess)'.'
  parse pull myQuestion
  yesNode = .node~new
  noNode = .node~new
  if prompt('Is the answer yes for' article(newGuess)'?') then do
    yesNode~setGuess(newGuess)
    noNode~setGuess(myGuess)
  end
  else do
    yesNode~setGuess(myGuess)
    noNode~setGuess(newGuess)
  end
  say "Thanks, I'll remember that for next time!"
  myGuess = .nil
  return

/* Define the askQuestion method on the node class.  This method branches     */
/* down either the yes or no fork based on the user's response.               */
::method askQuestion
  expose myQuestion yesNode noNode
  if prompt(myQuestion) then
    yesNode~interact
  else
    noNode~interact
