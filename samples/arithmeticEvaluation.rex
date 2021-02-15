#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/*  arithmeticEvaluation.rex Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program demonstrates a number of principles of object-oriented       */
/*  program.  This implements a simple expression evaluator that builds a     */
/*  parse tree for an arithmetic expression that can also evaluate the        */
/*  given expression.                                                         */
/******************************************************************************/

-- test expressions.  Each will be parsed, and if syntactically correct, will
-- also be evaluated.
expressions = .array~of("2+3", "2+3/4", "2*3-4", "2*(3+4)+5/6", "2 * (3 + (4 * 5 + (6 * 7) * 8) - 9) * 10", "2*-3--4+-.25")

loop input over expressions
    -- parse the target expression into a tree
    expression = createExpression(input)
    -- parsing failures return .nil...so if we get something good, display the
    -- parsed result and then evaluate the expression.
    if expression \= .nil then
        say 'Expression "'input'" parses to "'expression~string'" and evaluates to "'expression~evaluate'"'
end


-- create an executable expression from the input, printing out any
-- errors if they are raised.
::routine createExpression
  use arg inputString

  -- enable syntax trapping in case we have an invalid expression
  signal on syntax
  return .ExpressionParser~new~parseExpression(inputString)

syntax:
   say 'Syntax error parsing expression "'inputString'"'
   condition = condition('o')
   say condition~errorText
   say condition~message
   -- return the failure indicator
   return .nil


-- a base class for tree nodes in the tree
-- all nodes return some sort of value.  This can be constant,
-- or the result of additional evaluations
::class EvaluatorNode
-- all evaluation is done here.  This an abstract method
-- in the base class...each subclass is expected to implement something
-- real for the evaluate method.
::method evaluate abstract

-- node for numeric values in the tree
::class Constant
::method init
  expose value
  use arg value

-- evaluate the constant.  This just returns the constant value
::method evaluate
  expose value
  return value

-- display the value of this node as a string.
::method string
  expose value
  return value

-- node for a parenthetical group on the tree
::class Parens
::method init
  -- a paren instance contains a pointer to the root node of the
  -- expression contained within the parens
  expose subexpression
  use arg subexpression

-- evaluate a parenthetical grouping.  The real work is done
-- by the nodes between the parens
::method evaluate
  expose subexpression
  return subexpression~evaluate

-- display the expression surrounded by parens
::method string
  expose subexpression
  return "("subexpression~string")"

-- base class for binary operators
::class BinaryOperator
::method init
  expose left right
  -- the left and right sides are set after the left and right sides have
  -- been resolved.
  left = .nil
  right = .nil

-- Evaluate the binary operation.  We apply our operation to
-- the result of evaluating the left- and right-hand sides of the
-- expression.
::method evaluate
  expose left right
  return self~operation(left~evaluate, right~evaluate)

-- More abstract methods to be implemented by the subclasses.
-- operation implements the operator, symbol is the display
-- value for the operation, precedence is the operator precedence
-- level used for building the parse tree.
::method operation abstract
::method symbol abstract
::method precedence abstract

-- display an operator as a string value.  We rely on
-- the symbol method here
::method string
  expose left right
  return '('left~string self~symbol right~string')'

-- accessor methods for retrieving and setting the operator operands
::attribute left
::attribute right

-- Operator class for an addition (+) operator
::class AddOperator subclass binaryoperator

::method operation
  use arg left, right
  return left + right

::method symbol
  return "+"

::method precedence
  return 1

-- Operator class for a subtraction (-) operator
::class SubtractOperator subclass binaryoperator
::method operation
  use arg left, right
  return left - right

::method symbol
  return "-"

::method precedence
  return 1

-- Operator class for a multiplication (*) operator
::class MultiplyOperator subclass binaryoperator
::method operation
  use arg left, right
  return left * right

::method symbol
  return "*"

::method precedence
  return 2

-- Operator class for a division (/) operator
::class DivideOperator subclass binaryoperator
::method operation
  use arg left, right
  return left / right

