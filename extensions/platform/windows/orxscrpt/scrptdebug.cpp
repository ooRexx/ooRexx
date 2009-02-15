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
#include "scrptdebug.hpp"

extern FL VariantFlags[];

extern FL VariantTypes[];

extern FL MSDispIDs[];

int invCount = 0;
int HeaderLength = 0;
CRITICAL_SECTION stop = { 0 };

#ifndef SCRIPTDEBUG

//  The invCount is only in this module.  There should be only one of these per method.
void _FPRINTF(FILE *Stream, const char *Format,...) {
    if (Stream == NULL)
    {
        return;
    }

    if (invCount == 0)
    {
        InitializeCriticalSection(&stop);
    }
    EnterCriticalSection(&stop);
    invCount++;
    SYSTEMTIME st;
    GetSystemTime(&st);
    fprintf(Stream,"%3d %02x %4d - [%2.2d:%2.2d.%3.3d] ",_getpid(),GetCurrentThreadId(),invCount, st.wMinute, st.wSecond,st.wMilliseconds);
    if (HeaderLength == 0)
    {
        char Temp[1024];
        sprintf(&Temp[0],"%3d %02x %4d - [%2.2d:%2.2d.%3.3d] ",_getpid(),GetCurrentThreadId(),invCount, st.wMinute, st.wSecond,st.wMilliseconds);
        HeaderLength = strlen(&Temp[0]);
    }
    va_list marker;

    va_start(marker,Format);
    RetCode = vfprintf(Stream,Format,marker);
    va_end(marker);
    fflush(Stream);
    LeaveCriticalSection(&stop);

}

void _FPRINTF2(FILE *Stream, const char *Format, ...) {
    if (Stream == NULL)
    {
        return;
    }
    va_list marker;
    for (int i=0; i<HeaderLength; ++i) fprintf(Stream," ");
    va_start(marker,Format);
    RetCode = vfprintf(Stream,Format,marker);
    va_end(marker);
    fflush(Stream);
}



/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void FPRINTF3(FILE *Stream, const char *Format, ...) {
    if (Stream == NULL)
    {
        return;
    }
    va_list marker;
    va_start(marker,Format);
    RetCode = vfprintf(Stream,Format,marker);
    va_end(marker);
    fflush(Stream);
    return RetCode;
}

#endif




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void Reverse(char *Buffer, int BufLen)
{
    char temp;
    int i;

    for (i=0; i<BufLen/2; i++)
    {
        temp = Buffer[i];
        Buffer[i] = Buffer[BufLen - (i+1)];
        Buffer[BufLen - (i+1)] = temp;
    }
}



/****************************************************************************
 *
 *
 *
 *****************************************************************************/
/*  Len is length of source.  */
void C2X(char *Dest, char *Source, int Len)
{

    int i;

    Dest[0] = 0;
    for (i=0; i<(2*Len); i+=2)
    {
        sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    }

    return;
}





/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void X2C(char *Dest, char *Source, int DLen, int SLen)
{

    char tBuff[]="00";
    int i,init,SLenC,Diff,SOff,DOff;


    SLenC = (SLen + 1) / 2;    //  Source Length Compressed to bytes used after conversion.
    Diff = DLen - SLenC;       //  How many bytes do we pad, or truncate?
    Dest[0] = 0;               //  Guarantee that the first byte is '00'x
    SOff = 0;                  //  Source Offset starts at zero.
    DOff = Diff;               //  Destination Offset starts as the difference.

    if (Diff < 0)
    {
        SOff = SLen - (DLen * 2);
        SLen = DLen * 2;
        DOff = 0;
    }
    else
    {
        for (i=0;i<Diff;i++)
        {
            Dest[i] = 0;
        }
    }

    //  For the case where the source is an odd number of bytes, pad left with a '0'.
    if (SLen%2 == 1 && SLen > 0)
    {
        tBuff[1] = Source[SOff];
        sscanf(&tBuff[0],"%2x",(unsigned char *)&Dest[DOff]);
        init = 2;
    }
    else
    {
        init = 0;
    }
    for (i=init; i<SLen; i+=2)
    {
        sscanf((const char *)&Source[i+SOff],"%2x",(unsigned char *)&Dest[(i/2)+DOff]);
    }

    return;
}





