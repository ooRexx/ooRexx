/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

/**
 * ooRexx OLE Automation Support                                    OLEVariant.c
 *
 * Methods for the OLEVariant class.
 *
 * OLEVariants are ooRexx objects that represent a VARIANTARG used as a
 * parameter in an IDispatch::Invoke method call.  When the OLEObject class uses
 * IDispatch to invoke methods on OLE / COM objects the parameters to these
 * methods must be converted from ooRexx objects to VARIANTARGs.  The OLEObject
 * class does that automatically.
 *
 * A primary purpose of the OLEVariant class is to allow an ooRexx programmer to
 * override the automatic conversion done by the OLEObject.  In addition, if the
 * parameter is an [IN/OUT] parameter, an OLEVariant is used to tranport the
 * returned value back to the calling ooRexx program.
 */

#include <stdio.h>
#include <stdlib.h>

#include "oorexxapi.h"
#include "OLEVariant.h"

/**
 * Method:  init
 *
 * Initializes an OLEVariant using the ooRexx object that will be converted to
 * a variant arg used as a parameter in an IDispatch invocation.
 *
 * @param v_value      The ooRexx object to be converted.
 *
 * @param v_type       The variant type of the parameter, the type the ooRexx
 *                     object should be corerced to.  This argument is optional.
 *                     If is is omitted or .nil, the OLEObject's automatic type
 *                     conversion will be used.
 *
 * @param param_flags  The PARAMFLAG_* flag(s) to use with the parameter in the
 *                     IDispatch invocation.  This argument is optional.  If it
 *                     is omitted or .nil, the OLEObject will use its internal
 *                     process for determining the flags.
 *
 * @return             This method returns .nil.
 */
RexxMethod4(int, OLEVariant_Init,
            OSELF, self,
            RexxObjectPtr, v_value,
            OPTIONAL_RexxObjectPtr, v_type,
            OPTIONAL_RexxStringObject, param_flags)
{
    RexxObjectPtr  vtString = NULL;

    convertToVT(context, v_type, 2);
    convertToParamFlag(context, param_flags, 3);

    context->SetObjectVariable("!_VAR_VALUE_", v_value);
    context->SetObjectVariable("!_CLEAR_VARIANT_", context->True());
    context->SetObjectVariable("!_VARIANT_PTR_", context->NewPointer(NULL));

    return 0;
}

/**
 * Method:  !varValue_ =
 *
 * Sets (changes) the ooRexx object that will be converted to a variant arg of
 * this OLE variant.
 *
 * @param v_value  The new ooRexx object to be converted.
 *
 * @return         This method returns .nil.
 */
RexxMethod2(RexxObjectPtr, OLEVariant_VarValueEquals,
            OSELF, self,
            RexxObjectPtr, v_value)
{
    context->SetObjectVariable("!_VAR_VALUE_", v_value);

    return context->Nil();
}

/**
 * Method:  !varType_ =
 *
 * Sets (changes) the VARTYPE value of this OLE variant.
 *
 * @param v_type  The new value for the VARTYPE.
 *
 * @return        This method returns .nil.
 */
RexxMethod2(RexxObjectPtr, OLEVariant_VarTypeEquals,
            OSELF, self,
            RexxObjectPtr, v_type)
{
    convertToVT(context, v_type, 1);
    return context->Nil();
}

/**
 * Method:  !paramFlags_ =
 *
 * Sets (changes) the PARAMFLAG_* value of this OLE variant.
 *
 * @param param_flags  The new value for the param flags.
 *
 * @return             This method returns .nil.
 */
RexxMethod2(RexxObjectPtr, OLEVariant_ParamFlagsEquals,
            OSELF, self,
            RexxObjectPtr, param_flags)
{
    convertToParamFlag(context, param_flags, 1);
    return context->Nil();
}

/**
 * Change the string representation of a VARTYPE, passed as an argument to an
 * OLEVariant method, to its numerical value, if possible.  Throw an exception
 * if it is not possible.
 *
 * @param v_type
 * @param position
 */
static void convertToVT(RexxMethodContext *context,  RexxObjectPtr v_type, int position )
{
    RexxObjectPtr vtString = NULL;
    if ( v_type != NULL && v_type != context->Nil() )
    {
        vtString = context->ObjectToString(v_type);
        vtString = stringToVT(context, vtString);
        if ( ! vtString )
        {
            context->RaiseException2(Rexx_Error_Incorrect_method_argType, context->WholeNumberToObject(position), context->NewStringFromAsciiz("VARTYPE"));
        }
    }

    context->SetObjectVariable("!_VAR_TYPE_", vtString == NULL ? context->Nil() : vtString);
    context->SetObjectVariable("!_VAR_TYPE_STR_", vtString == NULL ? context->NewStringFromAsciiz("default") : v_type);
}

