;
; ooRexx Install Script, based on Modern Example Script Written by Joost Verburg
;
; This script requires 2 plugins added to your NISI installation.  They need to
; be put into ${NSISDIR}\Plugins directory:
;   services.dll       -> http://nsis.sourceforge.net/File:Services.zip
;   ooRexxProcess.dll  -> http://sourceforge.net/projects/oorexx/files/  under oorexx-buildutils
;
; Run as:
;  makensis /DVERSION=x.x /DNODOTVER=xx /DSRCDIR=xxx /DBINDIR=xxx /DCPU=xxx oorexx.nsi
;  eg
;  makensis /DVERSION=4.0.0 /DNODOTVER=400 /DSRCDIR=d:\oorexx\oorexx /DBINDIR=d:\oorexx\oorexx\win32rel /DCPU=x64 oorexx.nsi
; Note:
;  oorexx.nsi MUST be in the current directory.

!define LONGNAME "Open Object Rexx"  ;Long Name (for descriptions)
!define SHORTNAME "ooRexx"           ;Short name (no slash) of package
!define DISPLAYICON "$INSTDIR\rexx.exe,0"
!define UNINSTALLER "uninstall.exe"
!define KEYFILE1     "rexx.exe"
!define KEYFILE2     "rxapi.dll"

Name "${LONGNAME} ${VERSION}"

!include "MUI2.nsh"
!include "Library.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "WordFunc.nsh"
!include "admin.nsh"
!include "isnt.nsh"
!include "newpath.nsh"
!include "WriteEnv.nsh"


!define MUI_ICON "${SRCDIR}\platform\windows\rexx.ico"
!define MUI_UNICON "${SRCDIR}\platform\windows\install\uninstall.ico"

!define MUI_CUSTOMPAGECOMMANDS
!define MUI_WELCOMEFINISHPAGE_BITMAP "orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "orange-uninstall.bmp"

!define MUI_LICENSEPAGE
!define MUI_COMPONENTSPAGE
!define MUI_DIRECTORYPAGE
!define MUI_FINISHPAGE
!define MUI_FINISHPAGE_NOAUTOCLOSE

!define MUI_ABORTWARNING

!define MUI_UNINSTALLER
!define MUI_UNCONFIRMPAGE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

;define MUI_CUSTOMFUNCTION_GUIINIT myGuiInit

!define UninstLog "uninstall.log"
Var UninstLog

!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"

!macro File FilePath FileName
 FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"

!macro CreateDirectory Path
 CreateDirectory "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"

!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"

!macro WriteUninstaller Path
 WriteUninstaller "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"

;--------------------------------
;Configuration

  ;General
  OutFile "${SHORTNAME}${NODOTVER}-${CPU}.exe"
  ShowInstdetails show
  ShowUninstDetails show
  SetOverwrite on
  SetPluginUnload alwaysoff
  RequestExecutionLevel admin
  InstallDir "$PROGRAMFILES\${SHORTNAME}"
;--------------------------------
;Pages
  /* Installer pages */
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "${SRCDIR}\CPLv1.0.txt"
  Page custom Uninstall_Old_ooRexx_page Uninstall_Old_ooRexx_Leave
  Page custom Uninstall_Type_page Uninstall_Type_Leave
  Page custom Ok_Stop_RxAPI_page Ok_Stop_RxAPI_leave

  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
;  Page custom SetCustomAssoc
;  Page custom SetCustomLanguage
  Page custom Rxapi_Options_page Rxapi_Options_leave
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  /* Uninstaller pages */
  !insertmacro MUI_UNPAGE_WELCOME
  UninstPage custom un.Ok_Stop_RxAPI_page un.Ok_Stop_RxAPI_leave
  UninstPage custom un.Uninstall_By_Log_page un.Uninstall_By_Log_leave
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
;--------------------------------
;Language
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserved files

;--------------------------------
; Variables
Var IsAdminUser                ; is the installer being run by an admin:           true / false
Var RxapiIsService             ; is rxapi installed as a service:                  true / false
Var RxapiIsRunning             ; is rxapi running:                                 true / false
Var RegVal_uninstallString     ; uninstall string (program) found in regsitry
Var RegVal_uninstallLocation   ; location of uninstall program found in registry
Var RegVal_uninstallVersion    ; Version / level of uninstaller program.  This only exists at 410 or greater
Var DoUpgrade                  ; try to do an upgrade install                      true / false
Var UpgradeTypeAvailable       ; Level of uninstaller sufficient for upgrade type  true / false

; Dialog variables
Var Dialog
Var Label_One
Var Label_Two
Var Uninstall_Previous_CK
Var Force_Install_CK
Var Do_Upgrade_Type_CK
Var RxAPI_Install_Service_CK
Var RxAPI_Start_CK
Var Delete_ooRexx_Tree_CK
Var RxAPIInstallService
Var RxAPIStartService

Var StopRxAPI_CK
Var StopRxAPI_CK_State

; Uninstall variables
Var LogFileExists
Var DeleteWholeTree

;===============================================================================
;Installer Sections
;===============================================================================

;-------------------------------------------------------------------------------
;  Hidden section to open the log file

Section -openlogfile
  CreateDirectory "$INSTDIR"
  IfFileExists "$INSTDIR\${UninstLog}" +3
    FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
    Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

;-------------------------------------------------------------------------------
; Core components

Section "${LONGNAME} Core (required)" SecMain
  SectionIn 1 RO
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR
  ; Distribution executables...
  ${File} "${BINDIR}\" "rexx.exe"
  ${File} "${BINDIR}\" "rexx.img"
  ${File} "${BINDIR}\" "rexxc.exe"
  ${File} "${BINDIR}\" "rxsubcom.exe"
  ${File} "${BINDIR}\" "rxqueue.exe"
  ${File} "${BINDIR}\" "rxapi.exe"
  ${File} "${BINDIR}\" "rexxhide.exe"
  ${File} "${BINDIR}\" "rexxpaws.exe"
  ; Distribution DLLs...
  ${File} "${BINDIR}\" "rexx.dll"
  ${File} "${BINDIR}\" "rexxapi.dll"
  ${File} "${BINDIR}\" "rexxutil.dll"
  ${File} "${BINDIR}\" "rxmath.dll"
  ${File} "${BINDIR}\" "rxsock.dll"
  ${File} "${BINDIR}\" "rxregexp.dll"
  ${File} "${BINDIR}\" "rxwinsys.dll"
  ${File} "${BINDIR}\" "oodialog.dll"
  ${File} "${BINDIR}\" "orexxole.dll"
  ${File} "${BINDIR}\" "hostemu.dll"
  ; CLASS files...
  ${File} "${BINDIR}\" "winsystm.cls"
  ${File} "${BINDIR}\" "socket.cls"
  ${File} "${BINDIR}\" "streamsocket.cls"
  ${File} "${BINDIR}\" "mime.cls"
  ${File} "${BINDIR}\" "smtp.cls"
  ${File} "${BINDIR}\" "rxregexp.cls"
  ${File} "${BINDIR}\" "rxftp.cls"
  ${File} "${BINDIR}\" "csvStream.cls"
  ${File} "${BINDIR}\" "orexxole.cls"
  ${File} "${BINDIR}\" "oodialog.cls"
  ${File} "${BINDIR}\" "oodwin32.cls"
  ${File} "${BINDIR}\" "oodplain.cls"

  ; rexxtry is technically a sample, but it is heavily used, so add it to
  ; the executables.  The same thing for the GUI version.
  ${File} "${SRCDIR}\samples\" "rexxtry.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\ooRexxTry\" "ooRexxTry.rex"

  ; Other files...
  ${File} "${SRCDIR}\platform\windows\" "rexx.ico"
  ${File} "${SRCDIR}\" "CPLv1.0.txt"

  ; readmes
  ${SetOutPath} $INSTDIR\doc
  ${File} "${SRCDIR}\doc\" "readme.pdf"
  File /oname=CHANGES.txt "${SRCDIR}\CHANGES"
  File /oname=ReleaseNotes.txt "${SRCDIR}\ReleaseNotes"
  ${AddItem} $INSTDIR\doc\CHANGES.txt
  ${AddItem} $INSTDIR\doc\ReleaseNotes.txt

  ; Set output path to the installation directory just in case
  SetOutPath $INSTDIR

