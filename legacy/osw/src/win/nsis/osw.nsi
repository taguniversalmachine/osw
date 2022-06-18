; osw.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of makensisw.exe
; there. 
;

;Set $5 to the destination directory of "make install"

!define OSW_DIR "C:\osw"

;

!include "MUI.nsh"
!include "UpgradeDLL.nsh"

;General
Name "Open Sound World 1.2"
OutFile "osw1.2.2.win.exe"

;Folder-selection page
InstallDir "$PROGRAMFILES\Open Sound World"


!define MUI_BRANDINGTEXT "Open Sound World"

!define MUI_ABORTWARNING

!define UPGRADEDLL_NOREGISTER

; Variables

Var STARTMENU_FOLDER
Var MUI_TEMP

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\..\License"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU "Application" $STARTMENU_FOLDER
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

	
;Language
!insertmacro MUI_LANGUAGE "English"


;--------------------------------
;Modern UI System


; The stuff to install
Section "ThisNameIsIgnoredSoWhyBother?"



  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put files there

  FILE itcl82.exe
  File /r ${OSW_DIR}\*

  

  IfFileExists $PROGRAMFILES\Tcl\bin\tcl82.dll posttcl dotcl
  IfFileExists $PROGRAMFILES\Tcl\bin\itcl31.dll posttcl dotcl
  IfFileExists $PROGRAMFILES\Tcl\lib\iwidgets* posttcl 
dotcl:
  MessageBox MB_YESNO "Tcl/Tk 8.2 and Itcl 3.1 are necessary in order to run OSW 1.1.  Would you like me to install them for you now?" IDNO posttcl
  MessageBox MB_OK 'I will now run the Tcl/Tk/Itcl installer.  When "Select Components" appears, make sure to check "Incr Tcl..."'
  ExecWait "$INSTDIR\itcl82.exe"

posttcl:

  Delete "$INSTDIR\itcl82.exe"

  !insertmacro UpgradeDLL "msvcr70.dll" "$SYSDIR\msvcr70.dll" "$SYSDIR"
  !insertmacro UpgradeDLL "msvcp70.dll" "$SYSDIR\msvcp70.dll" "$SYSDIR"

  WriteRegStr HKLM Software\OSW Home "$INSTDIR"
  WriteRegStr HKCR .osw "" "OSW Patch"
  WriteRegStr HKCR "OSW Patch" "" "OSW Patch"
  WriteRegStr HKCR "OSW Patch\shell\open\command" "" "$INSTDIR\osw.exe %1"
  WriteRegStr HKCR .osx "" "OSW External"
  WriteRegStr HKCR "OSW External" "" "OSW External"
  WriteRegStr HKCR "OSW External\shell\open\command" "" "$INSTDIR\osw.exe --install %"
  WriteRegStr HKCR .osd "" "OSW Externalizer Description"
  WriteRegStr HKCR "OSW Externalizer Description" "" "OSW External"
  WriteRegStr HKCR "OSW Externalizer Description\shell\open\command" "" "$INSTDIR\osw.exe  %1"
 

 !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Open Sound World.lnk" "$INSTDIR\osw.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Open Sound World (command-line).lnk" "$INSTDIR\oswbin.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\OSW Users Guide.lnk" "$INSTDIR\html\osw.html"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\OSW Help Patches.lnk" "$INSTDIR\help_patches"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Tutorial Patches.lnk" "$INSTDIR\tutorials"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Demo Patches.lnk" "$INSTDIR\demos"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
    CreateShortCut "$DESKTOP\Open Sound World.lnk" "$INSTDIR\osw.exe"

    ;Write shortcut location to the registry (for Uninstaller)
    WriteRegStr HKCU "Software\Open Sound World" "Start Menu Folder" "$STARTMENU_FOLDER"
    
  !insertmacro MUI_STARTMENU_WRITE_END

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd ; end the section

; eof

Section "Uninstall"

  ;ADD YOUR OWN STUFF HERE!

  DeleteRegKey  HKLM Software\OSW
  DeleteRegKey  HKCU Software\OSW
  DeleteRegKey  HKCR .osw
  DeleteRegKey  HKCR .osx
  DeleteRegKey  HKCR .osd
  DeleteRegKey  HKCR "OSW Patch"
  DeleteRegKey  HKCR "OSW External"
  DeleteRegKey  HKCR "OSW Externalizer Description"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  Delete "$SMPROGRAMS\$MUI_TEMP\*.*"
  RMDir "$SMPROGRAMS\$MUI_TEMP"

  Delete "$DESKTOP\Open Sound World.lnk"

  RMDir /r "$INSTDIR"


SectionEnd
