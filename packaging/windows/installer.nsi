; Deadliner NSIS Installer Script
Unicode true
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
!include nsDialogs.nsh

Var StartMenuCheckbox
Var DesktopCheckbox

!define MUI_FINISHPAGE_RUN "$INSTDIR\deadliner.exe"
!define MUI_FINISHPAGE_RUN_TEXT "$(RunApp)"
!define MUI_FINISHPAGE_RUN_NOTCHECKED

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
Page custom ShortcutPage ShortcutPageLeave
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE English
!insertmacro MUI_LANGUAGE Russian

LangString RunApp                ${LANG_ENGLISH} "Run ${APPNAME} after installation"
LangString CreateDesktopShortcut ${LANG_ENGLISH} "Create desktop shortcut"
LangString CreateStartMenuShortcut ${LANG_ENGLISH} "Create Start Menu shortcut"
LangString ShortcutsTitle        ${LANG_ENGLISH} "Shortcuts"
LangString ShortcutsSubtitle     ${LANG_ENGLISH} "Create shortcuts for ${APPNAME}"
LangString RunApp                ${LANG_RUSSIAN}  "Run ${APPNAME} after installation"
LangString CreateDesktopShortcut ${LANG_RUSSIAN}  "Create desktop shortcut"
LangString CreateStartMenuShortcut ${LANG_RUSSIAN}  "Create Start Menu shortcut"
LangString ShortcutsTitle        ${LANG_RUSSIAN}  "Shortcuts"
LangString ShortcutsSubtitle     ${LANG_RUSSIAN}  "Create shortcuts for ${APPNAME}"

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

Function ShortcutPage
    !insertmacro MUI_HEADER_TEXT "$(ShortcutsTitle)" "$(ShortcutsSubtitle)"
    nsDialogs::Create 1018
    Pop $0

    ${NSD_CreateCheckbox} 0 0 100% 12u "$(CreateStartMenuShortcut)"
    Pop $StartMenuCheckbox
    ${NSD_SetState} $StartMenuCheckbox ${BST_CHECKED}

    ${NSD_CreateCheckbox} 0 20u 100% 12u "$(CreateDesktopShortcut)"
    Pop $DesktopCheckbox

    nsDialogs::Show
FunctionEnd

Function ShortcutPageLeave
    ${NSD_GetState} $StartMenuCheckbox $0
    ${If} $0 = ${BST_CHECKED}
        CreateDirectory "$SMPROGRAMS\${APPNAME}"
        CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"
    ${EndIf}

    ${NSD_GetState} $DesktopCheckbox $0
    ${If} $0 = ${BST_CHECKED}
        CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"
    ${EndIf}
FunctionEnd

Section "install"
    ; Ensure any running instance is closed before overwriting files
    ExecWait '"$SYSDIR\taskkill.exe" /F /IM deadliner.exe /T'
    Sleep 1000

    SetOverwrite on
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