;;;;  Comment out orxscrpt stuff temporarily
  /*
  ; orxscrpt.dll needs to be registered
  !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${BINDIR}\orxscrpt.dll" "$INSTDIR\orxscrpt.dll" "$INSTDIR"

  ; Stop rxapi.exe (again!) the registration process starts rxapi.exe.
  ooRexxProcess::killProcess "rxapi.exe"
  */

  ; Set the REXX_HOME environment variable
  Push "REXX_HOME"
  Push $INSTDIR
  Push $IsAdminUser ; "true" or "false"
  Call WriteEnvStr

  ; add the Install directory to the PATH env variable; either system wide or user-specific
  Push $INSTDIR
  Push $IsAdminUser ; "true" or "false"
  Push "PATH"
  Call AddToPath

  ; Add the Start Menu folder and start adding the items.
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Try Rexx.lnk" "$INSTDIR\rexx.exe" '"$INSTDIR\rexxtry.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Try Rexx.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Try Rexx (GUI).lnk" "$INSTDIR\rexx.exe" '"$INSTDIR\ooRexxTry.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Try Rexx (GUI).lnk"

  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}\Documentation"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx README.lnk" "$INSTDIR\doc\readme.pdf" "" "$INSTDIR\doc\readme.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx README.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx CHANGES.lnk" "$INSTDIR\doc\CHANGES.txt" "" "$INSTDIR\doc\CHANGES.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx CHANGES.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ReleaseNotes.lnk" "$INSTDIR\doc\ReleaseNotes.txt" "" "$INSTDIR\doc\ReleaseNotes.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ReleaseNotes.lnk"

  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Uninstall ${SHORTNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Uninstall ${SHORTNAME}.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\LICENSE.lnk" "$INSTDIR\CPLv1.0.txt" "" "$INSTDIR\CPLv1.0.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\LICENSE.lnk"
  WriteINIStr "$SMPROGRAMS\${LONGNAME}\ooRexx Home Page.url" "InternetShortcut" "URL" "http://www.oorexx.org/"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\ooRexx Home Page.url"

  ; Write the uninstall keys
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "InstallLocation" '"$INSTDIR"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayName" "${LONGNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayIcon" "${DISPLAYICON}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "HelpLink" "http://www.rexxla.org/support.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=119701"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLInfoAbout" "http://www.rexxla.org/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "Publisher" "Rexx Language Association"

  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation" "$INSTDIR" ; dont quote it
  WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallVersion" "${VERSION}"
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoModify" 0x00000001
  WriteRegDWORD     HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoRepair" 0x00000001
  ${WriteUninstaller} "$INSTDIR\${UNINSTALLER}"

  ; Associate .rex with ooRexx (REXXScript)
  Push 1
  Push ".rex"
  Call DoFileAssociation

  ; Add .rex to the PATHEXT
  Push ".rex"
  Call AddToPathExt

  ; If an administrator, install rxapi as a service depending on what the user
  ; selected.
  ${if} $IsAdminUser == "true"
    Call InstallRxapi
  ${endif}

SectionEnd

;-------------------------------------------------------------------------------
; Sample programs

Section "${LONGNAME} Samples" SecDemo
  DetailPrint "********** Samples **********"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples
  ${File} "${SRCDIR}\samples\" "rexxcps.rex"
  ${File} "${SRCDIR}\samples\" "ccreply.rex"
  ${File} "${SRCDIR}\samples\" "complex.rex"
  ${File} "${SRCDIR}\samples\" "factor.rex"
  ${File} "${SRCDIR}\samples\" "greply.rex"
  ${File} "${SRCDIR}\samples\" "guess.rex"
  ${File} "${SRCDIR}\samples\" "ktguard.rex"
  ${File} "${SRCDIR}\samples\" "makestring.rex"
  ${File} "${SRCDIR}\samples\" "month.rex"
  ${File} "${SRCDIR}\samples\" "philfork.rex"
  ${File} "${SRCDIR}\samples\" "pipe.rex"
  ${File} "${SRCDIR}\samples\" "properties.rex"
  ${File} "${SRCDIR}\samples\" "qdate.rex"
  ${File} "${SRCDIR}\samples\" "qtime.rex"
  ${File} "${SRCDIR}\samples\" "scclient.rex"
  ${File} "${SRCDIR}\samples\" "scserver.rex"
  ${File} "${SRCDIR}\samples\" "semcls.rex"
  ${File} "${SRCDIR}\samples\" "sfclient.rex"
  ${File} "${SRCDIR}\samples\" "sfserver.rex"
  ${File} "${SRCDIR}\samples\" "stack.rex"
  ${File} "${SRCDIR}\samples\" "usecomp.rex"
  ${File} "${SRCDIR}\samples\" "usepipe.rex"
  ${File} "${SRCDIR}\samples\windows\rexutils\" "drives.rex"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\misc
  ${File} "${SRCDIR}\samples\windows\misc\" "fileDrop.empty"
  ${File} "${SRCDIR}\samples\windows\misc\" "fileDrop.input"
  ${File} "${SRCDIR}\samples\windows\misc\" "fileDrop.readMe"
  ${File} "${SRCDIR}\samples\windows\misc\" "fileDrop.rex"
  ${CreateDirectory} $INSTDIR\samples\ole
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\adsi
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\adsi\" "*.rex"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\apps
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "MSAccessDemo.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "oleUtils.frm"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp01.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp02.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp03.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp04.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp05.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp06.mwp"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp06.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp07.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp08.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp09.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp10.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp11.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp12.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp13.rex"
  ${File} "${SRCDIR}\samples\windows\ole\apps\" "samp14.rex"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\methinfo
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\methinfo\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\ole\methinfo\" "*.cls"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\oleinfo
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\oleinfo\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\ole\oleinfo\" "*.txt"
  ${File} "${SRCDIR}\samples\windows\ole\oleinfo\" "*.bmp"
  ${File} "${SRCDIR}\samples\windows\ole\oleinfo\" "*.rc"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\wmi
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\wmi\" "*.rex"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\ole\wmi\sysinfo
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\ole\wmi\sysinfo\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\ole\wmi\sysinfo\" "*.rc"
;;; Temporarily block out the orxscrpt samples
;;;   ; Set output path to the installation directory.
;;;   SetOutPath $INSTDIR\samples\wsh
;;;   ; Distribution files...
;;;   ${File} "${SRCDIR}\samples\windows\wsh\" "*.rex"
;;;   ${File} "${SRCDIR}\samples\windows\wsh\" "*.htm"
;;;   ${File} "${SRCDIR}\samples\windows\wsh\" "*.wsf"
;;;   ${File} "${SRCDIR}\samples\windows\wsh\" "*.wsc"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\winsystem
  ${File} "${SRCDIR}\samples\windows\winsystem\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\winsystem\" "*.rc"
  ${File} "${SRCDIR}\samples\windows\winsystem\" "*.h"
  ${File} "${SRCDIR}\samples\windows\winsystem\" "*.frm"
  ; Create start menu shortcuts
  SetOutPath $INSTDIR\samples
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\RexxCPS.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\rexxcps.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\RexxCPS.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Quick Date.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\qdate.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Quick Date.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Quick Time.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\qtime.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Quick Time.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Display Event Log.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\winsystem\eventlog.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Display Event Log.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Display Drive Info.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\drives.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Display Drive Info.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Windows Manager.lnk" "$INSTDIR\rexxhide.exe" '"$INSTDIR\samples\winsystem\usewmgr.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\Windows Manager.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\MS Access.lnk" "$INSTDIR\rexxpaws.exe" '"$INSTDIR\samples\ole\apps\MSAccessDemo.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\MS Access.lnk"
  ;
  ; OOdialog samples
  ;
  ${SetOutPath} $INSTDIR\samples\oodialog
  ; Distribution files...
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "oodialog.ico"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.h"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.inp"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.ico"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\bmp
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\bmp\" "*.bmp"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\controls
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\controls\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\controls\" "*.rc"
  ${File} "${SRCDIR}\samples\windows\oodialog\controls\" "*.h"
  ${File} "${SRCDIR}\samples\windows\oodialog\controls\" "*.txt"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\examples
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\examples\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\examples\" "*.txt"
  ${SetOutPath} $INSTDIR\samples\oodialog\ooRexxTry
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\ooRexxTry\" "ooRexxTry.rex"
  ${SetOutPath} $INSTDIR\samples\oodialog\ooRexxTry\doc
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\ooRexxTry\doc\" "ooRexxTry.pdf"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\rc
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\rc\" "*.rc"
  ${File} "${SRCDIR}\samples\windows\oodialog\rc\" "*.h"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\res
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\res\" "*.res"
  ${File} "${SRCDIR}\samples\windows\oodialog\res\" "*.dll"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\simple
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\simple\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\simple\" "*.txt"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\wav
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\wav\" "*.wav"
  ${File} "${SRCDIR}\samples\windows\oodialog\wav\" "*.txt"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\examples\resources
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\examples\resources\" "*.bmp"
  ${File} "${SRCDIR}\samples\windows\oodialog\examples\resources\" "*.h"
  ${File} "${SRCDIR}\samples\windows\oodialog\examples\resources\" "*.rc"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\tutorial
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\tutorial\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\tutorial\" "*.bmp"
  ${File} "${SRCDIR}\samples\windows\oodialog\tutorial\" "*.rc"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\source
  ; Distribution files...
  ${File} "${BINDIR}\" "oodialog.cls"
  ${File} "${BINDIR}\" "oodwin32.cls"
  ${File} "${BINDIR}\" "oodplain.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "0_READ_ME_FIRST.txt"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "build_ooDialog_cls.rex"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "AnimatedButton.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "BaseDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "CategoryDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "ControlDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "DeprecatedClasses.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "DialogControls.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "DialogExtensions.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "DynamicDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "EventNotification.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "Menu.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "PlainBaseDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "PropertySheet.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "RcDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "ResDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "UserDialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "UtilityClasses.cls"
  ; Create start menu shortcuts
  SetOutPath $INSTDIR\samples\oodialog
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Samples.lnk" "$INSTDIR\rexxhide.exe" '"$INSTDIR\samples\oodialog\sample.rex"' "$INSTDIR\samples\oodialog\oodialog.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Samples.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Calculator.lnk" "$INSTDIR\rexxhide.exe" '"$INSTDIR\samples\oodialog\calculator.rex"' "$INSTDIR\samples\oodialog\oodialog.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Calculator.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Change Editor.lnk" "$INSTDIR\rexxhide.exe" '"$INSTDIR\samples\oodialog\editrex.rex"' "$INSTDIR\samples\oodialog\oodialog.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\Change Editor.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\FTYPE Changer.lnk" "$INSTDIR\rexxhide.exe" '"$INSTDIR\samples\oodialog\ftyperex.rex"' "$INSTDIR\samples\oodialog\oodialog.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\ooDialog\FTYPE Changer.lnk"
SectionEnd

;------------------------------------------------------------------------
; Development tools

Section "${LONGNAME} Development Kit" SecDev
  DetailPrint "********** Development Kit **********"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\api
  ; Distribution files...
  ${File} "${BINDIR}\" "rexx.lib"
  ${File} "${BINDIR}\" "rexxapi.lib"
  ${File} "${SRCDIR}\api\" "oorexxapi.h"
  ${File} "${SRCDIR}\api\" "rexx.h"
  ${File} "${SRCDIR}\api\" "oorexxerrors.h"
  ${File} "${SRCDIR}\api\" "rexxapidefs.h"
  ${File} "${SRCDIR}\api\platform\windows\" "rexxapitypes.h"
  ${File} "${SRCDIR}\api\platform\windows\" "rexxplatformapis.h"
  ${File} "${SRCDIR}\api\platform\windows\" "rexxplatformdefs.h"
  ;
  ; API samples
  ;
  ${SetOutPath} $INSTDIR\samples\api
  ${File} "${SRCDIR}\samples\windows\api\" "readme.txt"
  ; Set output path to the installation directory for callrxnt.
  ${SetOutPath} $INSTDIR\samples\api\callrxnt
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\callrxnt\" "backward.fnc"
  ${File} "${SRCDIR}\samples\windows\api\callrxnt\" "callrxnt.c"
  ${File} "${SRCDIR}\samples\windows\api\callrxnt\" "callrxnt.ico"
  ${File} "${SRCDIR}\samples\windows\api\callrxnt\" "callrxnt.mak"
  ${File} "${SRCDIR}\samples\windows\api\callrxnt\" "callrxnt.exe"
  ; Set output path to the installation directory for callrxwn.
  ${SetOutPath} $INSTDIR\samples\api\callrxwn
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "backward.fnc"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.c"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.h"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.ico"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.mak"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.exe"
  ${File} "${SRCDIR}\samples\windows\api\callrxwn\" "callrxwn.rc"
  ; Set output path to the installation directory for rexxexit.
  ${SetOutPath} $INSTDIR\samples\api\rexxexit
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\rexxexit\" "rexxexit.c"
  ${File} "${SRCDIR}\samples\windows\api\rexxexit\" "rexxexit.ico"
  ${File} "${SRCDIR}\samples\windows\api\rexxexit\" "rexxexit.mak"
  ${File} "${SRCDIR}\samples\windows\api\rexxexit\" "rexxexit.exe"
  ${File} "${SRCDIR}\samples\windows\api\rexxexit\" "testRexxExit"
  ; Set output path to the installation directory the wpipe examples.
  ${SetOutPath} $INSTDIR\samples\api\wpipe
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\wpipe\" "readme.txt"
  ; Set output path to the installation directory for wpipe 1.
  ${SetOutPath} $INSTDIR\samples\api\wpipe\wpipe1
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe1\" "rexxapi1.c"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe1\" "rexxapi1.def"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe1\" "apitest1.rex"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe1\" "rexxapi1.mak"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe1\" "rexxapi1.dll"
  ; Set output path to the installation directory for wpipe 2.
  ${SetOutPath} $INSTDIR\samples\api\wpipe\wpipe2
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe2\" "rexxapi2.c"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe2\" "rexxapi2.def"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe2\" "apitest2.rex"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe2\" "rexxapi2.mak"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe2\" "rexxapi2.dll"
  ; Set output path to the installation directory for wpipe 3.
  ${SetOutPath} $INSTDIR\samples\api\wpipe\wpipe3
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe3\" "rexxapi3.c"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe3\" "rexxapi3.def"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe3\" "apitest3.rex"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe3\" "rexxapi3.mak"
  ${File} "${SRCDIR}\samples\windows\api\wpipe\wpipe3\" "rexxapi3.dll"
  ;
  ; Create start menu shortcuts
  ;
  ; All three of these examples have files that the executable needs to locate
  ; in the same directory. The short cut menu item has to have the 'Start in:'
  ; field set to the directory of the executable. The 'SetOutPath' command is
  ; what controls that.
  ;
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API"
  SetOutPath $INSTDIR\samples\api\callrxnt
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx in a Console.lnk" "$INSTDIR\samples\api\callrxnt\callrxnt.exe" "" "$INSTDIR\samples\api\callrxnt\callrxnt.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx in a Console.lnk"
  SetOutPath $INSTDIR\samples\api\callrxwn
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx in a Window.lnk" "$INSTDIR\samples\api\callrxwn\callrxwn.exe" "" "$INSTDIR\samples\api\callrxwn\callrxwn.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx in a Window.lnk"
  SetOutPath $INSTDIR\samples\api\rexxexit
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx with Exits.lnk" "$INSTDIR\samples\api\rexxexit\rexxexit.exe" 'testRexxExit "189 8"' "$INSTDIR\samples\api\rexxexit\rexxexit.ico"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\${SHORTNAME} Samples\API\Call ooRexx with Exits.lnk"
SectionEnd

;------------------------------------------------------------------------
; Documentation

Section "${LONGNAME} Documentation" SecDoc
  DetailPrint "********** Documentation **********"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\doc
  ${File} "${SRCDIR}\doc\" "rexxpg.pdf"
  ${File} "${SRCDIR}\doc\" "rexxref.pdf"
  ${File} "${SRCDIR}\doc\" "rxmath.pdf"
  ${File} "${SRCDIR}\doc\" "rxsock.pdf"
  ${File} "${SRCDIR}\doc\" "rxftp.pdf"
  ${File} "${SRCDIR}\doc\" "oodialog.pdf"
  ${File} "${SRCDIR}\doc\" "winextensions.pdf"
  ${File} "${SRCDIR}\samples\windows\oodialog\ooRexxTry\doc\" "ooRexxTry.pdf"
  ; Create start menu shortcuts
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Reference.lnk" "$INSTDIR\doc\rexxref.pdf" "" "$INSTDIR\doc\rexxref.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Programming Guide.lnk" "$INSTDIR\doc\rexxpg.pdf" "" "$INSTDIR\doc\rexxpg.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Programming Guide.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Mathematical Functions Reference.lnk" "$INSTDIR\doc\rxmath.pdf" "" "$INSTDIR\doc\rxmath.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Mathematical Functions Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx TCP-IP Sockets Functions Reference.lnk" "$INSTDIR\doc\rxsock.pdf" "" "$INSTDIR\doc\rxsock.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx TCP-IP Sockets Functions Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx rxFTP Class Reference.lnk" "$INSTDIR\doc\rxftp.pdf" "" "$INSTDIR\doc\rxftp.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx rxFTP Class Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ooDIalog Method Reference.lnk" "$INSTDIR\doc\oodialog.pdf" "" "$INSTDIR\doc\oodialog.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ooDIalog Method Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Windows Extensions Reference.lnk" "$INSTDIR\doc\winextensions.pdf" "" "$INSTDIR\doc\winextensions.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx Windows Extensions Reference.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexxTry Reference.lnk" "$INSTDIR\doc\ooRexxTry.pdf" "" "$INSTDIR\doc\ooRexxTry.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexxTry Reference.lnk"
SectionEnd

;-------------------------------------------------------------------------------
;  Hidden section to close the log file

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd

Var PreviousVersionInstalled

;===============================================================================
;Installer Functions
;===============================================================================

Function .onInit
  ;  Called by the installer before any page is shown.  We use it to check for a
  ;  previously installed ooRexx and to set execution variables.

  ${if} ${CPU} == "x86_64"
    strcpy $INSTDIR "$PROGRAMFILES64\${SHORTNAME}"
  ${endif}

  ;
  ; Install as All Users if an admin
  ;
  Call IsUserAdmin
  Pop $IsAdminUser
  ${if} $IsAdminUser == "true"
    SetShellVarContext all
    StrCpy $RxAPIInstallService ${BST_CHECKED}
    StrCpy $RxAPIStartService ${BST_CHECKED}
  ${else}
    StrCpy $RxAPIInstallService ${BST_UNCHECKED}
    StrCpy $RxAPIStartService ${BST_UNCHECKED}
  ${endif}

  ReadRegStr $RegVal_uninstallString HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString"
  ReadRegStr $RegVal_uninstallLocation HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation"
  ReadRegStr $RegVal_uninstallVersion HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallVersion"

  ; Check for previous version and if the upgrade type of uninstall is available.
  ${if} $RegVal_uninstallString == ""
    StrCpy $PreviousVersionInstalled 'false'
    StrCpy $UpgradeTypeAvailable 'false'
  ${else}
    StrCpy $PreviousVersionInstalled 'true'

    ${if} $RegVal_uninstallLocation == ""
      ; No location in the registry, we'll try the installation directory.
      StrCpy $RegVal_uninstallLocation $INSTDIR
    ${endif}

    ${if} $RegVal_uninstallVersion == ""
      StrCpy $UpgradeTypeAvailable 'false'
    ${else}
      ${VersionCompare} $RegVal_uninstallVersion "4.1.0.0" $0
      ; Returned in $0: 0 == versions are equal, 1 == version is greater than 410, 2 == version is less than 410
      ${if} $0 < 2
        StrCpy $UpgradeTypeAvailable 'true'
      ${else}
        StrCpy $UpgradeTypeAvailable 'false'
      ${endif}
    ${endif}
  ${endif}

FunctionEnd

/** Uninstall_Old_ooRexx_page()  Custom page function.
 */
Function Uninstall_Old_ooRexx_page

  /* Skip this page if no previous version is present */
  ${if} $PreviousVersionInstalled == "false"
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT "Previous ooRexx Version." "A previous version of ${LONGNAME} is already installed."
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 40u \
    "A version of ${LONGNAME} is currently installed.  If the previous version is \
    not uninstalled it is almost certain to cause problems.$\n$\n\
    To uninstall the previous version check the check box. To skip the uninstall step, \
    uncheck the check box."
  Pop $Label_One

  ${NSD_CreateCheckBox} 0 44u 100% 8u "Uninstall previous version of ooRexx"
  Pop $Uninstall_Previous_CK
  ${NSD_Check} $Uninstall_Previous_CK
  ${NSD_OnClick} $Uninstall_Previous_CK ShowHideForceInstall

  ${NSD_CreateLabel} 0 70u 100% 48u \
    "There are very few cases where installing ${LONGNAME} without removing the previous \
    version will work.  If you think you know better than this, proceed with the installation. \
    However, the ${SHORTNAME} developers will not support this type of installation$\n$\n\
    To proceed with the install without uninstalling the previous version of ooRexx you must \
    check the Force install to continue check box."
  Pop $Label_Two
  ShowWindow $Label_Two ${SW_HIDE}

  ${NSD_CreateCheckBox} 0 122u 100% 8u "Force install to continue"
  Pop $Force_Install_CK
  ${NSD_Uncheck} $Force_Install_CK
  ShowWindow $Force_Install_CK ${SW_HIDE}

  nsDialogs::Show

FunctionEnd

/** ShowHideForceInstall()  Callback function.
 *
 * Called when the Unistall previous ooRexx version check box is clicked.
 *
 * If the check box is checked, the user wants to uninstall and we hide the
 * force check box.  If the user unchecks the uninstall check box we show the
 * foce check box.
 */
Function ShowHideForceInstall
	Pop $Uninstall_Previous_CK
	${NSD_GetState} $Uninstall_Previous_CK $0

	${If} $0 == 1
		ShowWindow $Label_Two ${SW_HIDE}
		ShowWindow $Force_Install_CK ${SW_HIDE}
	${Else}
		ShowWindow $Label_Two ${SW_SHOW}
		ShowWindow $Force_Install_CK ${SW_SHOW}
    ${NSD_Uncheck} $Force_Install_CK
	${EndIf}
FunctionEnd

/** Uninstall_Old_ooRexx_Leave()  Callback function.
 *
 * The installer calls this function when the user presses the Next button on
 * the uninstall old ooRexx version page.
 */
Function Uninstall_Old_ooRexx_Leave
	${NSD_GetState} $Uninstall_Previous_CK $0
	${NSD_GetState} $Force_Install_CK $1

  /* If the user said to not uninstall the previous and did not check force the
   * install, then send them back to the page.
   */
  ${if} $0 == 0
  ${andif} $1 == 0
    MessageBox MB_OK \
      "To bypass uninstalling the previous version of ooRexx you must check$\n\
      the Force install check box.  If it is not the intention to skip the$\n\
      uninstall step, then the Uninstall previous version must be checked.$\n$\n\
      Please check the appropriate check boxes to continue." \
      /SD IDOK
      Abort
  ${endif}

  /* If the user said to not uninstall the previous and to force the install,
   * then we skip the uninstall.  To signal this we set the
   * PreviousVersionInstalled variable to false.
   */
  ${if} $0 == 0
  ${andif} $1 == 1
    StrCpy $PreviousVersionInstalled 'false'
  ${endif}

FunctionEnd

Function Uninstall_Type_page

  /* Skip this page if no previous version is present */
  ${if} $PreviousVersionInstalled == "false"
    Abort
  ${endif}

  ${if} $UpgradeTypeAvailable == 'false'
    StrCpy $DoUpgrade 'false'
    Call Do_The_Uninstall
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT "Upgrade Previous ooRexx Version." "It is possible to do an 'upgrade' type of install."
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 64u \
    "An 'upgrade' type of install will remove all the previously installed files and install \
    the new version of ${SHORTNAME}.  However, it will not undo any of the environment, file \
    association, or registry settings done by the previous install.$\n$\n\
    Use this type of install when you have customized these types of settings and do not wish \
    them changed.$\n$\n\
    Note: with this type of install, the install location and other settings can NOT be changed.$\n$\n\"
  Pop $Label_One

  ${NSD_CreateCheckBox} 0 68u 100% 8u "Perform an upgrade type of install"
  Pop $Do_Upgrade_Type_CK

  nsDialogs::Show

FunctionEnd

/** Uninstall_Type_Leave()  Callback function.
 *
 * The installer calls this function when the user presses the Next button on
 * the uninstall type page.
 */
Function Uninstall_Type_Leave
	${NSD_GetState} $Do_Upgrade_Type_CK $0

  ${if} $0 == 1
    StrCpy $DoUpgrade 'true'
  ${else}
    StrCpy $DoUpgrade 'false'
  ${endif}

  Call Do_The_Uninstall

FunctionEnd

Function Do_The_Uninstall

  HideWindow
  ClearErrors

  /* the "_?=$RegVal_uninstallLocation" portion of the command sets the
   * uninstall dir to $R2 and *prevents* the uninstaller from running in the
   * temp dir, which is what we want.
   */
  ExecWait '$RegVal_uninstallString /U=$DoUpgrade _?=$RegVal_uninstallLocation' $0

  IfErrors UninstallErrors
  ${if} $0 == 0
    ; No errors, do a sanity check and finish up.
    IfFileExists "$INSTDIR\${KEYFILE1}" UninstallErrors
    IfFileExists "$INSTDIR\${KEYFILE2}" UninstallErrors
    Delete "$RegVal_uninstallLocation\${UNINSTALLER}"
    RMDir "$RegVal_uninstallLocation"

    BringToFront
    Goto NotInstalled
  ${endif}

  ${if} $0 == 1
    ; The user canceled the uninstall ...
    MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST \
               "You have elected to cancel the uninstall of the previous version of$\n\
               ${SHORTNAME}.  There are very few cases where installing ${LONGNAME}$\n\
               without removing the previous version will work and this installation$\n\
               will abort.$\n$\n\
               If you are determined to install ${SHORTNAME} without removing the$\n\
               previous version, rerun the installation and select to not uninstall$\n\
               the previous version.  Then select to force the installation." \
               /SD IDOK
    Goto DoAbort
  ${endif}

  ; Return code is 2 or greater.  2 is uninstall canceled by script, treat
  ; anything greater than 2 in the same way.
  MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST \
             "The uninstall of the previous version of ${SHORTNAME} was terminated by$\n\
             the uninstall script.  The most likely cause of this is that the rxapi$\n\
             process is running, either as a service or stand alone, and could not be$\n\
             stopped.$\n$\n\
             When the previous version of rxapi is not stopped, the new version of$\n\
             of ${SHORTNAME} can not be installed correctly.$\n$\n\
             The installation will abort.$\n$\n\
             You can:$\n$\n\
             1.) Contact the developers on the SourceForge project for help.$\n$\n\
             2.) Try stopping rxapi manually and rerunning the installation.  If$\n\
             rxapi is installed as a service, use the service manager to stop rxapi.$\n\
             Otherwise, use the task manager to stop the rxapi process.$\n$\n\
             3.) Elect to not stop rxapi and to not remove the previous version.  To do$\n\
             this, rerun the installation and click No when asked to uninstall the previous$\n\
             version.  Then click Yes when asked if you want to continue.  However, if$\n\
             you do this ${SHORTNAME} will not be installed correctly." \
             /SD IDOK
  Goto DoAbort

  UninstallErrors:
    MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST \
               "There were unexpected errors uninstalling the previous version$\n\
               of ${SHORTNAME}.  There are very few cases where installing ${LONGNAME}$\n\
               without removing the previous version will work and this installation$\n\
               will abort.  You should first try rerunning the installtion.$\n$\n\
               If this is the second attempt at installation and you still get this$\n\
               message, you can:$\n$\n\
               1.) Contact the developers on the SourceForge project for help.$\n$\n\
               2.) Try removing the previous version manually and rerun the installation.$\n$\n\
               3.) Elect to not remove the previous version.  To do this, rerun the installation$\n\
               and click No when asked to uninstall the previous version.  Then click Yes when$\n\
               asked if you want to continue.$\n$\n\
               In all cases, please report this problem to the ${SHORTNAME} project on SourceForge." \
               /SD IDOK
    Goto DoAbort

  DoAbort:
    Quit

  NotInstalled:
  ; The previous version is now uninstalled, so we set the variable to reflect that.
  StrCpy $PreviousVersionInstalled 'false'
FunctionEnd

/** Ok_Stop_RxAPI_page()  Custom page function.
 *
 * This is a custom page that is shown if rxapi is still running.  If rxapi.exe
 * is not running the page is skipped.  On the page, the user is asked for the
 * okay to stop rxapi and advised of possible consequences.  If the user says
 * no, the install will be aborted..
 */
Function Ok_Stop_RxAPI_page
  Call CheckIsRxapiService
  Call CheckIsRxapiRunning

  ${if} $RxapiIsRunning == 'false'
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT "The ${LONGNAME} memory manager (rxapi) is currently running" \
                               "A new version of rxapi can not be installed while rxapi is running."
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 112u \
    "A previous version of the ${LONGNAME} memory manager (rxapi) is currently running. \
    The new version of rxapi can not be installed while the previous version is running.$\n$\n\
    When the Stop rxapi check box is checked, the installation will stop rxapi before \
    proceeding.  If the check box is not checked, rxapi will not be stopped.$\n$\n\
    If rxapi is not stopped the install will be canceled.$\n$\n\
    If, and only if, there are Rexx programs running, stopping the memory manager could \
    possibly cause data loss.$\n$\n\
    If you are worried about this, please uncheck the Stop rxapi check box.  When the \
    install ends, stop all running Rexx programs, and rerun the installation program."

  Pop $Label_One

  ${NSD_CreateCheckBox} 0 116u 100% 15u "Stop rxapi"
  Pop $StopRxAPI_CK
  ${NSD_SetState} $StopRxAPI_CK 1

  ;GetFunctionAddress $0 un.CheckOkToStopRxapiBack
  ;nsDialogs::OnBack $0

  nsDialogs::Show

FunctionEnd

Function Ok_Stop_RxAPI_leave

  ${NSD_GetState} $StopRxAPI_CK $StopRxAPI_CK_State

  ${if} $StopRxAPI_CK_State == 0
    Quit
  ${endif}

  /*
   * If the user decided to force the installation without removing the previous
   * installation, then it is possible that rxapi is still installed as a
   * service.  Since stopping a service is the surest means of stopping rxapi,
   * we'll check that first.
   */
  ${if} $$RxapiIsService == 'true'
    Services::SendServiceCommand 'stop' 'RXAPI'
    Pop $R0
    ${if} $R0 == 'Ok'
      Goto NotRunning
    ${endif}
  ${else}
    ooRexxProcess::killProcess "rxapi.exe"
    Pop $R0
    ${if} $R0 == 0
      Goto NotRunning
    ${endif}
  ${endif}

  /* We'll try one more time to kill rxapi and capture the return code */
  ooRexxProcess::killProcess "rxapi.exe"
  Pop $R0
  ${if} $R0 == 0
    Goto NotRunning
  ${endif}

  MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST \
             "Can not determine conclusively that the Open Object Rexx memory manager (rxapi) is$\n\
             stopped.  Since a new version of rxapi can not be installed while rxapi is running,$\n\
             the installation will quit.$\n$\n\
             Please use the task manager to locate rxapi.exe and end that process, then restart the$\n\
             installation.  If you are uncomfortable with using the task manager, contact the$\n\
             ${SHORTNAME} developers on SourceForge for help.$\n$\n\
             If you report this problem, please not that the error code is $R0." /SD IDOK
    Quit

  NotRunning:
FunctionEnd

/** Rxapi_Options_page()  Custom page function.
 *
 * This function is used to display the custom page that asks the user if they
 * want to install rxapi as a service, and if they want rxapi started when the
 * installation finishes.  It checks if the user is an admin and skips the
 * page if not.  (Admin privileges are required to install a service. ? is that
 * true?)
 */
Function Rxapi_Options_page

  ${if} $IsAdminUser == 'false'
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT "The ooRexx rxapi process" "Install rxapi as a Windows Service"
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_OnBack} Rxapi_Options_leave

  ${NSD_CreateLabel} 0 0 100% 80u \
    "ooRexx starts a daemon process, rxapi.exe, the first time a Rexx program executes.  This \
    process manages all data that can persist across interpreter invocations or is used for \
    cross-process communications.  The rxapi process manages the Rexx data queues, the macrospace, \
    and all of the external function, subcommand handler and exit registrations.$\n$\n\
    On Windows, in all cases, it is best to install rxapi as a Windows service.  In Windows Vista \
    and later operating systems, it is strongly recommmended that rxapi is always installed as a \
    service."
  Pop $Label_One

  ${NSD_CreateCheckBox} 0 86u 100% 15u "Install rxapi as a Windows Service"
  Pop $RxAPI_Install_Service_CK
  ${NSD_SetState} $RxAPI_Install_Service_CK $RxAPIInstallService
  ${NSD_OnClick} $RxAPI_Install_Service_CK EnableStartService

  ${NSD_CreateCheckBox} 0 106u 100% 15u "Start the rxapi Service during installation"
  Pop $RxAPI_Start_CK
  ${NSD_SetState} $RxAPI_Start_CK $RxAPIStartService
  ${if} $RxAPIInstallService == 0
	  EnableWindow $RxAPI_Start_CK 0
  ${endif}

  nsDialogs::Show
