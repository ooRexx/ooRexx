;
; ooRexx Install Script, based on Modern Example Script Written by Joost Verburg
; Requires: ${NSISDIR}\Plugins\FindProcDll.dll - http://nsis.sourceforge.net/FindProcDLL_plug-in
; Requires: ${NSISDIR}\Plugins\KillProcDll.dll - http://nsis.sourceforge.net/KillProcDLL_plug-in
; Requires: ${NSISDIR}\Plugins\services.dll    - http://nsis.sourceforge.net/File:Services.zip
; Run as:
;  makensis /DVERSION=x.x /DNODOTVER=xx /DSRCDIR=xxx /DBINDIR=xxx /DCPU=xxx oorexx.nsi
;  eg
;  makensis /DVERSION=4.0.0 /DNODOTVER=400 /DSRCDIR=d:\oorexx\oorexx /DBINDIR=d:\oorexx\oorexx\win32rel /DCPU=x64 oorexx.nsi
; Note:
;  oorexx.nsi MUST be in the current directory!

!define LONGNAME "Open Object Rexx"  ;Long Name (for descriptions)
!define SHORTNAME "ooRexx" ;Short name (no slash) of package
!define DISPLAYICON "$INSTDIR\rexx.exe,0"
!define UNINSTALLER "uninstall.exe"
!define KEYFILE     "rexx.exe"

!define MUI_ICON "${SRCDIR}\platform\windows\rexx.ico"
!define MUI_UNICON "${SRCDIR}\platform\windows\install\uninstall.ico"

Name "${LONGNAME} ${VERSION}"

!include "MUI.nsh"
!include "Library.nsh"

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

Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

;--------------------------------
;Configuration

  ;General
  OutFile "${SHORTNAME}${NODOTVER}-${CPU}.exe"
  ShowInstdetails show
  SetOverwrite on
  SetPluginUnload alwaysoff
  RequestExecutionLevel admin

  ;Folder-select dialog
  InstallDir "$PROGRAMFILES\${SHORTNAME}"

  LangString TEXT_IO_PAGETITLE_RXAPI ${LANG_ENGLISH} "The ooRexx rxapi process"
  LangString TEXT_IO_SUBTITLE_RXAPI ${LANG_ENGLISH} "Install rxapi as a Windows Service"
;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "${SRCDIR}\CPLv1.0.txt"
  !define MUI_PAGE_CUSTOMFUNCTION_PRE CheckForRxAPI
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
;  Page custom SetCustomAssoc
;  Page custom SetCustomLanguage
  Page custom SetCustomRxAPI
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE un.CheckForRxAPI
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
;--------------------------------
;Language
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserved files

  ReserveFile "oorexx_ss.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
; Variables
Var rxapichk
Var IsAdminUser

;========================================================================
;Installer Sections

;------------------------------------------------------------------------
; Core