/**
 * Change the string representation of PARAMFLAGs, passed as an argument in an
 * OLEVariant method, to its numerical value, if possible.  Throw an exception
 * if it is not possible.  This function sets the !_PARAM_FLAGS_ and
 * !_PARAM_FLAGS_STR_ variables on success.
 *
 * @param param_flags  The ooRexx object to convert.
 *
 * @param position     The argument position of param_flags in
 *                     the OLEVariant method.
 */
static void convertToParamFlag(RexxMethodContext *context,  RexxObjectPtr param_flags, int position )
{
    RexxObjectPtr  flagsString = NULL;

    if ( param_flags != NULL && param_flags != context->Nil() )
    {
        flagsString = context->ObjectToString(param_flags);
        flagsString = stringToFlags(context, flagsString);
        if ( ! flagsString )
        {
            context->RaiseException2(Rexx_Error_Incorrect_method_argType, context->WholeNumberToObject(position), context->NewStringFromAsciiz("PARAMFLAG"));
        }
    }

    context->SetObjectVariable("!_PARAM_FLAGS_", flagsString == NULL ? context->Nil() : flagsString);
    context->SetObjectVariable("!_PARAM_FLAGS_STR_",
                flagsString == NULL ? context->NewStringFromAsciiz("default") : param_flags);
}

/**
 * Take a string representing a VARTYPE expression and turn it into a string
 * representing its numerical value.  Only valid VARIANTARGs are allowed, that
 * is, only VARTYPEs that are valid to use in DISPPARAMS are converted.
 *
 * @param rxStr  The string to convert.
 *
 * @return       The numerical value of the VARIANTARG corresponding to rxStr,
 *               or null if the string is not valid.
 */
static RexxObjectPtr stringToVT(RexxMethodContext *context, RexxObjectPtr rxStr )
{
    RexxObjectPtr  rxResult = NULL;
    CHAR       *pszRxStr;
    CHAR       *pTmp;
    CHAR        szBuffer[6];  // Largest value is 0xFFFF == 65535.
    VARENUM     v1, v2;

    pszRxStr = pszStringDupe(context->ObjectToStringValue(rxStr));
    if ( !pszRxStr )
    {
        context->RaiseException0(Rexx_Error_System_resources);
    }

    // Allow case insensitive.
    pszRxStr = strupr(pszRxStr);

    // There must be either 1 or 2 VT_xx symbols, anything else is not valid.
    switch ( countSymbols(pszRxStr, FLAG_SEPARATOR_CHAR) )
    {
        case 0 :
            v1 = findVT(stripNonCSyms(pszRxStr));
            if ( v1 != VT_ILLEGAL && v1 != VT_VARIANT && v1 != VT_BYREF &&
                 v1 != VT_ARRAY )
            {
                sprintf(szBuffer, "%d", v1);
                rxResult = context->WholeNumberToObject(v1);
            }
            break;

        case 1 :
            pTmp = strchr(pszRxStr, FLAG_SEPARATOR_CHAR);
            v2 = findVT(stripNonCSyms(pTmp));

            *pTmp = 0x0;
            v1 = findVT(stripNonCSyms(pszRxStr));

            if ( v1 != VT_ILLEGAL && v2 != VT_ILLEGAL && areValidVTs(v1, v2) )
            {
                sprintf(szBuffer, "%d", v1 | v2);
                rxResult = context->WholeNumberToObject(v1 | v2);
            }
            break;

        default :
            break;
    }

    ORexxOleFree(pszRxStr);

    return rxResult;
}

/**
 * Take a string representing the wParmFlags field in a PARAMDESC struct and
 * turn it into a string representing its numerical value.
 *
 * wParmFlags are true flags, as long as each token is a valid flag, any number
 * of tokens can be or'd together any number of times and the result is valid.
 *
 * @param   The string to convert.
 *
 * @return  The integer value (as a string) of the wParamFlags, or null if the
 *          string to convert was not valid.
 */