FunctionEnd

/** EnableStartService()  Callback function.
 *
 * Called when the install rxap as a service check box is clicked.
 *
 * If the check box is checked, we enable the start the service check box, if it
 * is unchecked we disable the check box.
 */
Function EnableStartService
	Pop $RxAPI_Install_Service_CK
	${NSD_GetState} $RxAPI_Install_Service_CK $0

	${If} $0 == 1
	  EnableWindow $RxAPI_Start_CK 1
    ${NSD_SetState} $RxAPI_Start_CK $RxAPIStartService
	${Else}
	  EnableWindow $RxAPI_Start_CK 0
    ${NSD_Uncheck} $RxAPI_Start_CK
	${EndIf}
FunctionEnd

/** Rxapi_Options_leave()  Call back function.
 *
 * Called by the installer when the user clicks Next or Back buttons on the
 * Rxapi_Options_page page.  We set the RxAPI option variables to match what the
 * user picked.
 */
Function Rxapi_Options_leave
  ${NSD_GetState} $RxAPI_Install_Service_CK $RxAPIInstallService
  ${NSD_GetState} $RxAPI_Start_CK $RxAPIStartService
FunctionEnd

/** CheckIsRxapiService()
 *
 * Determines if rxapi is installed as a service.  On return the variable
 * RxapiIsService will be set to either 'true' or 'false'
 *
 */