Section "${LONGNAME} Core (required)" SecMain
  SectionIn 1 RO
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR
  ; Set the REXX_HOME environment variable
  Push "REXX_HOME"
  Push $INSTDIR
  Push $IsAdminUser ; "true" or "false"
  Call WriteEnvStr
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
  ; CLASS files...
  ${File} "${BINDIR}\" "winsystm.cls"
  ${File} "${BINDIR}\" "socket.cls"
  ${File} "${BINDIR}\" "rxregexp.cls"
  ${File} "${BINDIR}\" "rxftp.cls"
  ${File} "${BINDIR}\" "orexxole.cls"
  ${File} "${BINDIR}\" "oodialog.cls"
  ${File} "${BINDIR}\" "oodwin32.cls"
  ${File} "${BINDIR}\" "oodplain.cls"
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}"
  ; rexxtry is technically a sample, but it is heavily used, so add it to
  ; the executables.  The same thing for the GUI version.
  ${File} "${SRCDIR}\samples\" "rexxtry.rex"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Try Rexx.lnk" "$INSTDIR\rexx.exe" '"$INSTDIR\rexxtry.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Try Rexx.lnk"
  ${File} "${SRCDIR}\samples\windows\oodialog\ooRexxTry\" "ooRexxTry.rex"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Try Rexx (GUI).lnk" "$INSTDIR\rexx.exe" '"$INSTDIR\ooRexxTry.rex"' "$INSTDIR\rexx.exe"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Try Rexx (GUI).lnk"
  ; Other files...
  ${File} "${SRCDIR}\platform\windows\" "rexx.ico"
  ${File} "${SRCDIR}\" "CPLv1.0.txt"
  ; readmes
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\doc
  ${File} "${SRCDIR}\doc\" "readme.pdf"
  File /oname=CHANGES.txt "${SRCDIR}\CHANGES"
  File /oname=ReleaseNotes.txt "${SRCDIR}\ReleaseNotes"
  ${AddItem} $INSTDIR\doc\CHANGES.txt
  ${AddItem} $INSTDIR\doc\ReleaseNotes.txt
  ${CreateDirectory} "$SMPROGRAMS\${LONGNAME}\Documentation"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx README.lnk" "$INSTDIR\doc\readme.pdf" "" "$INSTDIR\doc\readme.pdf" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx README.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx CHANGES.lnk" "$INSTDIR\doc\CHANGES.txt" "" "$INSTDIR\doc\CHANGES.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx CHANGES.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ReleaseNotes.lnk" "$INSTDIR\doc\ReleaseNotes.txt" "" "$INSTDIR\doc\ReleaseNotes.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Documentation\ooRexx ReleaseNotes.lnk"

;;;;  Comment out orxscrpt stuff temporarily
  ; Set output path to the installation directory just in case
;;  SetOutPath $INSTDIR
  ; orxscrpt.dll needs to be registered
;;  !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${BINDIR}\orxscrpt.dll" "$INSTDIR\orxscrpt.dll" "$INSTDIR"
  ;
  ; Stop rxapi.exe (again!) the registration process starts rxapi.exe GRRRR!!!
