!verbose 3
!include "WinMessages.NSH"
!verbose 4

; AddToPath - Adds the given dir to the search path.
;        Input - head of the stack
;        Note - Win9x systems requires reboot
; Usage:
; Push "directory"           The directory or the ".ext" to add
; Push "true" or "false"     True if installing with 'admin' rights, otherwise false
; Push "PATH" or "PATHEXT"   Environment variable being added to
; Call AddToPath

Function AddToPath
  Pop $R8 ; R8 now contains the env variable we are working with, PATH or PATHEXT
  Pop $R7 ; R7 now cotains "true" or "false".  True if installing with admin rights
  Exch $0 ; $0 now contains the value to add
  Push $1
  Push $2
  Push $3

  StrCpy $4 "$0" ; save the value being added (directory or .ext) in $4

  ; Get the current registry value and determine if the value being added already exists in the
  ; current value.  Don't add the new value if it already exist.
  StrCmp $R7 "true" ReadPath_AllUsers      ; if admin rights
  ReadRegStr $1 HKCU "Environment" "$R8"
  Goto ReadPath_UserCont
  ReadPath_AllUsers:
  ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8"
  ReadPath_UserCont:
  Push "$1;"
  Push "$0;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  Push "$1;"
  Push "$0\;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  ;
  ; Only compare shortpath if PATH env variable
  ;
  StrCmp $R8 "PATH" "" AddToPath_Not_PATH
  GetFullPathName /SHORT $3 $0
  Push "$1;"
  Push "$3;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  Push "$1;"
  Push "$3\;"
  Call StrStr
  Pop $2
  StrCmp $2 "" "" AddToPath_done
  Goto AddToPath_cont
  AddToPath_Not_PATH:
  StrCpy $3 $0
  AddToPath_cont:
  Call IsNT
  Pop $1
  StrCmp $1 1 AddToPath_NT
    ; Not on NT
    StrCpy $1 $WINDIR 2
    FileOpen $1 "$1\autoexec.bat" a
    FileSeek $1 -1 END
    FileReadByte $1 $2
    IntCmp $2 26 0 +2 +2 # DOS EOF
      FileSeek $1 -1 END # write over EOF
    FileWrite $1 "$\r$\nSET $R8=%$R8%;$3$\r$\n"
    FileClose $1
    SetRebootFlag true
    Goto AddToPath_done

  ; A very few users have reported that their path setting was erased and
  ; replaced by only the ooRexx install directory.  This could happen if there
  ; is an error reading the current system path from the registry.  If we detect
  ; this problem we do not change the PATH (or PATHEXT.)
  AddToPath_NT:
    StrCmp $R7 "true" AddToPath_AllUsers
    ReadRegStr $1 HKCU "Environment" "$R8"          ; The current user's path, it could be blank
    Goto AddToPath_UserCont
  AddToPath_AllUsers:
    ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8"
    StrCmp $1 "" 0 AddToPath_UserCont          ; if the value is blank, there is some problem, try to read it again
    ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8"
    StrCmp $1 "" AddToPath_Abort               ; if the value is still blank, there is some problem, then abort
  AddToPath_UserCont:
    StrCpy $2 $1 1 -1  # copy last char from path value to $2
    StrCmp $2 ";" 0 +2 # if last char == ';' then
    StrCpy $1 $1 -1    # remove last char from path value
    ; There is a path limit of 1024 chars prior to XP sp2, 2048 after that. However, NSIS has a string limit
    ; of 1024 unless a special build is used.  We check if the new path is going to be longer than 1024, and
    ; if so, abort and tell the user she needs to update the path manually.
    StrLen $2 $1
    StrLen $3 $0
    IntOp $2 $2 + 2   ; 1 for semi-colon, 1 for a (maybe) terminating null.
                      ;NSIS docs not clear on if 1024 is the string limit or buffer limit
    IntOp $2 $2 + $3
    IntCmp $2 1024 AddToPath_Abort_TooLong 0 AddToPath_Abort_TooLong  ; abort if equal or more than, less than continue
    StrCpy $0 "$1;$0"
    StrCmp $R7 "true" AddToPath_AllUsers_doit
    ; Writing registry for current user.  It's okay here to write back only what we are adding.
    ; The original value for the user specific PATH could have been blank.  In this case it is
    ; possible, maybe even likely, that an uneccesary ';' is tacked onto the front of the string.
    ; If so, remove it.
    StrCpy $2 $0 1     # copy first char string to write to $2
    StrCmp $2 ";" 0 +2 # if first char == ';' then
    StrCpy $0 $0 "" 1  # remove first char from string
    WriteRegExpandStr HKCU "Environment" "$R8" $0
    DetailPrint "$4 added to $R8 for Current User"
    Goto AddToPath_UserCont_doit
  AddToPath_AllUsers_doit:
    ; Writing registry for all users. This is the system-wide PATH (or PATHEXT.)  If what we are writing
    ; is only what we started out to add - something is wrong, we need to abort.
    StrCmp $0 ";$4" AddToPath_Abort
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8" $0
    DetailPrint "$4 added to $R8 for All Users"
  AddToPath_UserCont_doit:
    SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=10000
    Goto AddToPath_done

  ; Error reading the PATH or PATHEXT value, warn the user.  NOTE that this will
  ; put up the message box on a silent install.  The message box could be
  ; prevented by using the  /SD IDOK flag.  But, then the user would not know
  ; that an error had happened.  I would like to log the error to the install
  ; log file, but that requires building the NSIS system and setting some
  ; compile flags.
  AddToPath_Abort:
    MessageBox MB_OK|MB_ICONINFORMATION "There was a problem reading the current value of $R8.$\n$\nYou will need to set the $R8 environment variable manually."
    Goto AddToPath_done
  AddToPath_Abort_TooLong:
    MessageBox MB_OK|MB_ICONINFORMATION "Adding the ooRexx directory to the PATH results in a string too$\nlong for the installer to handle.$\n$\nYou will need to set the PATH environment variable manually."

  AddToPath_done:
    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