Function CheckIsRxapiService
  services::IsServiceInstalled 'RXAPI'
  Pop $R0

  ${if} $R0 == 'Yes'
    StrCpy $RxapiIsService 'true'
  ${else}
    StrCpy $RxapiIsService 'false'
  ${endif}
FunctionEnd

/** CheckIsRxapirunning()
 *
 * Determines if a rxapi.exe process is running.  On return the variable
 * RxapiIsRunning is set to 'true', 'false' or 'unknown N' where N is an error
 * return code.
 *
 * TODO still need to test on Vista / Windows 7
 *
 */
Function CheckIsRxapiRunning
  ${if} $RxapiIsService == 'true'
    services::IsServiceRunning 'RXAPI'
    Pop $R0
    ${if} $R0 == 'Yes'
      StrCpy $RxapiIsRunning 'true'
    ${else}
      StrCpy $RxapiIsRunning 'false'
    ${endif}
  ${else}
    ooRexxProcess::findProcess "rxapi.exe"
    Pop $R0
    DetailPrint "ooRexxProcess::findProcess rc: $R0"
    ${if} $R0 == '0'
      StrCpy $RxapiIsRunning 'true'
    ${elseif} $R0 == '1'
      StrCpy $RxapiIsRunning 'false'
    ${else}
      StrCpy $RxapiIsRunning 'unknown $R0'
    ${endif}
  ${endif}