;;  KillProcDLL::KillProc "rxapi.exe"

  ; add the Install directory to the PATH env variable; either system wide or user-specific
  Push $INSTDIR
  Push $IsAdminUser ; "true" or "false"
  Push "PATH"
  Call AddToPath
  ; Add Start Menu items
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\Uninstall ${SHORTNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\Uninstall ${SHORTNAME}.lnk"
  CreateShortCut "$SMPROGRAMS\${LONGNAME}\LICENSE.lnk" "$INSTDIR\CPLv1.0.txt" "" "$INSTDIR\CPLv1.0.txt" 0
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\LICENSE.lnk"
  WriteINIStr "$SMPROGRAMS\${LONGNAME}\ooRexx Home Page.url" "InternetShortcut" "URL" "http://www.oorexx.org/"
  ${AddItem} "$SMPROGRAMS\${LONGNAME}\ooRexx Home Page.url"
  ; Write the uninstall keys
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayName" "${LONGNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayIcon" "${DISPLAYICON}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "HelpLink" "http://www.rexxla.org/support.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLUpdateInfo" "http://sourceforge.net/project/showfiles.php?group_id=119701"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "URLInfoAbout" "http://www.rexxla.org/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "Publisher" "Rexx Language Association"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString" '"$INSTDIR\${UNINSTALLER}"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "InstallLocation" '"$INSTDIR"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation" "$INSTDIR" ; dont quote it
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoModify" 0x00000001
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "NoRepair" 0x00000001
  ${WriteUninstaller} "$INSTDIR\${UNINSTALLER}"

  ; associate .REX with ooRexx (REXXScript)
  StrCpy $5 0
  Push 1
  Push ".REX"
  Call DoFileAssociation
  Call DoFileAssociationDetails

  ; read the result of the custom rxapi page if an administrator
  StrCmp $IsAdminUser "false" NotAdmin
  ReadIniStr $R0 "$PLUGINSDIR\oorexx_ss.ini" "Field 2" State
  ReadIniStr $R1 "$PLUGINSDIR\oorexx_ss.ini" "Field 3" State
  Push $R0
  Push $R1
  Call Installrxapi
  NotAdmin:
SectionEnd

;------------------------------------------------------------------------
; Demos

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
  ${File} "${SRCDIR}\samples\windows\" "philfork.rex"
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
  ${File} "${BINDIR}\" "oodialog.cls"
  ${File} "${BINDIR}\" "oodwin32.cls"
  ${File} "${BINDIR}\" "oodplain.cls"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.rex"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.inp"
  ${File} "${SRCDIR}\samples\windows\oodialog\" "*.ico"
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\bmp
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\bmp\" "*.bmp"
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
  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\samples\oodialog\res
  ; Distribution files...
  ${File} "${SRCDIR}\samples\windows\oodialog\res\" "*.res"
  ${File} "${SRCDIR}\samples\windows\oodialog\res\" "*.dll"
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
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "advctrl.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "anibuttn.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "basedlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "build.rex"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "catdlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "dialog.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "dlgext.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "dyndlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "makedll.bat"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "msgext.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "oodutils.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "plbdlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "pludlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "propsht.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "resdlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "stddlg.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "stdext.cls"
  ${File} "${SRCDIR}\extensions\platform\windows\oodialog\" "userdlg.cls"
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
; Doco

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


Section ""

  ;Invisible section to display the Finish header
; !insertmacro MUI_FINISHHEADER

SectionEnd

;========================================================================
;Installer Functions

Function .onInit
;  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "oorexx_fa.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "oorexx_ss.ini"
;  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "oorexx_mt.ini"

  StrCmp ${CPU} "x86_64" 0 +2
    strcpy $INSTDIR "$PROGRAMFILES64\${SHORTNAME}"

  ;
  ; Uninstall previous version if present
  ;
  ReadRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UninstallString"
  StrCmp $R1 "" NoUninstall
    ;
    ; ask the user to run the uninstaller
    ;
    ReadRegStr $R2 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${SHORTNAME}" "UnInstallLocation"
    StrCmp $R2 "" NoUninstallLocation
    Goto StartUninstall
    NoUninstallLocation:
      StrCpy $R2 $INSTDIR
    StartUninstall:
    MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_TOPMOST "A version of ${LONGNAME} is currently installed.$\nIt is recommended that it be uninstalled before proceeding.$\nUninstall previous version?" /SD IDYES IDNO NoUninstall IDCANCEL DoAbort
    HideWindow
    ClearErrors
    ExecWait '$R1 _?=$R2'
    IfErrors no_remove_uninstaller
    IfFileExists "$INSTDIR\${KEYFILE}" no_remove_uninstaller
      Delete "$R2\${UNINSTALLER}"
      RMDir "$R2"
    no_remove_uninstaller:
    BringToFront

    Goto NoUninstall
  DoAbort:
    Abort
  NoUninstall:
  ;
  ; Install as All Users if an admin
  ;
  Call IsUserAdmin
  Pop $IsAdminUser
  StrCmp $IsAdminUser "false" DefaultUser
  SetShellVarContext all
  DefaultUser:
  ;
  ; Initialise rxapichk
  ;
  StrCpy $rxapichk 0
  ;
FunctionEnd

; checks if the user is an admin and skips the page if not
Function SetCustomRxAPI
  StrCmp $IsAdminUser "true" NoAbort
  Abort
  NoAbort:
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_PAGETITLE_RXAPI)" "$(TEXT_IO_SUBTITLE_RXAPI)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "oorexx_ss.ini"
FunctionEnd

Function CheckForRxAPI
  ;
  ; Determines if rxapi.exe is running.  On Vista, FindProcDll does not work.
  ; Rather than fix FindProcDll, (easy to do, but I'm not sure of its license,)
  ; we first check if it is running as a service.  If so, we stop it.
  ;
  StrCmp $rxapichk 1 NotRunning
  StrCpy $rxapichk 1
  services::IsServiceRunning 'RXAPI'
  Pop $R0
  StrCmp $R0 'Yes' ServiceIsRunning
  ; Try FindProcDLL
  FindProcDLL::FindProc "rxapi.exe"
  StrCmp $R0 0 NotRunning
  ;
  ; rxapi.exe is running, we need to stop it
  ServiceIsRunning:
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION|MB_TOPMOST "The Open Object Rexx memory manager (RXAPI) is currently active.$\nSelect OK to stop it (possible loss of data) and continue.$\nSelect CANCEL to continue with the installation without stopping the memory manager." /SD IDOK IDCANCEL NotRunning
  ;
  ; Stop rxapi.exe.  Send the service stop command first.  If it is not a
  ; service, we don't care, just try kill.
  Services::SendServiceCommand 'stop' 'RXAPI'
  Pop $R0
  StrCmp $R0 'Ok' NotRunning
  KillProcDLL::KillProc "rxapi.exe"
  DetailPrint "rc from KillProcDll $R0"

  NotRunning:
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
  Pop $R1
  Pop $R0
  Strcmp $R0 0 exitfa
  Strcmp $R1 "" exitfa
  ; do the association
  DetailPrint "Registering $R1 extension to run with ooRexx"
  WriteRegStr HKCR $R1 "" "REXXScript"
  ;
  ; Append $R1 to .PATHEXT for NT-based systems, for Administrators only
  ;
  Call IsNT
  Pop $1
  StrCmp $1 0 exitfa
  StrCmp $IsAdminUser "false" exitfa
  Push $R1
  Push $IsAdminUser ; should only be "true" at this point
  Push "PATHEXT"
  Call AddToPath
  exitfa:
  Return
FunctionEnd

Function DoFileAssociationDetails
  ; do the association details
  WriteRegStr HKCR "REXXScript" "" "ooRexx Rexx Program"
  WriteRegStr HKCR "REXXScript\shell" "" "open"
  WriteRegStr HKCR "REXXScript\DefaultIcon" "" "$INSTDIR\rexx.exe,0"
  WriteRegStr HKCR "REXXScript\shell\open" "" "Run"
  WriteRegStr HKCR "REXXScript\shell\open\command" "" '"$INSTDIR\rexx.exe" "%1" %*'
  WriteRegStr HKCR "REXXScript\shell\edit" "" "Edit"
  WriteRegStr HKCR "REXXScript\shell\edit\command" "" 'notepad.exe "%1"'
  WriteRegStr HKCR "REXXScript\shellex\DropHandler" "" "{60254CA5-953B-11CF-8C96-00AA00B8708C}"
  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
  Return
FunctionEnd

Function Installrxapi
  ; $R1 is start flag
  ; $R0 is install flag
  Pop $R1
  Pop $R0
  Strcmp $R0 0 exitss
    ; do the install of rxapi
    DetailPrint "Installing rxapi as a Windows Service"
    nsExec::ExecToLog "$INSTDIR\rxapi /i /s"
    Pop $R0
    StrCmp $R0 0 dostart
      MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST "Failed to install rxapi as a Windows Service:$\n$R0" /SD IDOK
      Goto exitss
    dostart:
    DetailPrint "rxapi successfully installed as a Windows Service"
    StrCmp $R1 0 exitss
      ; start the service
      Services::SendServiceCommand 'start' 'RXAPI'
      Pop $R0
      StrCmp $R0 'Ok' exitss
        MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST "Failed to start the ooRexx rxapi service:$\n$0" /SD IDOK
  exitss:
FunctionEnd

Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd

;========================================================================
;Uninstaller Section

Section "Uninstall"

;;;; temporarily comment out orxscrpt stuff while it is disabled in the build.
  ; orxscrpt.dll needs to be degistered
;;  !insertmacro UnInstallLib REGDLL NOTSHARED REBOOT_PROTECTED "$INSTDIR\orxscrpt.dll"
  ;
  ; Stop rxapi.exe (again!) the de-registration process starts rxapi.exe GRRRR!!!
  KillProcDLL::KillProc "rxapi.exe"

  ; get rid of file association
  Push ".REX"
  Call un.DeleteFileAssociation

  ; removes the rxapi service - ignore if we get errors
  StrCmp $IsAdminUser "false" NotAdmin
    Services::IsServiceInstalled 'RXAPI'
    Pop $R0
    StrCmp $R0 'No' NotAdmin
    DetailPrint "Uninstalling ooRexx rxapi Service"
    Services::SendServiceCommand 'stop' 'RXAPI'
    Pop $R0
    StrCmp $R0 'Ok' StoppedOK
      ; rxapi.exe used to have a bug where after the Service stopped, rxapi would restart as a
      ; normal process.  This code is a hold over from that, probably not needed anymore.
      DetailPrint "Service Control Manager failed to stop rxapi, forcing termination"
      KillProcDLL::KillProc "rxapi.exe"
    StoppedOK:
    nsExec::ExecToLog "$INSTDIR\rxapi /u /s"
    Pop $R0
    StrCmp $R0 0 doprintok
      DetailPrint "Failed to uninstall rxapi as a service"
      goto NotAdmin
    doprintok:
      DetailPrint "Uninstalled rxapi as a service"
  NotAdmin:

  ; Stop rxapi.exe (again!) just in case
  KillProcDLL::KillProc "rxapi.exe"

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

; !insertmacro MUI_UNFINISHHEADER


 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" logExists
  MessageBox MB_YESNO|MB_ICONSTOP "${UninstLog} not found!$\r$\nWould you like to remove all files under $INSTDIR?$\r$\n(WARNING! This will remove ANY folders or files put there after the last installation.)" /SD IDNO IDYES deltree
   Abort
 deltree:
  ; remove all installed files
  RMDir /r "$INSTDIR"
  ; remove shortcuts directory and everything in it
  RMDir /r "$SMPROGRAMS\${LONGNAME}"
 Goto done

 logExists:
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

 done:
SectionEnd

;========================================================================
;Uninstaller Functions

Function un.onInit
  ;
  ; UnInstall as All Users if an admin
  ;
  Call un.IsUserAdmin
  Pop $IsAdminUser
  StrCmp $IsAdminUser "false" DefaultUser
  SetShellVarContext all
  Goto DefaultUser
  DefaultUser:
FunctionEnd

Function un.DeleteFileAssociation
  Pop $R0
  ReadRegStr $R1 HKCR "$R0" ""
  StrCmp $R1 "REXXScript" 0 NoOwn ; only delete key if we own it
  DeleteRegKey HKCR "$R0"
  DetailPrint "Deleting file association for $R0"
  NoOwn:
  ;
  ; Remove $R0 from PATHEXT for NT-based systems
  ;
  Call un.IsNT
  Pop $1
  StrCmp $1 0 NoAdmin
  StrCmp $IsAdminUser "false" NoAdmin
  Push $R0
  Push $IsAdminUser ; should only be "true" at this point
  Push "PATHEXT"
  Call un.RemoveFromPath
  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
  NoAdmin:
FunctionEnd

Function un.CheckForRxAPI
  ;
  ; Determines if rxapi.exe is running.  On Vista, FindProcDll does not work.
  ; Rather than fix FindProcDll, (easy to do, but I'm not sure of its license,)
  ; we first check if it is running as a service.  If so, we stop it.
  ;
  StrCmp $rxapichk 1 NotRunning
  StrCpy $rxapichk 1
  services::IsServiceRunning 'RXAPI'
  Pop $R0
  StrCmp $R0 'Yes' ServiceIsRunning
  ; Try FindProcDLL
  FindProcDLL::FindProc "rxapi.exe"
  StrCmp $R0 0 NotRunning
  ;
  ; rxapi.exe is running, we need to stop it
  ServiceIsRunning:
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION|MB_TOPMOST "The Open Object Rexx memory manager (RXAPI) is currently active.$\nSelect OK to stop it (possible loss of data) and continue.$\nSelect CANCEL to continue with the uninstall without stopping the service." /SD IDOK IDCANCEL NotRunning
  ;
  ; Stop rxapi.exe.  Send the service stop command first.  If it is not a
  ; service, we don't care, just try kill.
  Services::SendServiceCommand 'stop' 'RXAPI'
  Pop $R0
  StrCmp $R0 'Ok' NotRunning
  KillProcDLL::KillProc "rxapi.exe"

  NotRunning:
FunctionEnd

!include "admin.nsh"
!include "isnt.nsh"
!include "newpath.nsh"
!include "WriteEnv.nsh"

;eof