; RemoveFromPath - Remove a given dir from the path
;     Input: head of the stack
; Usage:
; Push "directory" or ".ext"
; Push "true" or "false"
; Push "PATH" or "PATHEXT"
; Call un.RemoveFromPath

Function un.RemoveFromPath
  Pop $R8 ; env variable; PATH or PATHEXT
  Pop $R7 ; "true" or "false" if admin user
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6

  IntFmt $6 "%c" 26 # DOS EOF

  Call un.IsNT
  Pop $1
  StrCmp $1 1 unRemoveFromPath_NT
    ; Not on NT
    StrCpy $1 $WINDIR 2
    FileOpen $1 "$1\autoexec.bat" r
    GetTempFileName $4
    FileOpen $2 $4 w
    ;
    ; Only compare shortpath if PATH env variable
    ;
    StrCmp $R8 "PATH" "" unRemoveFromPath_Not_PATH
    GetFullPathName /SHORT $0 $0
    unRemoveFromPath_Not_PATH:
    StrCpy $0 "SET $R8=%$R8%;$0"
    Goto unRemoveFromPath_dosLoop

    unRemoveFromPath_dosLoop:
      FileRead $1 $3
      StrCpy $5 $3 1 -1 # read last char
      StrCmp $5 $6 0 +2 # if DOS EOF
        StrCpy $3 $3 -1 # remove DOS EOF so we can compare
      StrCmp $3 "$0$\r$\n" unRemoveFromPath_dosLoopRemoveLine
      StrCmp $3 "$0$\n" unRemoveFromPath_dosLoopRemoveLine
      StrCmp $3 "$0" unRemoveFromPath_dosLoopRemoveLine
      StrCmp $3 "" unRemoveFromPath_dosLoopEnd
      FileWrite $2 $3
      Goto unRemoveFromPath_dosLoop
      unRemoveFromPath_dosLoopRemoveLine:
        SetRebootFlag true
        Goto unRemoveFromPath_dosLoop

    unRemoveFromPath_dosLoopEnd:
      FileClose $2
      FileClose $1
      StrCpy $1 $WINDIR 2
      Delete "$1\autoexec.bat"
      CopyFiles /SILENT $4 "$1\autoexec.bat"
      Delete $4
      Goto unRemoveFromPath_done

  unRemoveFromPath_NT:
    StrCmp $R7 "true" unRemoveFromPath_AllUsers
    ReadRegStr $1 HKCU "Environment" "$R8"
    Goto unRemoveFromPath_UserCont
  unRemoveFromPath_AllUsers:
    ReadRegStr $1 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8"
  unRemoveFromPath_UserCont:
    StrCpy $5 $1 1 -1 # copy last char
    StrCmp $5 ";" +2 # if last char != ;
      StrCpy $1 "$1;" # append ;
    Push $1
    Push "$0;"
    Call un.StrStr ; Find `$0;` in $1
    Pop $2 ; pos of our dir
    StrCmp $2 "" unRemoveFromPath_done
      ; else, it is in path
      # $0 - path to add
      # $1 - path var
      StrLen $3 "$0;"
      StrLen $4 $2
      StrCpy $5 $1 -$4 # $5 is now the part before the path to remove
      StrCpy $6 $2 "" $3 # $6 is now the part after the path to remove
      StrCpy $3 $5$6

      StrCpy $5 $3 1 -1 # copy last char
      StrCmp $5 ";" 0 +2 # if last char == ;
        StrCpy $3 $3 -1 # remove last char
      ;
      ; If the value is now empty, delete it
      ;
      StrCmp $3 "" "" unRemoveFromPath_rewrite
      ; we need to delete the value
      StrCmp $R7 "true" unRemoveFromPath_AllUsers_delete
      DeleteRegValue HKCU "Environment" "$R8"
      DetailPrint "$0 removed from $R8 for Current User"
      Goto unRemoveFromPath_UserCont_doit
    unRemoveFromPath_AllUsers_delete:
      DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8"
      DetailPrint "$0 removed from $R8 for All Users"
      Goto unRemoveFromPath_UserCont_doit
    unRemoveFromPath_rewrite:
      StrCmp $R7 "true" unRemoveFromPath_AllUsers_rewrite
      WriteRegExpandStr HKCU "Environment" "$R8" $3
      DetailPrint "$0 removed from $R8 for Current User"
      Goto unRemoveFromPath_UserCont_doit
    unRemoveFromPath_AllUsers_rewrite:
      WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "$R8" $3
      DetailPrint "$0 removed from $R8 for All Users"
    unRemoveFromPath_UserCont_doit:
      SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=10000

  unRemoveFromPath_done:
    Pop $6
    Pop $5
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

;====================================================
; EmptyDirectory - Determines if directory passed is empty
;                  and deletes it and from PATH if not empty
;          Returns: nothing
;          Input - PATH
;                - "true" or "false"
;====================================================
Function un.EmptyDirectory
  Pop $R7 ; "true" or "false" if admin user
  Pop $R5
  FindFirst $2 $1 "$5\*.*"
  StrCmp $1 "" empty
  StrCmp $1 "." cont
  StrCmp $1 ".." cont
  Goto done
cont:
  FindNext $2 $1
  ;Dumpstate::debug
  StrCmp $1 "" empty
  StrCmp $1 "." cont
  StrCmp $1 ".." cont
  ; to get here we have a real file, so get out now
  Goto done
empty:
  ; Remove the directory from PATH
  Push $R5
  Push $R7
  Push "PATH"
  Call un.RemoveFromPath
done:
  FindClose $2
FunctionEnd

###########################################
#            Utility Functions            #
###########################################


; StrStr
; input, top of stack = string to search for
;        top of stack-1 = string to search in
; output, top of stack (replaces with the portion of the string remaining)
; modifies no other variables.
;
; Usage:
;   Push "this is a long ass string"
;   Push "ass"
;   Call StrStr
;   Pop $R0
;  ($R0 at this point is "ass string")

!macro StrStr un
Function ${un}StrStr
Exch $R1 ; st=haystack,old$R1, $R1=needle
  Exch    ; st=old$R1,haystack
  Exch $R2 ; st=old$R1,old$R2, $R2=haystack
  Push $R3
  Push $R4
  Push $R5
  StrLen $R3 $R1
  StrCpy $R4 0
  ; $R1=needle
  ; $R2=haystack
  ; $R3=len(needle)
  ; $R4=cnt
  ; $R5=tmp
  loop:
    StrCpy $R5 $R2 $R3 $R4
    StrCmp $R5 $R1 done
    StrCmp $R5 "" done
    IntOp $R4 $R4 + 1
    Goto loop
done:
  StrCpy $R1 $R2 "" $R4
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Exch $R1
FunctionEnd
!macroend
!insertmacro StrStr ""
!insertmacro StrStr "un."