::method symbol
  return "/"

::method precedence
  return 2

-- a class to parse the expression and build an evaluation tree
::class ExpressionParser

-- parse the expression into an executable tree
::method parseExpression
  expose operands operator
  use strict arg inputString
  -- stacks for managing the operands and pending operators
  operands = .queue~new
  operators = .queue~new
  -- this flags what sort of item we expect to find at the current
  -- location
  afterOperand = .false

  loop currentIndex = 1 to inputString~length
      char = inputString~subChar(currentIndex)
      -- skip over whitespace
      if char == ' ' then iterate currentIndex
      -- If the last thing we parsed was an operand, then
      -- we expect to see either a closing paren or an
      -- operator to appear here
      if afterOperand then do
          -- handle a paren close...pop items off until we find the open
          if char == ')' then do
              loop while \operators~isempty
                  operator = operators~pull
                  -- if we find the opening paren, replace the
                  -- top operand with a paren group wrapper
                  -- and stop popping items
                  if operator == '(' then do
                     operands~push(.parens~new(operands~pull))
                     leave
                  end
                  -- collapse the operator stack a bit
                  self~createNewOperand(operator)
              end
              -- done with this character
              iterate currentIndex
          end

          afterOperand = .false

          -- not a paren, check for each of the operators
          select
              when char == "+" then operator = .addoperator~new
              when char == "-" then operator = .subtractoperator~new
              when char == "*" then operator = .multiplyoperator~new
              when char == "/" then operator = .divideoperator~new
              otherwise
              -- this is an invalid expression
              raise syntax 98.900 array("Invalid expression character" char)
          end

          -- we have an operator, now handle the operator precedence
          loop while \operators~isEmpty
              top = operators~peek
              -- start of a paren group stops the popping
              if top == '(' then leave
              -- or the top operator has a lower precedence
              if top~precedence < operator~precedence then leave
              -- process this pending one
              self~createNewOperand(operators~pull)
          end

          -- this new operator is now top of the stack
          operators~push(operator)
          -- and back to the top
          iterate currentIndex
      end
      -- if we've hit an open paren, add this to the operator stack
      -- as a phony operator
      if char == '(' then do
          operators~push('(')
          iterate currentIndex
      end
      -- not an operator, so we have an operand of some type
      afterOperand = .true
      startindex = currentIndex

      -- allow a leading minus sign on this
      if inputString~subchar(currentIndex) == '-' then
          currentIndex += 1
      -- now scan for the end of numbers
      loop while currentIndex <= inputString~length
          -- exit for any non-numeric value
          if \inputString~matchChar(currentIndex, "0123456789.") then leave
          currentIndex += 1
      end
      -- extract the string value
      operand = inputString~substr(startIndex, currentIndex - startIndex)

      -- make sure this actually is a valid number (their might be multiple
      -- decimal points, for example)
      if \operand~datatype('Number') then
          raise syntax 98.900 array("Invalid numeric operand '"operand"'")
      -- back this up to the last valid character
      currentIndex -= 1
      -- add this to the operand stack as a tree element that returns a constant
      operands~push(.constant~new(operand))
  end

  -- now process all the rest of the operators from the stack
  loop while \operators~isEmpty
      operator = operators~pull
      -- If we have a ( on the stack, then we're missing a closing paren
      if operator == '(' then
          raise syntax 98.900 array("Missing closing ')' in expression")
      -- add this to the parse tree
      self~createNewOperand(operator)
  end

  -- our entire expression should be the top of the expression tree
  expression = operands~pull

  -- somehow, we have dangling expression
  if \operands~isEmpty then
      raise syntax 98.900 array("Invalid expression")
  return expression

-- create a resolved operand from an operator instance and the top
-- two entries on the operand stack.
::method createNewOperand
  expose operands
  use strict arg operator
  -- the operands are a stack, so they are in inverse order current
  operator~right = operands~pull
  operator~left = operands~pull
  -- this goes on the top of the stack now
  operands~push(operator)