static RexxObjectPtr stringToFlags(RexxMethodContext *context, RexxObjectPtr rxStr )
{
    RexxObjectPtr  rxResult = NULL;  // Return null if invalid.
    CHAR       *pszRxStr;
    CHAR       *ptr;
    int         tmp, count, i;
    int         val = 0;

    pszRxStr = pszStringDupe(context->ObjectToStringValue(rxStr));
    if ( !pszRxStr )
    {
        context->RaiseException0(Rexx_Error_System_resources);
    }

    // Allow case insensitive.
    pszRxStr = strupr(pszRxStr);

    count = countSymbols(pszRxStr, FLAG_SEPARATOR_CHAR);

    // Look at each token, and if valid use it.
    for ( i = 0; i < count; i++ )
    {
        ptr = strrchr(pszRxStr, FLAG_SEPARATOR_CHAR);
        tmp = findFlag(stripNonCSyms(ptr));

        if ( tmp == PARAMFLAG_ILLEGAL )
        {
            val = tmp;
            break;
        }
        val |= tmp;
        *ptr = 0x0;
    }

    // If still valid, pick up the last token.
    if ( val != PARAMFLAG_ILLEGAL )
    {
        tmp = findFlag(stripNonCSyms(pszRxStr));
        if ( tmp != PARAMFLAG_ILLEGAL )
        {
            rxResult = context->WholeNumberToObject(val | tmp);
        }
    }

    ORexxOleFree(pszRxStr);

    return rxResult;
}

/**
 * Count the number of occurrences of a character symbol in a string.
 *
 * @param pszStr  The string to examine.
 *
 * @param symbol  The character to count.
 *
 * @return  The number counted.
 */
static __inline int countSymbols( PSZ pszStr, char symbol )
{
    int count = 0;
    PSZ ptr   = pszStr;

    while ( (ptr = strchr(ptr, symbol)) != NULL )
    {
        ptr++;
        count++;
    }
    return count;
}

/**
 * Return a pointer to the specified string, stripped of leading and trailing
 * non-C symbol characters.
 *
 * Note that pszStr may be changed after return, the first trailing non-C symbol
 * will be replaced with a null.
 *
 * @param pszStr  The string to strip.
 *
 * @return  A pointer to the start of the stripped string.
 */
static __inline PSZ stripNonCSyms( PSZ pszStr )
{
    CHAR *pFront = pszStr;
    CHAR *pBack  = pszStr + strlen(pszStr);

    while ( pBack > pFront && ! iscsym(*pBack) )
        pBack--;
    if ( ! iscsym(*pBack) )
    {
        // No C symbols in string.
        *pBack = 0x0;
    } else
    {
        *(++pBack) = 0x0;
        while ( pFront < pBack && ! iscsym(*pFront) )
            pFront++;
    }
    return pFront;
}

/**
 * Determine if v1 and v2 are VARENUMs that can be combined for a valid
 * VARIANTARG.
 *
 * Assumes:  v1 and v2 are valid VARENUMs for a VARIANTARG, and that the only
 * thing needed to be checked is that the combination is valid.
 *
 * @param v1   Value for the first VARENUM.
 *
 * @param v2   Value for the second VARENUM.
 *
 * @return     True if v1 and v2 form a valid VARIANTARG, otherwise false.
 */
static __inline BOOL areValidVTs( VARENUM v1, VARENUM v2 )
{
    if ( v1 == VT_BYREF || v1 == VT_ARRAY )
    {
        if ( v2 != VT_BYREF && v2 != VT_ARRAY && v2 != VT_EMPTY &&
             v2 != VT_NULL )
        {
            return TRUE;
        }
    } else if ( v2 == VT_BYREF || v2 == VT_ARRAY )
    {
        if ( v1 != VT_BYREF && v1 != VT_ARRAY && v1 != VT_EMPTY &&
             v1 != VT_NULL )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Return the VARENUM that matches pszStr, or return VT_ILLEGAL if no match.
 *
 * @param pszStr  The string to examine.
 *
 * @return        The VARENUM equivalent of pszStr if there is a match, or
 *                VT_ILLEGAL if there is no match.
 */
static __inline VARENUM findVT( PSZ pszStr )
{
    int i;
    for ( i = 0; vtStrTable[i] != NULL; i++ )
    {
        if ( strcmp(pszStr, vtStrTable[i]) == 0 )
        {
            return vtIntTable[i];
        }
    }
    return VT_ILLEGAL;
}

/**
 * Find the the PARAMFLAG that matches pszStr.
 *
 * @param pszStr  The string to match.
 *
 * @return        The PARAMFLAG matching pszStr, or PARAMFLAG_ILLEAGAL if no
 *                match.
 */
static __inline int findFlag( PSZ pszStr )
{
    int i;
    for ( i = 0; flagStrTable[i] != NULL; i++ )
    {
        if ( strcmp(pszStr, flagStrTable[i]) == 0 )
        {
            return flagIntTable[i];
        }
    }
    return PARAMFLAG_ILLEGAL;
}