/****************************************************************************
 *
 *     IID (UUID, GUID, etc. to registry format.  Dest should be at least
 *  40 bytes long.
 *
 *****************************************************************************/
void pIID2Reg(char *Dest, char *Source)
{
    int i;

    Dest[0] = '{';
    for (i=1; i<(2*4); i+=2)  sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    Dest[i] = '-';
    for (i+=1; i<(2*10); i+=2)  sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    Dest[i] = '-';
    for (i+=1; i<(2*12); i+=2)  sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    Dest[i] = '-';
    for (i+=1; i<(2*14); i+=2)  sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    Dest[i] = '-';
    for (i+=1; i<(2*16); i+=2)  sprintf(&Dest[i],"%02x",(unsigned char)Source[i/2]);
    Dest[i] = '}';
    Dest[++i] = '\0';
    return;
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void strstrip(char *Dest,char *Source)
{
    int i,Begin,End,SourceLen;

    SourceLen = strlen(Source);
    for (End=SourceLen-1; End>-1 && ' '==Source[End] ; End--);
    if (End < 0)
    {  // All blank.
        Dest[0] = '\0';
        return;
    }
    End++;
    for (Begin=0; Begin<SourceLen && ' '==Source[Begin] ; Begin++);
    if (Source != Dest || Begin != 0)
    {
        for (i=0; i<(End-Begin); i++)
        {
            Dest[i] = Source[i+Begin];
        }
        Dest[i] = '\0';
    }

    if (Source == Dest && Begin == 0 && End != SourceLen)
    {
        Dest[End] = '\0';
    }

    return;
}





/****************************************************************************
 *
 *
 *
 *****************************************************************************/
int Dump(FILE *Stream,char *Mem, int Length)
{
    char Line[100];
    int Outer, O, Inner,h,i,j,l;


    if (Length < 1)
    {
        printf("%016x ",0);
    }

    Outer = (Length + 15) / 16;
    j = 0;
    for (O=0; O<Outer; O++)
    {
        sprintf(Line,"%016x ",j);
        l = strlen(Line);
        h = j;
        for (i=0; i<4; i++)
        {
            Inner = (h+4);
            for (;h<Inner && h<Length;h++,l++)
            {
                sprintf(&Line[l++],"%02x",(unsigned char)Mem[h]);
            }
            for (;h<Inner ;h++,l++)
            {
                strcat(&Line[l++],"  ");
            }
            strcat(Line," ");
            l++;
            if (i == 1)
            {
                strcat(Line," ");
                l++;
            }
        }
        strcat(Line,"      *");
        l = strlen(Line);
        Inner = j + 16;
        for (; j<Inner; j++,l++)
        {
            if ((unsigned int)Mem[j] > 27  && (unsigned int)Mem[j] < 128 && j < Length)
            {
                Line[l] = Mem[j];
            }
            else if (j<Length)
            {
                Line[l] = '.';
            }
            else
            {
                Line[l] = ' ';
            }
            if ((Inner-j) == 9)
            {
                Line[++l] = ' ';
            }
        }
        Line[l] = '\0';
        strcat(Line,"*\n");
        fprintf(Stream,Line);
    }
    return 0;
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
const char *FlagMeaning(char FlagType, DWORD Flag, pFL List)
{
    static char RetBuf[1024];
    DWORD LeftOver;
    //  Even though this is by value, changing parm values is kind of annhhh......
    char FT;
    pFL oList;
    int i;


    //  Check the first parm.
    switch (FlagType)
    {
        case 'v':
        case 'V':
            FT = 'V';  // For easier checking later.
            break;
        case 'H':
        case 'h':
            FT = 'H';
            break;
        default:
            strcpy(RetBuf,"'x' is not a valid flag type: 'v' or 'h'.");
            RetBuf[1] = FlagType;   // Bad things happen if FlagType < 27.
            return RetBuf;
    }
    //  Check the last parm.
    //  The rest of the parms are evaluated in the code that follows.
    if (List == NULL)
    {
        return "No List was passed!!!!!!";
    }

    oList = List;

    //  This is why these weren't split into separate routines.  The code
    // for the vertical is so small.
    if (FT == 'V')
    {
        while (oList->Meaning != NULL)
        {
            if (oList->Flag == Flag)
            {
                return oList->Meaning;
            }
            oList++;
        }
        sprintf(RetBuf,"0x%08x is not a known flag value!",Flag);
        return RetBuf;
    }

    if (FT == 'H')
    {
        if (!Flag) return "No flags were set.";  // Not a valid horizontal.
        LeftOver = Flag;
        RetBuf[0] = '\0';  // strcpy(RetBuf,"") without the overhead.
        RetBuf[1] = '\0';  // So we can cheat, later.
        while (oList->Meaning != NULL)
        {
            if (oList->Flag & Flag)
            {
                strcat(RetBuf,"+");
                strcat(RetBuf,oList->Meaning);
                LeftOver = LeftOver & ~(oList->Flag);
            }
            oList++;
        }
        if (LeftOver)
        {           //  If there is anything left over, say what it is.
            i = strlen(RetBuf);
            sprintf(&RetBuf[i],"+0x%08x is(are) not a known flag value(s)!",LeftOver);
        }
        return &RetBuf[1];      //  This is later, and this is the cheat.  It elide's
        // the leading plus sign.
    }
    return "The impossible has happened.";    // Keep the compiler happy.
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
char *NameThatInterface(OLECHAR *IID)
{
    char  KeyName[MAX_PATH], *ReturnString=NULL;
    HKEY  Interface;
    LONG  WinRC;
    DWORD Type,RetLength;


    if (wcslen(IID) > (MAX_PATH-20))
    {
        return ReturnString;
    }
    sprintf(KeyName,"Interface\\%S",IID);
    WinRC = RegOpenKeyEx(HKEY_CLASSES_ROOT,KeyName,0,KEY_READ,&Interface);
    if (WinRC != ERROR_SUCCESS)
    {
        return ReturnString;
    }
    WinRC = RegQueryValueEx(Interface,NULL,NULL,&Type,NULL,&RetLength);
    if (WinRC != ERROR_SUCCESS)
    {
        goto NTI_Failed;
    }
    if (Type  != REG_SZ)
    {
        goto NTI_Failed;
    }
    RetLength += 3;
    if (!(ReturnString = (char *)malloc((int)RetLength)))
    {
        goto NTI_Failed;
    }
    WinRC = RegQueryValueEx(Interface,NULL,NULL,&Type,(LPBYTE)ReturnString,&RetLength);
    if (WinRC != ERROR_SUCCESS)
    {
        free(ReturnString);
        ReturnString = NULL;
    }
    NTI_Failed:
    RegCloseKey(Interface);
    return ReturnString;
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void PrntMutant(FILE *Stream, VARIANT *Mutant)
{
    HRESULT RetCode=S_OK;
    VARIANT String;
    short int lVFlags,lVType;



    //  Variant & ~VT_TYPEMASK     yields variant flags.
    //  Variant & VT_TYPEMASK      yields variant types.
    lVFlags = (short int)Mutant->vt & ~VT_TYPEMASK;
    lVType  = (short int)Mutant->vt & VT_TYPEMASK;
    FPRINTF2(Stream,"Variant Type %s and Variant Flags: \"%s\"\n",
             FlagMeaning('V',lVType,VariantTypes),
             FlagMeaning('H',lVFlags,VariantFlags));
    FPRINTF2(Stream,">>>>> Parm Value:\n");
    if ( ((short int)Mutant->vt == VT_UNKNOWN)  ||
         ((short int)Mutant->vt == VT_PTR)       ||
         ((short int)Mutant->vt & ~VT_TYPEMASK)  ||
         ((short int)Mutant->vt == VT_DISPATCH) )
    {
        FPRINTF2(Stream,"this is a ptr.\n");
        if (((short int)Mutant->vt & ~VT_TYPEMASK))
        {
            FPRINTF2(Stream,"High bits are on!\n");
        }
        FPRINTF2(Stream,"%p\n",Mutant->pdispVal);
    }
    else
    {
        FPRINTF2(Stream,"this is a non-pointer\n");
        VariantInit(&String);
        if (VariantChangeType(&String,Mutant,0,VT_BSTR) == S_OK)
        {
            FPRINTF2(Stream,"%S\n",String.bstrVal);
            VariantClear(&String);    // This releases everything.
        }
        else
        {
            FPRINTF2(Stream,"Could not convert Parm Value to BSTR\n");
        }
    }

    FPRINTF2(Stream,"<<<< End Parm Value\n");
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
void PrntDispParams(FILE *Stream, DISPPARAMS *Params)
{
    int     i,lNACount;
    Dump(Stream,(char *)Params,  sizeof(DISPPARAMS));

    FPRINTF2(Stream,"Parms has %d arguments, %d are named.\n",
             Params->cArgs,Params->cNamedArgs);
    if (Params->cNamedArgs > 0 && Params->rgdispidNamedArgs == NULL)
    {
        FPRINTF2(Stream,"Houston, we have a problem. cNamedArgs > 0, but no pointer.\n");
        lNACount = 0;
    }
    else
    {
        lNACount = Params->cNamedArgs;
    }
    for (i=0; (unsigned)i < Params->cArgs; i++)
    {
        //  See if this is a "named" arg.
        if (i < lNACount)
        {
            if (Params->rgdispidNamedArgs[i] < 1)
            {
                FPRINTF2(Stream,"This is a Microsoft defined DISPID %08x (%s)",Params->rgdispidNamedArgs[i],
                         FlagMeaning('V',Params->rgdispidNamedArgs[i],MSDispIDs));
            }
            FPRINTF2(Stream,"The DISPID is %ld\n",Params->rgdispidNamedArgs[i]);
        }
        PrntMutant(Stream, &(Params->rgvarg[i]));
    }
}


void EnumerateProperty(FILE *Stream, IDispatchEx *DispEx, LCID Lang)
{
    HRESULT    RetCode;
    DISPPARAMS DispParms;
    EXCEPINFO  ExcepInfo;
    UINT       ErrCode;
    BSTR       lName;
    VARIANT    RetInfo;
    DISPID     DispID;


    FPRINTF2(Stream,"---- debug --- EnumerateProperty()  \n");
    //  fdexEnumAll
    //  fdexEnumDefault
    RetCode = DispEx->GetNextDispID(fdexEnumAll,-1,&DispID);
    FPRINTF2(Stream,"EnumerateProperty() --  DispID %11d  ",DispID);
    VariantInit(&RetInfo);
    while (DispID != -1)
    {
        RetCode = DispEx->GetMemberName(DispID,&lName);
        FPRINTF3(Stream,"Name \"%S\" \n",lName);
        SysFreeString(lName);
        if (SUCCEEDED(RetCode))
        {
            ErrCode = 0;
            DispParms.rgvarg = NULL;
            DispParms.rgdispidNamedArgs = NULL;
            DispParms.cArgs = 0;
            DispParms.cNamedArgs = 0;

            RetCode = DispEx->InvokeEx(DispID,Lang,DISPATCH_PROPERTYGET,&DispParms,&RetInfo,&ExcepInfo,NULL);
            PrntMutant(Stream,&RetInfo);

            VariantClear(&RetInfo);
        }  // GetIDsOfNames()
        RetCode = DispEx->GetNextDispID(fdexEnumAll,DispID,&DispID);
        FPRINTF2(Stream,"EnumerateProperty() --  DispID %11d  ",DispID);
    }
    FPRINTF2(Stream,"\n");



    return;
}