FunctionEnd

Function .onMouseOverSection

  !insertmacro MUI_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Installs the core components of ${LONGNAME} to the application folder."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDev} "Installs the files required to embed ${LONGNAME} into you C/C++ application."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDemo} "Install sample ${LONGNAME} programs."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} "Install ${LONGNAME} documentation."
 !insertmacro MUI_DESCRIPTION_END

FunctionEnd

Function DoFileAssociation
  ;  Does the file associations for ooRexx.  Right now we only do .rex and the
  ;  arguments are a little pointless.
  ;
  ;  TODO - I added the section to install into the user customizable area some
  ;         years ago.  Really what we should be doing, rather than this IsAdmin
  ;         stuff is deciding if this is a machine-wide install (All Users) or
  ;         a single-user install.
  ;
  ;  Usage:
  ;    Push <doIt>
  ;    Push <ext>
  ;
  ;  Here:
  ;    Pop $R1  -> $R1 contains the extension, .rex is the only case for now.
  ;    Pop $R0  -> $R0 contains doIt, 0 or 1.

  Pop $R1
  Pop $R0
  ${if} $R0 == 0
    Return
  ${endif}
  ${if} $R1 == ""
    Return
  ${endif}

  ClearErrors
  WriteRegStr HKCR $R1 "" "REXXScript"
  IfErrors TryCurrentUser
  DetailPrint "Registering $R1 extension to run with ooRexx for all users"
  Goto WriteHKCRRegKeys

  TryCurrentUser:
  ; Failed to write to the registy, try the user customizable area.
  ClearErrors
  WriteRegStr HKCU $R1 "" "REXXScript"
  IfErrors 0
    DetailPrint "Failed to register $R1 extension to run with ooRexx for all users and the current user"
    Return

  DetailPrint "Registering $R1 extension to run with ooRexx for the current user"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript" "" "ooRexx Rexx Program"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shell" "" "open"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\DefaultIcon" "" "$INSTDIR\rexx.exe,0"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shell\open" "" "Run"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shell\open\command" "" '"$INSTDIR\rexx.exe" "%1" %*'
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shell\edit" "" "Edit"
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shell\edit\command" "" 'notepad.exe "%1"'
  WriteRegStr HKCU "SOFTWARE\Classes\REXXScript\shellex\DropHandler" "" "{60254CA5-953B-11CF-8C96-00AA00B8708C}"
  Goto NotifyTheShell

  WriteHKCRRegKeys:
  WriteRegStr HKCR "REXXScript" "" "ooRexx Rexx Program"
  WriteRegStr HKCR "REXXScript\shell" "" "open"
  WriteRegStr HKCR "REXXScript\DefaultIcon" "" "$INSTDIR\rexx.exe,0"
  WriteRegStr HKCR "REXXScript\shell\open" "" "Run"
  WriteRegStr HKCR "REXXScript\shell\open\command" "" '"$INSTDIR\rexx.exe" "%1" %*'
  WriteRegStr HKCR "REXXScript\shell\edit" "" "Edit"
  WriteRegStr HKCR "REXXScript\shell\edit\command" "" 'notepad.exe "%1"'
  WriteRegStr HKCR "REXXScript\shellex\DropHandler" "" "{60254CA5-953B-11CF-8C96-00AA00B8708C}"


  NotifyTheShell:
  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
