/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/***************************************************************************/
/* REXX sockets function support                               rxsock.h    */
/*                 sockets utility function package                        */
/***************************************************************************/

#include "oorexxapi.h"

/*------------------------------------------------------------------
 * typedef for struct
 *------------------------------------------------------------------*/
typedef struct sockaddr_in sockaddr_in;
typedef struct in_addr     in_addr;

#if defined(WIN32)
typedef int socklen_t;
#endif

class StemManager;


/*------------------------------------------------------------------
 * strip blanks from a line
 *------------------------------------------------------------------*/
void stripBlanks(char *string);

/*------------------------------------------------------------------
 * convert a stem variable to an array of ints
 *------------------------------------------------------------------*/
void stemToIntArray(RexxCallContext *context, RexxObjectPtr stem, int &count, int *&arr);

/*------------------------------------------------------------------
 * convert an array of ints to a stem variable
 *------------------------------------------------------------------*/
void intArrayToStem(RexxCallContext *context, RexxObjectPtr stem, int count, int *arr);

/*------------------------------------------------------------------
 * convert a stemmed variable to a sockaddr
 *------------------------------------------------------------------*/
void stemToSockAddr(RexxCallContext *context, StemManager &stem, sockaddr_in *pSockAddr);

/*------------------------------------------------------------------
 * convert a sockaddr to a stemmed variable
 *------------------------------------------------------------------*/
void sockAddrToStem(RexxCallContext *context, sockaddr_in *pSockAddr, StemManager &stem);

/*------------------------------------------------------------------
 * convert a hostent to a stemmed variable
 *------------------------------------------------------------------*/
void hostEntToStem(RexxCallContext *context, struct hostent *pHostEnt, StemManager &stem);

/*------------------------------------------------------------------
 * convert a string sock option to an integer
 *------------------------------------------------------------------*/
int stringToSockOpt(const char *pszOptName);

/*------------------------------------------------------------------
 * set errno
 *------------------------------------------------------------------*/
void setErrno(RexxCallContext *context);

/*------------------------------------------------------------------
 * set h_errno
 *------------------------------------------------------------------*/
void SetH_Errno(RexxCallContext *context);

/*------------------------------------------------------------------
 * perform end-of-function processing (mostly setting error info
 *------------------------------------------------------------------*/
void cleanup(RexxCallContext *context);

/*------------------------------------------------------------------
 * portable caseless compare function.
 *------------------------------------------------------------------*/
int caselessCompare(const char *op1, const char *op2);


class StemManager
{
public:
    StemManager(RexxCallContext *c) : context(c), stem(NULL), prefix(NULL) { }
    ~StemManager()
    {
        if (prefix != NULL)
        {
            free(prefix);
        }
    }

    /**
     * Resolve the stem object that was passed as an argument.
     *
     * @param source The source argument object.
     *
     * @return true if the stem could be resolved, false for any errors
     *         resolving the stem object.
     */
    bool resolveStem(RexxObjectPtr source)
    {
        // this is the simplest solution
        if (context->IsStem(source))
        {
            stem = (RexxStemObject)source;
        }
        else
        {
            const char *stemName = context->ObjectToStringValue(source);
            const char *dotPos = strchr(stemName, '.');
            // if no dot or the dot is the last character, this is a standard
            // stem value
            if (dotPos == NULL || dotPos == (stemName + strlen(stemName) + 1))
            {
                stem = context->ResolveStemVariable(source);
            }
            else
            {
                prefix = strdup(dotPos + 1);
                if (prefix == NULL)
                {
                    context->InvalidRoutine();
                    return false;
                }

                // uppercase the rest of the prefix value
                char *scanner = prefix;
                while (*scanner != '\0')
                {
                    *scanner = toupper(*scanner);
                    scanner++;
                }
                RexxStringObject stemPortion = context->String(stemName, (dotPos - stemName) + 1);
                stem = context->ResolveStemVariable(stemPortion);
            }
            if (stem == NULL)
            {
                context->InvalidRoutine();
                return false;
            }
        }
        return true;
    }

    /**
     * Set a value in the argument stem.
     *
     * @param name   The name to set.
     * @param value
     */
    void setValue(const char *name, RexxObjectPtr value)
    {
        if (prefix == NULL)
        {
            context->SetStemElement(stem, name, value);
        }
        else
        {
            char fullName[256];
            sprintf(fullName, "%s%s", prefix, name);
            context->SetStemElement(stem, fullName, value);
        }
    }

    /**
     * Set a value in the argument stem.
     *
     * @param index  The index to set.
     * @param value
     */
    void setValue(size_t index, RexxObjectPtr value)
    {
        if (prefix == NULL)
        {
            context->SetStemArrayElement(stem, index, value);
        }
        else
        {
            char fullName[256];
            sprintf(fullName, "%s.%d", prefix, (int)index);
            context->SetStemElement(stem, fullName, value);
        }
    }

    /**
     * Retrieve a value from an argument stem.
     *
     * @param name   The argument stem name.
     *
     * @return The retrieved object, if any.
     */
    RexxObjectPtr getValue(const char *name)
    {
        if (prefix == NULL)
        {
            return context->GetStemElement(stem, name);
        }
        else
        {
            char fullName[256];
            sprintf(fullName, "%s%s", prefix, name);
            return context->GetStemElement(stem, fullName);
        }
    }

    RexxObjectPtr getValue(size_t index)
    {
        if (prefix == NULL)
        {
            return context->GetStemArrayElement(stem, index);
        }
        else
        {
            char fullName[256];
            sprintf(fullName, "%s.%d", prefix, (int)index);
            return context->GetStemElement(stem, fullName);
        }
    }


protected:
    RexxCallContext *context; // the context pointer
    RexxStemObject stem;      // the target stem
    char *prefix;             // extra prefix to use on the stem
};
