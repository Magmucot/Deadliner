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

Page directory
Page instfiles

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

Section "install"
    SetOutPath $INSTDIR

    File "deadliner.exe"
    ; Deploy all Qt DLLs placed alongside the exe by windeployqt
    File /nonfatal "*.dll"
    ; Qt plugin subdirectories (platforms, styles, imageformats, etc.)
    File /nonfatal /r "platforms"
    File /nonfatal /r "styles"
    File /nonfatal /r "imageformats"
    File /nonfatal /r "sqldrivers"
    File "LICENSE"
    File "README.md"

    ; Start Menu shortcut
    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"
    CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\deadliner.exe"

    ; Uninstaller
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
    Delete "$DESKTOP\${APPNAME}.lnk"
    RMDir  "$SMPROGRAMS\${APPNAME}"

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