FunctionEnd

Function AddToPathExt
  ;  Adds the specified file extension to PATHEXT  Right now we only do .rex and
  ;  the argument is a little pointless.
  ;
  ;  @notes - This could be done for a single-user install, but remember that
  ;           the user specific PATHEXT *replaces* the system wide PATHEXT, so
  ;           we would need to read the system wide value and add the extension
  ;           to it.
  ;
  ;  Usage:
  ;    Push <ext>
  ;
  ;  Here:
  ;    Pop $R0  -> $R0 now contains the extension to add to PATHEXT.

  Pop $R0

  ${if} $IsAdminUser == "false"
    Return
  ${endif}

  Call IsNT
  Pop $1
  ${if} $1 == 0
    Return
  ${endif}

  DetailPrint "Adding the $R1 extension to PATHEXT"
  Push $R0
  Push $IsAdminUser      ; should only be "true" at this point
  Push "PATHEXT"
  Call AddToPath
FunctionEnd

/**
 */
Function InstallRxapi
  ;  Installs rxapi as a service, if the user elected to do so.

  ${if} $RxAPIInstallService == 1
    ; User asked to install rxapi as a service.
    DetailPrint "Installing rxapi as a Windows Service"
    nsExec::ExecToLog "$INSTDIR\rxapi /i /s"
    Pop $R0

    ${if} $R0 == 0
      DetailPrint "rxapi successfully installed as a Windows Service"
      ${if} $RxAPIStartService == 1
        ; User also asked to start the service.
        Services::SendServiceCommand 'start' 'RXAPI'
        Pop $R0

        ${if} $R0 == 'Ok'
          DetailPrint "The rxapi service was successfully sent the start command"
        ${else}
          MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST "Failed to start the ooRexx rxapi service: $R0\n" /SD IDOK
        ${endif}
      ${endif}
    ${else}
      MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST "Failed to install rxapi as a Windows Service: $R0\n" /SD IDOK
    ${endif}
  ${endif}
FunctionEnd

;===============================================================================
;  Uninstaller Sections
;===============================================================================

;
; Note this:  If the install was done by a non-admin user, the .rex file association may have been written
;             to the HKEY_CURRENT_USER\SOFTWARE\Classes area.  You would think that we need to check for
;             that and specifically delete those keys.  However, testing shows that the current code always
;             removes those keys.  Tested on a number of machines with a number of different users.

;-------------------------------------------------------------------------------
; Uninstall

