; Deadliner NSIS Installer Script
!define APPNAME "Deadliner"
!define COMPANYNAME "Deadliner"
!define DESCRIPTION "Desktop break and reminder utility"
!define VERSIONMAJOR 0
!define VERSIONMINOR 1
!define VERSIONBUILD 0
!define HELPURL    "https://github.com/Magmucot/Deadliner"
!define UPDATEURL  "https://github.com/Magmucot/Deadliner/releases"
!define ABOUTURL   "https://github.com/Magmucot/Deadliner"
!define INSTALLSIZE 50000

RequestExecutionLevel admin
InstallDir "$PROGRAMFILES64\${APPNAME}"
Name "${APPNAME}"
; NOTE: NSIS requires an .ico file for the installer icon.
; Uncomment and set the correct path once an .ico file is added to the project:
; Icon "resources\icons\deadliner.ico"
OutFile "deadliner-installer.exe"

!include LogicLib.nsh
!include MUI2.nsh

Var StartMenuCB

!define MUI_FINISHPAGE_RUN "$INSTDIR\deadliner.exe"
!define MUI_FINISHPAGE_RUN_TEXT "$(RunApp)"
!define MUI_FINISHPAGE_RUN_NOTCHECKED

!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_TEXT "$(CreateDesktopShortcut)"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDesktopShortcut
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

!define MUI_FINISHPAGE_CUSTOMFUNCTION_SHOW FinishShow
!define MUI_FINISHPAGE_CUSTOMFUNCTION_LEAVE FinishLeave

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

LangString RunApp                ${LANG_ENGLISH} "Run ${APPNAME} after installation"
LangString CreateDesktopShortcut ${LANG_ENGLISH} "Create desktop shortcut"
LangString CreateStartMenuShortcut ${LANG_ENGLISH} "Create Start Menu shortcut"
LangString RunApp                ${LANG_RUSSIAN}  "Запустить ${APPNAME} после установки"
LangString CreateDesktopShortcut ${LANG_RUSSIAN}  "Создать ярлык на рабочем столе"
LangString CreateStartMenuShortcut ${LANG_RUSSIAN}  "Создать ярлык в меню Пуск"

!insertmacro MUI_LANGUAGE English
!insertmacro MUI_LANGUAGE Russian

!macro VerifyUserIsAdmin
    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "admin"
        MessageBox MB_ICONSTOP "Administrator rights required!"
        SetErrorLevel 740
        Quit
    ${EndIf}
!macroend

Function .onInit
    SetShellVarContext all
    !insertmacro VerifyUserIsAdmin
FunctionEnd

Function CreateDesktopShortcut
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"
FunctionEnd

Function FinishShow
    GetDlgItem $1 $MUI_HWND 1200
    ${If} $1 <> 0
        System::Call 'user32::GetWindowRect(i $1, @ .r2, @ .r3, @ .r4, @ .r5)'
        System::Call 'user32::ScreenToClient(i $MUI_HWND, @ .r2, @ .r3)'
        IntOp $3 $3 + 20
        IntOp $5 $5 - $2
        System::Call 'user32::CreateWindowEx(i0, t"BUTTON", t"$(CreateStartMenuShortcut)", \
            i0x50010003, i$r2, i$r3, i$r5, i16, i$MUI_HWND, i0, i0, i0) i.StartMenuCB'
        SendMessage $1 ${WM_GETFONT} 0 0 $6
        SendMessage $StartMenuCB ${WM_SETFONT} $6 0
        SendMessage $StartMenuCB ${BM_SETCHECK} ${BST_CHECKED} 0
    ${EndIf}
FunctionEnd

Function FinishLeave
    SendMessage $StartMenuCB ${BM_GETCHECK} 0 0 $0
    ${If} $0 = ${BST_CHECKED}
        CreateDirectory "$SMPROGRAMS\${APPNAME}"
        CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"
    ${EndIf}
FunctionEnd

Section "install"
    SetOutPath $INSTDIR

    File "deadliner.exe"
    File /nonfatal "*.dll"
    File /nonfatal /r "platforms"
    File /nonfatal /r "styles"
    File /nonfatal /r "imageformats"
    File /nonfatal /r "sqldrivers"
    File "LICENSE"
    File "README.md"

    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Add/Remove Programs entry
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "DisplayName"          "${APPNAME} - ${DESCRIPTION}"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "UninstallString"      "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "InstallLocation"      "$\"$INSTDIR$\""
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "DisplayIcon"          "$\"$INSTDIR\deadliner.exe$\""
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "Publisher"            "${COMPANYNAME}"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "HelpLink"             "${HELPURL}"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "URLUpdateInfo"        "${UPDATEURL}"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "URLInfoAbout"         "${ABOUTURL}"
    WriteRegStr  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "DisplayVersion"       "${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "VersionMajor"  ${VERSIONMAJOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "VersionMinor"  ${VERSIONMINOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "NoModify"      1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "NoRepair"      1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" \
                 "EstimatedSize" ${INSTALLSIZE}
SectionEnd

Section "uninstall"
    Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
    RMDir  "$SMPROGRAMS\${APPNAME}"
    Delete "$DESKTOP\${APPNAME}.lnk"

    Delete "$INSTDIR\deadliner.exe"
    Delete "$INSTDIR\*.dll"
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\sqldrivers"
    Delete "$INSTDIR\LICENSE"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\uninstall.exe"
    RMDir  "$INSTDIR"

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd
