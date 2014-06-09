!include WinMessages.nsh

#
# WriteEnvStr - Writes an environment variable
# Note: Win9x systems requires reboot
#
# Example:
#  Push "HOMEDIR"           # name
#  Push "C:\New Home Dir\"  # value
#  Push "true"              # true or false if admin user
#  Call WriteEnvStr
#
Function WriteEnvStr
  Pop $8 ; $8 IsAdminUser: true or false
  Exch $1 ; $1 has environment variable value
  Exch
  Exch $0 ; $0 has environment variable name
  Push $2

  StrCmp $8 "false" WriteEnvStr_NormalUser
  WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" $0 $1
  DetailPrint "Setting environment variable $0 to $1 for All Users"
  Goto WriteEnvStr_cont
WriteEnvStr_NormalUser:
  WriteRegStr HKCU "Environment" $0 $1
  DetailPrint "Setting environment variable $0 to $1 for Current User"
WriteEnvStr_cont:
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} \
    0 "STR:Environment" /TIMEOUT=5000

  Pop $2
  Pop $1
  Pop $0
FunctionEnd

#
# un.DeleteEnvStr - Removes an environment variable
# Note: Win9x systems requires reboot
#
# Example:
#  Push "HOMEDIR"           # name
#  Push "true"              # true or false if admin user
#  Call un.DeleteEnvStr
#
Function un.DeleteEnvStr
  Pop $8 ; $8 IsAdminUser: true or false
  Exch $0 ; $0 now has the name of the variable
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5

  StrCmp $8 "false" DeleteEnvStr_NormalUser
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" $0
  DetailPrint "Deleting environment variable $0 for All Users"
  Goto DeleteEnvStr_cont
DeleteEnvStr_NormalUser:
  DeleteRegValue HKCU "Environment" $0
  DetailPrint "Deleting environment variable $0 for Current User"
DeleteEnvStr_cont:
SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} \
  0 "STR:Environment" /TIMEOUT=5000

    Pop $5
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