Section "Uninstall"

  /*
   temporarily comment out orxscrpt stuff while it is disabled in the build.

   ; orxscrpt.dll needs to be degistered  NOTE WSH DLL name may have changed.
   !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\orxscrpt.dll"
   ;
   ; In the old WSH code, entering the orxscrpt.dll started up the interpreter.
   ; This may no longer be the case.  If it is, rxapi will need to be stopped
   ; again.
   ooRExxProcess::killProc "rxapi.exe"
  */

  ${if} $RxapiIsService == 'true'
    Call un.InstallRxapiService
    Pop $R0
    ${if} $R0 != 0
      MessageBox MB_OK|MB_ICONSTOP \
      "Unexpected error removing the rxapi service.$\n$\n\
      The uninstall will abort.  Report this error to$\n\
      the ${SHORTNAME} developers:$\n$\n\
      Uninstall as service failed: $R0" \
      /SD IDOK
      SetErrorLevel 3
      Quit
    ${endif}
  ${endif}

  ; Get rid of the file association
  Push ".rex"
  Call un.DeleteFileAssociation

  ; Remove .rex from PATHEXT
  Push ".rex"
  Call un.AddToPathExt

  ; remove directory from PATH
  Push $INSTDIR
  Push $IsAdminUser ; pushes "true" or "false"
  Push "PATH"
  Call un.RemoveFromPath

  ;
  DeleteRegKey HKCR "REXXScript"

  ; remove the REXX_HOME environment variable
  Push "REXX_HOME"
  Push $IsAdminUser ; "true" or "false"
  Call un.DeleteEnvStr

  ; Remove the installation stuff
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}"
  DeleteRegKey HKLM "SOFTWARE\${LONGNAME}"

  Call un.Delete_Installed_Files

SectionEnd

;===============================================================================
;  Uninstaller Functions
;===============================================================================

Function un.onInit
  ;  Called by the uninstaller program before any pages are shown.  We use it to
  ;  set up the execution variables.

  ${GetOptions} "$CMDLINE" "/U="  $R0
  ${if} $R0 == 'true'
    StrCpy $DoUpgrade 'true'
  ${else}
    StrCpy $DoUpgrade 'false'
  ${endif}

  ; UnInstall as All Users if an admin
  Call un.IsUserAdmin
  Pop $IsAdminUser
  ${if} $IsAdminUser == "true"
    SetShellVarContext all
  ${endif}

  StrCpy $StopRxAPI_CK_State 1
  Call un.CheckIsRxapiService
  Call un.CheckIsRxapiRunning
FunctionEnd

/** un.Delete_Installed_Files()
 *
 * Deletes the installed files in the manner specified by the user.  Either by
 * using the log file to delete only files installed by the previous
 * installaltion or by simply deleting the whole installation directory.
 */
Function un.Delete_Installed_Files

  ${if} $DeleteWholeTree == 'true'
    DetailPrint "Uninstall files by deleting the $INSTDIR directory tree"
    RMDir /r "$INSTDIR"
    DetailPrint "Removing all Start Menu short cuts by removing the $SMPROGRAMS\${LONGNAME} folder"
    RMDir /r "$SMPROGRAMS\${LONGNAME}"
  ${else}
    DetailPrint "Uninstall files using the install log file"
    Push $R0
    Push $R1
    Push $R2
    SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
    FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
    StrCpy $R1 0

    GetLineCount:
      ClearErrors
      FileRead $UninstLog $R0
      IntOp $R1 $R1 + 1
      IfErrors 0 GetLineCount

    LoopRead:
      FileSeek $UninstLog 0 SET
      StrCpy $R2 0
      FindLine:
      FileRead $UninstLog $R0
      IntOp $R2 $R2 + 1
      StrCmp $R1 $R2 0 FindLine

      StrCpy $R0 $R0 -2
      IfFileExists "$R0\*.*" 0 +3
        RMDir $R0  #is dir
      Goto +3
      IfFileExists $R0 0 +2
        Delete $R0 #is file

      IntOp $R1 $R1 - 1
      StrCmp $R1 0 LoopDone
      Goto LoopRead
    LoopDone:
    FileClose $UninstLog
    Delete "$INSTDIR\${UninstLog}"
    RMDir "$INSTDIR"
    Pop $R2
    Pop $R1
    Pop $R0
  ${endif}

FunctionEnd

/** un.InstallRxapiService()
 * Removes the rxapi service.  Only called if RxapiIsService is true.
 *
 * The uninstaller always quits right at the beginning if it can not stop the
 * rxapi.  So logically, rxapi should be stopped, and if it isn't, it should be
 * stoppable.
 *
 * Pushes a return code to the top of the stack.  0 for success, otherwise an
 * error code.
 */
Function un.InstallRxapiService
  Call un.CheckIsRxapiRunning
  ${if} $$RxapiIsRunning == 'true'
    Services::SendServiceCommand 'stop' 'rxapi'
    Pop $R0

    ${if} $R0 != 'Ok'
      ; Seems impossible to get here, but if we did, we'll try to kill rxapi.
      ; If that fails we give up.
      DetailPrint "Service Control Manager failed to stop rxapi, forcing termination"
      ooRexxProcess::killProcess 'rxapi'
      Pop $R0
      ${if} $R0 != 0
      ${andif} $R0 != 1
        Push $R0
        Return
      ${endif}
    ${endif}
  ${endif}

  DetailPrint "Uninstalling ooRexx rxapi Service"
  nsExec::ExecToLog "$INSTDIR\rxapi /u /s"
  Pop $R0

  ${if} $R0 != 0
    ; One reason for this could be that rxapi.exe, the file, has been deleted.
    ; We'll try to handle this case by having the service manager delete the
    ; service.  This can be done even if rxapi.exe is missing.
    Services::SendServiceCommand 'delete' 'rxapi'
    Pop $R0
    ${if} $R0 != 'Ok'
      DetailPrint "Failed to uninstall rxapi as a service.  Reason: $R0"
      Push $R0
      Return
    ${endif}
  ${endif}

  DetailPrint "Uninstalled rxapi as a service"
  Push 0
FunctionEnd

Function un.DeleteFileAssociation
  ; Top of stack has the file extension, only "REX" at this time.  Pop it off
  ; into R0.
  Pop $R0
  ReadRegStr $R1 HKCR "$R0" ""
  ${if} $R1 == "RexxScript"
    ; Only delete the .rex association if we own it, (we use RexxScript for the ftype name.)
    DeleteRegKey HKCR "$R0"
    DetailPrint "Deleted file association for $R0"
  ${endif}
FunctionEnd

Function un.AddToPathExt
  ;  Removes the specified file extension from PATHEXT  Right now we only do
  ;  .rex and ;  the argument is a little pointless.
  ;
  ;  @notes - This could be done for a single-user install, but remember that
  ;           ... what? - need to think about how to do this for a single user,
  ;           could easily hose the system for a user.  I.e., end up with no
  ;           programs running from the command line.
  ;
  ;  Usage:
  ;    Push <ext>
  ;
  ;  Here:
  ;    Pop $R0  -> $R0 now contains the extension to add to PATHEXT.

  Pop $R0

  ${if} $IsAdminUser == "false"
    Return
  ${endif}

  Call un.IsNT
  Pop $1
  ${if} $1 == 0
    Return
  ${endif}

  DetailPrint "Removing the $R0 extension from PATHEXT"
  Push $R0
  Push $IsAdminUser      ; should only be "true" at this point
  Push "PATHEXT"
  Call un.RemoveFromPath
FunctionEnd

Function un.CheckIsRxapiService
  services::IsServiceInstalled 'RXAPI'
  Pop $R0
  ${if} $R0 == 'Yes'
    StrCpy $RxapiIsService 'true'
  ${else}
    StrCpy $RxapiIsService 'false'
  ${endif}
FunctionEnd

Function un.CheckIsRxapiRunning
  ;
  ; Determines if rxapi.exe is running.
  ;
  ; TODO still need to test on Vista / Windows 7
  ;
  ${if} $RxapiIsService == 'true'
    services::IsServiceRunning 'RXAPI'
    Pop $R0
    ${if} $R0 == 'Yes'
      StrCpy $RxapiIsRunning 'true'
    ${else}
      StrCpy $RxapiIsRunning 'false'
    ${endif}
  ${else}
    ooRexxProcess::findProcess "rxapi.exe"
    Pop $R0
    DetailPrint "ooRexxProcess::findProcess rc: $R0"
    ${if} $R0 == '0'
      StrCpy $RxapiIsRunning 'true'
    ${elseif} $R0 == '1'
      StrCpy $RxapiIsRunning 'false'
    ${else}
      StrCpy $RxapiIsRunning 'unknown $R0'
    ${endif}
  ${endif}
