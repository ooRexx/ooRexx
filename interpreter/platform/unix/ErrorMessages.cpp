/******************************************************************************/
/*                                                                            */
/* Retrieve message from message repository using the X/Open catopen(),       */
/* catgets() and catclose() function calls.                                   */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>                     /* include standard headers          */
#include <string.h>
#include <ctype.h>
#include <limits.h>

#if defined( HAVE_MESG_H )
# include <mesg.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#define ERROR_TABLE                    /* include error message table       */
#include "RexxCore.h"                    /* incl general definitions        */
#include "StringClass.hpp"
#include "SystemInterpreter.hpp"
#include "RexxInternalApis.h"
                                       /* define macros to bulid entries in */
/* the msgEntry table for msg lookup */
#define MESSAGE(code, message)   {code, message}, /* Major error codes                 */


// a define for the message table entries
typedef struct msgEntry
{
     int    code;                          // symbolic code for the message
     const char *message;                  // the error message text
} ERROR_MESSAGE;

#include "RexxErrorMessages.h"


const char *REXXENTRY RexxGetErrorMessage(int code)
{
    for (ERROR_MESSAGE *p = Message_table; p->code != 0; p++)
    {
        // did we find the target code
        if (p->code == code)
        {
            // make this into a string object
            return p->message;
        }
    }
    // no message retrieved
    return NULL;
}

/**
 * Retrieve the message text for a give error code.
 *
 * @param code   The Rexx error code
 *
 * @return The error message associated with that code.
 */
RexxString* SystemInterpreter::getMessageText(wholenumber_t code)
{
    const char *message = RexxGetErrorMessage(code);
    if (message != NULL)
    {
        return new_string(message);
    }
    // no message retrieved
    return OREF_NULL;
}