FunctionEnd

/** un.Ok_Stop_RxAPI_page()  Custom page function.
 *
 * This is a custom page that follows the Welcome page.  If rxapi.exe is not
 * running the page is skipped.  If rxapi.exe is running, the user is asked for
 * the okay to stop it.  If the user says no, we quit the uninstall.
 */
Function un.Ok_Stop_RxAPI_page
  ${if} $RxapiIsRunning == 'false'
    Abort
  ${endif}

  !insertmacro MUI_HEADER_TEXT "The ooRexx rxapi process" "The ${LONGNAME} memory manager (rxapi) is currently running"
  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${NSD_CreateLabel} 0 0 100% 96u \
    "${SHORTNAME} can not be completely uninstalled while rxapi is running.$\n$\n\
    When the Stop rxapi check box is checked, the uninstall will stop rxapi before \
    proceeding.  If the check box is not checked, rxapi will not be stopped.$\n$\n\
    If rxapi is not stopped the uninstall will be canceled.$\n$\n\
    If, and only if, there are Rexx programs running, stopping the memory manager could \
    possibly cause data loss.$\n$\n\
    If you are worried about this, please uncheck the Stop rxapi check box.  When the \
    uninstall (or install) ends, stop all running Rexx programs, and rerun the uninstall, \
    (or install.)"

  Pop $Label_One

  ${NSD_CreateCheckBox} 0 100u 100% 15u "Stop rxapi"
  Pop $StopRxAPI_CK
  ${NSD_SetState} $StopRxAPI_CK $StopRxAPI_CK_State

  GetFunctionAddress $0 un.CheckOkToStopRxapiBack
  nsDialogs::OnBack $0

  nsDialogs::Show

FunctionEnd

/** un.Ok_Stop_RxAPI_leave()  Call back function.
 *
 * Invoked by the uninstaller when the user clicks Next on the Ok to stop rxapi
 * page.  We check if the user agreed to stopping rxapi.  If not we quit the
 * uninstaller.  Otherwise we stop rxapi.  If we can not stop rxapi, we also
 * quit the uninstaller.
 */
Function un.Ok_Stop_RxAPI_leave

  ${NSD_GetState} $StopRxAPI_CK $StopRxAPI_CK_State

  ${if} $StopRxAPI_CK_State == 0
    SetErrorLevel 1
    Quit
  ${endif}

  Call un.StopRxapi
  Pop $R0
  ${if} $R0 == 'Ok'
    Return
  ${endif}

  ; rxapi was not stopped, the error code is now at top of stack
  Pop $R0

  MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST \
             "Can not determine conclusively that rxapi is currently stopped.  It may be$\n\
             that you do not have sufficient privileges to uninstall ${SHORTNAME}.$\n\$\n\
             The uninstall will quit.$\n$\n\
             If you believe you do have sufficient privileges to do the uninstall,$\n\
             you can:$\n$\n\
             1.) Manually stop the rxapi process and restart the uninstall.  If rxapi$\n\
             is installed as a service, stop the service.  Otherwise, use the task$\n\
             manager to end the rxapi process.$\n$\n\
             2.) Ask the ${SHORTNAME} developers for help.  Please report this error code$\n\
             to the ${SHORTNAME} developers: killProcess $R0$\n$\n"\
             /SD IDOK
  SetErrorLevel 2
  Quit

FunctionEnd

Function un.CheckOkToStopRxapiBack
  ${NSD_GetState} $StopRxAPI_CK $StopRxAPI_CK_State
FunctionEnd

/** un.Uninstall_By_Log_page()  Custom page function.
 *
 * This is a custom page that follows the page that checks if it is okay to
 * stop rxapi.  Here we check for the uninstall log.  We then give the user the
 * option of simply deleting the ooRexx directory tree (50 times faster on my
 * system) or using the log file to individually delete only the install files.
 *
 * If the uninstall log is missing, the user does not have the option of
 * deleting individual files and directories.
 */
Function un.Uninstall_By_Log_page

  StrCpy $LogFileExists 'false'
  IfFileExists "$INSTDIR\${UninstLog}" 0 +4
    StrCpy $LogFileExists 'true'

  ${if} $LogFileExists == 'false'
    !insertmacro MUI_HEADER_TEXT \
      "${UninstLog} NOT found" \
      "The option of only removing files installed by the prior ooRexx installer is not available."

    StrCpy $0 \
      "Because the ${UninstLog} file is missing, the uninstall process must remove all \
      files in the $INSTDIR directory tree.$\n$\n\
      WARNING: This will remove all folders and files in the $INSTDIR folder, including \
      any folders or files not placed there by the ooRexx installation.$\n$\n\
      If there are any personl folders or files in the $INSTDIR directory tree that need \
      to be saved, please cancel the uninstall, move the files, and restart the uninstall \
      program."
  ${else}
    !insertmacro MUI_HEADER_TEXT \
      "Choose the method for removing installed files" \
      "Delete only installed files or delete entire directory tree?"

    StrCpy $0 \
      "The uninstall program can use an install log to delete only the folders and files \
      placed in the $INSTDIR directory tree by the original installation program.$\n$\n\
      Optionally, the entire $INSTDIR directory tree can be deleted.$\n$\n\
      WARNING: Deleting the entire directory tree will remove all folders and files in the \
      $INSTDIR folder.  This will include any folders or files not placed there by the ooRexx \
      installation."

  ${endif}

  nsDialogs::Create /NOUNLOAD 1018
  Pop $Dialog

  ${If} $Dialog == error
    Abort
  ${EndIf}

  ${if} $LogFileExists == 'false'
    ${NSD_CreateLabel} 0 0 100% 80u $0
    Pop $Label_One

    ${NSD_CreateCheckBox} 0 84u 100% 8u "Delete entire directory tree"
    Pop $Delete_ooRexx_Tree_CK
    ${NSD_Check} $Delete_ooRexx_Tree_CK
    EnableWindow $Delete_ooRexx_Tree_CK 0
  ${else}
    ${NSD_CreateLabel} 0 0 100% 64u $0
    Pop $Label_One

    ${NSD_CreateLabel} 0 80u 100% 16u "To DELETE the entire $INSTDIR directory tree, check the check box."
    Pop $Label_Two

    ${NSD_CreateCheckBox} 0 100u 100% 8u "Delete entire directory tree"
    Pop $Delete_ooRexx_Tree_CK
  ${endif}

  nsDialogs::Show

FunctionEnd

/** un.Uninstall_By_Log_leave()  Call back function.
 *
 * Invoked by the uninstaller when the user clicks Next on the uninstall using
 * the log page.
 */
Function un.Uninstall_By_Log_leave

  ${NSD_GetState} $Delete_ooRexx_Tree_CK $0

  ${if} $0 == 1
    StrCpy $DeleteWholeTree 'true'
  ${else}
    StrCpy $DeleteWholeTree 'false'
  ${endif}

FunctionEnd

Function un.StopRxapi
  ;  Stop rxapi.exe. If rxapi is installed as a service, the stop command should
  ;  stop it.  Otherwise, we use killProcess.
  ;
  ;  On return top of stack will contain 'Ok' for success or 'Error'.  If there
  ;  was an error top of stack minus 1 will contain the error code from
  ;  ooRexx::killProcess.

  ${if} $RxapiIsService == 'true'
    Services::SendServiceCommand 'stop' 'RXAPI'
    Pop $R0
  ${else}
    ooRexxProcess::killProcess 'rxapi'
    Pop $R0
  ${endif}

  StrCpy $4 $R0 ; TEMP
  Call un.CheckIsRxapiRunning  ; TODO this function seems to return true sometimes when rxapi is not running
  ${if} $RxapiIsRunning == 'false'
    Push 'Ok'
    return
  ${endif}

  MessageBox MB_OK "un.StopRxapi AFTER stop checkIsRxapiRunning reports true RxapiIsService=$RxapiIsRunning popped R0=$4"

  ; Still running, try one more time, although this is probably a waste of time.
  ; But, we will capture the error code for debugging.  Note that if the return
  ; from killProcess is 1, then the process was not found, so it is killed.  See
  ; the above TODO
  ooRexxProcess::killProcess 'rxapi'
  Pop $R0
  ${if} $R0 == 0
  ${orif} $R1 == 1
    Push 'Ok'
  ${else}
    Push $R0
    Push 'Error'
  ${endif}
FunctionEnd

;eof
