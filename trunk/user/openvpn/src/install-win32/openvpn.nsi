; ****************************************************************************
; * Copyright (C) 2002-2005 OpenVPN Solutions LLC                            *
; *  This program is free software; you can redistribute it and/or modify    *
; *  it under the terms of the GNU General Public License version 2          *
; *  as published by the Free Software Foundation.                           *
; ****************************************************************************

; OpenVPN install script for Windows, using NSIS

!include "MUI.nsh"
!include "setpath.nsi"

!define HOME ".."
!define BIN "${HOME}\bin"

!define PRODUCT_NAME "OpenVPN"
!define VERSION "2.0.9" # AUTO_VERSION

!define TAP "tap0801"
!define TAPDRV "${TAP}.sys"

; something like "-DBG2"
!define OUTFILE_LABEL ""

; something like "DEBUG2"
!define TITLE_LABEL ""

; Default service settings
!define SERV_CONFIG_DIR   "$INSTDIR\config"
!define SERV_CONFIG_EXT   "ovpn"
!define SERV_EXE_PATH     "$INSTDIR\bin\openvpn.exe"
!define SERV_LOG_DIR      "$INSTDIR\log"
!define SERV_PRIORITY     "NORMAL_PRIORITY_CLASS"
!define SERV_LOG_APPEND   "0"

;--------------------------------
;Configuration

  ;General

  OutFile "openvpn-${VERSION}${OUTFILE_LABEL}-install.exe"

  SetCompressor bzip2

  ShowInstDetails show
  ShowUninstDetails show

  ;Folder selection page
  InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
  
  ;Remember install folder
  InstallDirRegKey HKCU "Software\${PRODUCT_NAME}" ""

;--------------------------------
;Modern UI Configuration

  Name "${PRODUCT_NAME} ${VERSION} ${TITLE_LABEL}"

  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of OpenVPN, an Open Source VPN package by James Yonan.\r\n\r\nNote that the Windows version of OpenVPN will only run on Win 2000, XP, or higher.\r\n\r\n\r\n"

  !define MUI_COMPONENTSPAGE_TEXT_TOP "Select the components to install/upgrade.  Stop any OpenVPN processes or the OpenVPN service if it is running.  All DLLs are installed locally."

  !define MUI_COMPONENTSPAGE_SMALLDESC
  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\INSTALL-win32.txt"
  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
  !define MUI_ABORTWARNING
  !define MUI_ICON "${HOME}\install-win32\openvpn.ico"
  !define MUI_UNICON "${HOME}\install-win32\openvpn.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${HOME}\install-win32\install-whirl.bmp"
  !define MUI_UNFINISHPAGE_NOAUTOCLOSE

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "${HOME}\install-win32\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES  
  !insertmacro MUI_UNPAGE_FINISH


;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  
;--------------------------------
;Language Strings

  LangString DESC_SecOpenVPNUserSpace ${LANG_ENGLISH} "Install OpenVPN user-space components, including openvpn.exe."

  LangString DESC_SecOpenVPNEasyRSA ${LANG_ENGLISH} "Install OpenVPN RSA scripts for X509 certificate management."

  LangString DESC_SecOpenSSLDLLs ${LANG_ENGLISH} "Install OpenSSL DLLs locally (may be omitted if DLLs are already installed globally)."

  LangString DESC_SecTAP ${LANG_ENGLISH} "Install/Upgrade the TAP-Win32 virtual device driver.  Will not interfere with CIPE."

  LangString DESC_SecService ${LANG_ENGLISH} "Install the OpenVPN service wrapper (openvpnserv.exe)"

  LangString DESC_SecOpenSSLUtilities ${LANG_ENGLISH} "Install the OpenSSL Utilities (used for generating public/private key pairs)."

  LangString DESC_SecAddPath ${LANG_ENGLISH} "Add OpenVPN executable directory to the current user's PATH."

  LangString DESC_SecAddShortcuts ${LANG_ENGLISH} "Add OpenVPN shortcuts to the current user's Start Menu."

  LangString DESC_SecFileAssociation ${LANG_ENGLISH} "Register OpenVPN config file association (*.${SERV_CONFIG_EXT})"

;--------------------------------
;Reserve Files
  
  ;Things that need to be extracted on first (keep these lines before any File command!)
  ;Only useful for BZIP2 compression
  
  ReserveFile "${HOME}\install-win32\install-whirl.bmp"

;--------------------------------
;Macros

!macro WriteRegStringIfUndef ROOT SUBKEY KEY VALUE
Push $R0
ReadRegStr $R0 "${ROOT}" "${SUBKEY}" "${KEY}"
StrCmp $R0 "" +1 +2
WriteRegStr "${ROOT}" "${SUBKEY}" "${KEY}" '${VALUE}'
Pop $R0
!macroend

!macro DelRegStringIfUnchanged ROOT SUBKEY KEY VALUE
Push $R0
ReadRegStr $R0 "${ROOT}" "${SUBKEY}" "${KEY}"
StrCmp $R0 '${VALUE}' +1 +2
DeleteRegValue "${ROOT}" "${SUBKEY}" "${KEY}"
Pop $R0
!macroend

!macro DelRegKeyIfUnchanged ROOT SUBKEY VALUE
Push $R0
ReadRegStr $R0 "${ROOT}" "${SUBKEY}" ""
StrCmp $R0 '${VALUE}' +1 +2
DeleteRegKey "${ROOT}" "${SUBKEY}"
Pop $R0
!macroend

!macro DelRegKeyIfEmpty ROOT SUBKEY
Push $R0
EnumRegValue $R0 "${ROOT}" "${SUBKEY}" 1
StrCmp $R0 "" +1 +2
DeleteRegKey /ifempty "${ROOT}" "${SUBKEY}"
Pop $R0
!macroend

;------------------------------------------
;Set reboot flag based on tapinstall return

Function CheckReboot
  IntCmp $R0 1 "" noreboot noreboot
  IntOp $R0 0 & 0
  SetRebootFlag true
  DetailPrint "REBOOT flag set"
 noreboot:
FunctionEnd

;--------------------------------
;Installer Sections

Function .onInit
  ClearErrors
  UserInfo::GetName
  IfErrors ok
  Pop $R0
  UserInfo::GetAccountType
  Pop $R1
  StrCmp $R1 "Admin" ok
    Messagebox MB_OK "Administrator privileges required to install OpenVPN [$R0/$R1]"
    Abort
  ok:
FunctionEnd

!define SF_SELECTED 1

Section "OpenVPN User-Space Components" SecOpenVPNUserSpace

  SetOverwrite on
  SetOutPath "$INSTDIR\bin"

  File "${HOME}\openvpn.exe"

SectionEnd

Section "OpenVPN RSA Certificate Management Scripts" SecOpenVPNEasyRSA

  SetOverwrite on
  SetOutPath "$INSTDIR\easy-rsa"

  File "${HOME}\install-win32\openssl.cnf.sample"
  File "${HOME}\easy-rsa\Windows\vars.bat.sample"

  File "${HOME}\easy-rsa\Windows\init-config.bat"

  File "${HOME}\easy-rsa\Windows\README.txt"
  File "${HOME}\easy-rsa\Windows\build-ca.bat"
  File "${HOME}\easy-rsa\Windows\build-dh.bat"
  File "${HOME}\easy-rsa\Windows\build-key-server.bat"
  File "${HOME}\easy-rsa\Windows\build-key.bat"
  File "${HOME}\easy-rsa\Windows\build-key-pkcs12.bat"
  File "${HOME}\easy-rsa\Windows\clean-all.bat"
  File "${HOME}\easy-rsa\Windows\index.txt.start"
  File "${HOME}\easy-rsa\Windows\revoke-full.bat"
  File "${HOME}\easy-rsa\Windows\serial.start"

SectionEnd

Section "OpenVPN Service" SecService

  SetOverwrite on

  SetOutPath "$INSTDIR\bin"
  File "${HOME}\service-win32\openvpnserv.exe"

  SetOutPath "$INSTDIR\config"

  FileOpen $R0 "$INSTDIR\config\README.txt" w
  FileWrite $R0 "This directory should contain OpenVPN configuration files$\r$\n"
  FileWrite $R0 "each having an extension of .${SERV_CONFIG_EXT}$\r$\n"
  FileWrite $R0 "$\r$\n"
  FileWrite $R0 "When OpenVPN is started as a service, a separate OpenVPN$\r$\n"
  FileWrite $R0 "process will be instantiated for each configuration file.$\r$\n"
  FileClose $R0

  SetOutPath "$INSTDIR\sample-config"
  File "${HOME}\install-win32\sample.${SERV_CONFIG_EXT}"
  File "${HOME}\install-win32\client.${SERV_CONFIG_EXT}"
  File "${HOME}\install-win32\server.${SERV_CONFIG_EXT}"

  CreateDirectory "$INSTDIR\log"
  FileOpen $R0 "$INSTDIR\log\README.txt" w
  FileWrite $R0 "This directory will contain the log files for OpenVPN$\r$\n"
  FileWrite $R0 "sessions which are being run as a service.$\r$\n"
  FileClose $R0

SectionEnd

Section "OpenVPN File Associations" SecFileAssociation
SectionEnd

Section "OpenSSL DLLs" SecOpenSSLDLLs

  SetOverwrite on
  SetOutPath "$INSTDIR\bin"
  File "${BIN}\libeay32.dll"
  File "${BIN}\libssl32.dll"

SectionEnd

Section "OpenSSL Utilities" SecOpenSSLUtilities

  SetOverwrite on
  SetOutPath "$INSTDIR\bin"
  File "${BIN}\openssl.exe"

SectionEnd

Section "TAP-Win32 Virtual Ethernet Adapter" SecTAP

  SetOverwrite on
  SetOutPath "$INSTDIR\bin"
  File "${BIN}\ti3790\tapinstall.exe"

  FileOpen $R0 "$INSTDIR\bin\addtap.bat" w
  FileWrite $R0 "rem Add a new TAP-Win32 virtual ethernet adapter$\r$\n"
  FileWrite $R0 '"$INSTDIR\bin\tapinstall.exe" install "$INSTDIR\driver\OemWin2k.inf" ${TAP}$\r$\n'
  FileWrite $R0 "pause$\r$\n"
  FileClose $R0

  FileOpen $R0 "$INSTDIR\bin\deltapall.bat" w
  FileWrite $R0 "echo WARNING: this script will delete ALL TAP-Win32 virtual adapters (use the device manager to delete adapters one at a time)$\r$\n"
  FileWrite $R0 "pause$\r$\n"
  FileWrite $R0 '"$INSTDIR\bin\tapinstall.exe" remove ${TAP}$\r$\n'
  FileWrite $R0 "pause$\r$\n"
  FileClose $R0

  SetOutPath "$INSTDIR\driver"
  File "${HOME}\tap-win32\i386\OemWin2k.inf"
  File "${HOME}\tap-win32\i386\${TAPDRV}"

SectionEnd

Section "Add OpenVPN to PATH" SecAddPath

  ; remove previously set path (if any)
  Push "$INSTDIR\bin"
  Call RemoveFromPath

  ; append our bin directory to end of current user path
  Push "$INSTDIR\bin"
  Call AddToPath

SectionEnd

Section "Add Shortcuts to Start Menu" SecAddShortcuts

  SetOverwrite on
  CreateDirectory "$SMPROGRAMS\OpenVPN"
  WriteINIStr "$SMPROGRAMS\OpenVPN\OpenVPN Windows Notes.url" "InternetShortcut" "URL" "http://openvpn.net/INSTALL-win32.html"
  WriteINIStr "$SMPROGRAMS\OpenVPN\OpenVPN Manual Page.url" "InternetShortcut" "URL" "http://openvpn.net/man.html"
  WriteINIStr "$SMPROGRAMS\OpenVPN\OpenVPN HOWTO.url" "InternetShortcut" "URL" "http://openvpn.net/howto.html"
  WriteINIStr "$SMPROGRAMS\OpenVPN\OpenVPN Web Site.url" "InternetShortcut" "URL" "http://openvpn.net/"
  CreateShortCut "$SMPROGRAMS\OpenVPN\Uninstall OpenVPN.lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------
;Post-install section

Section -post

  ; delete old devcon.exe
  Delete "$INSTDIR\bin\devcon.exe"

  ;
  ; install/upgrade TAP-Win32 driver if selected, using tapinstall.exe
  ;
  SectionGetFlags ${SecTAP} $R0
  IntOp $R0 $R0 & ${SF_SELECTED}
  IntCmp $R0 ${SF_SELECTED} "" notap notap
    ; TAP install/update was selected.
    ; Should we install or update?
    ; If tapinstall error occurred, $5 will
    ; be nonzero.
    IntOp $5 0 & 0
    nsExec::ExecToStack '"$INSTDIR\bin\tapinstall.exe" hwids ${TAP}'
    Pop $R0 # return value/error/timeout
    IntOp $5 $5 | $R0
    DetailPrint "tapinstall hwids returned: $R0"

    ; If tapinstall output string contains "${TAP}" we assume
    ; that TAP device has been previously installed,
    ; therefore we will update, not install.
    Push "${TAP}"
    Call StrStr
    Pop $R0

    IntCmp $5 0 "" tapinstall_check_error tapinstall_check_error
    IntCmp $R0 -1 tapinstall

 ;tapupdate:
    DetailPrint "TAP-Win32 UPDATE"
    nsExec::ExecToLog '"$INSTDIR\bin\tapinstall.exe" update "$INSTDIR\driver\OemWin2k.inf" ${TAP}'
    Pop $R0 # return value/error/timeout
    Call CheckReboot
    IntOp $5 $5 | $R0
    DetailPrint "tapinstall update returned: $R0"
    Goto tapinstall_check_error

 tapinstall:
    DetailPrint "TAP-Win32 REMOVE OLD TAP"
    nsExec::ExecToLog '"$INSTDIR\bin\tapinstall.exe" remove TAP'
    Pop $R0 # return value/error/timeout
    DetailPrint "tapinstall remove TAP returned: $R0"
    nsExec::ExecToLog '"$INSTDIR\bin\tapinstall.exe" remove TAPDEV'
    Pop $R0 # return value/error/timeout
    DetailPrint "tapinstall remove TAPDEV returned: $R0"

    DetailPrint "TAP-Win32 INSTALL (${TAP})"
    nsExec::ExecToLog '"$INSTDIR\bin\tapinstall.exe" install "$INSTDIR\driver\OemWin2k.inf" ${TAP}'
    Pop $R0 # return value/error/timeout
    Call CheckReboot
    IntOp $5 $5 | $R0
    DetailPrint "tapinstall install returned: $R0"

 tapinstall_check_error:
    DetailPrint "tapinstall cumulative status: $5"
    IntCmp $5 0 notap
    MessageBox MB_OK "An error occurred installing the TAP-Win32 device driver."

 notap:

  ; Store install folder in registry
  WriteRegStr HKLM SOFTWARE\OpenVPN "" $INSTDIR

  ; install as a service if requested
  SectionGetFlags ${SecService} $R0
  IntOp $R0 $R0 & ${SF_SELECTED}
  IntCmp $R0 ${SF_SELECTED} "" noserv noserv

    ; set registry parameters for openvpnserv	
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "config_dir"  "${SERV_CONFIG_DIR}"
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "config_ext"  "${SERV_CONFIG_EXT}"
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "exe_path"    "${SERV_EXE_PATH}"
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "log_dir"     "${SERV_LOG_DIR}"
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "priority"    "${SERV_PRIORITY}"
    !insertmacro WriteRegStringIfUndef HKLM "SOFTWARE\OpenVPN" "log_append"  "${SERV_LOG_APPEND}"

    ; install openvpnserv as a service
    DetailPrint "Previous Service REMOVE (if exists)"
    nsExec::ExecToLog '"$INSTDIR\bin\openvpnserv.exe" -remove'
    Pop $R0 # return value/error/timeout
    DetailPrint "Service INSTALL"
    nsExec::ExecToLog '"$INSTDIR\bin\openvpnserv.exe" -install'
    Pop $R0 # return value/error/timeout

 noserv:
  ; Store README, license, icon
  SetOverwrite on
  SetOutPath $INSTDIR
  File "${HOME}\install-win32\INSTALL-win32.txt"
  File "${HOME}\install-win32\license.txt"
  File "${HOME}\install-win32\openvpn.ico"

  ; Create file association if requested
  SectionGetFlags ${SecFileAssociation} $R0
  IntOp $R0 $R0 & ${SF_SELECTED}
  IntCmp $R0 ${SF_SELECTED} "" noass noass
    !insertmacro WriteRegStringIfUndef HKCR ".${SERV_CONFIG_EXT}" "" "OpenVPNFile"
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile" "" "OpenVPN Config File"
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile\shell" "" "open"
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile\DefaultIcon" "" "$INSTDIR\openvpn.ico,0"
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile\shell\open\command" "" 'notepad.exe "%1"'
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile\shell\run" "" "Start OpenVPN on this config file"
    !insertmacro WriteRegStringIfUndef HKCR "OpenVPNFile\shell\run\command" "" '"$INSTDIR\bin\openvpn.exe" --pause-exit --config "%1"'

    ; Create start menu shortcuts to addtap.bat and deltapall.bat
 noass:
    IfFileExists "$INSTDIR\bin\addtap.bat" "" trydeltap
      CreateShortCut "$SMPROGRAMS\OpenVPN\Add a new TAP-Win32 virtual ethernet adapter.lnk" "$INSTDIR\bin\addtap.bat" ""

 trydeltap:
    IfFileExists "$INSTDIR\bin\deltapall.bat" "" config_shortcut
      CreateShortCut "$SMPROGRAMS\OpenVPN\Delete ALL TAP-Win32 virtual ethernet adapters.lnk" "$INSTDIR\bin\deltapall.bat" ""

    ; Create start menu shortcuts for config and log directories
 config_shortcut:
    IfFileExists "$INSTDIR\config" "" log_shortcut
      CreateShortCut "$SMPROGRAMS\OpenVPN\OpenVPN configuration file directory.lnk" "$INSTDIR\config" ""

 log_shortcut:
    IfFileExists "$INSTDIR\log" "" samp_shortcut
      CreateShortCut "$SMPROGRAMS\OpenVPN\OpenVPN log file directory.lnk" "$INSTDIR\log" ""

 samp_shortcut:
    IfFileExists "$INSTDIR\sample-config" "" genkey_shortcut
      CreateShortCut "$SMPROGRAMS\OpenVPN\OpenVPN Sample Configuration Files.lnk" "$INSTDIR\sample-config" ""

 genkey_shortcut:
    IfFileExists "$INSTDIR\bin\openvpn.exe" "" noshortcuts
      IfFileExists "$INSTDIR\config" "" noshortcuts
        CreateShortCut "$SMPROGRAMS\OpenVPN\Generate a static OpenVPN key.lnk" "$INSTDIR\bin\openvpn.exe" '--pause-exit --verb 3 --genkey --secret "$INSTDIR\config\key.txt"' "$INSTDIR\openvpn.ico" 0

 noshortcuts:
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Show up in Add/Remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME} ${VERSION}"
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayIcon" "$INSTDIR\openvpn.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayVersion" "${VERSION}"

  ; Advise a reboot
  ;Messagebox MB_OK "IMPORTANT: Rebooting the system is advised in order to finalize TAP-Win32 driver installation/upgrade (this is an informational message only, pressing OK will not reboot)."

SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenVPNUserSpace} $(DESC_SecOpenVPNUserSpace)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenVPNEasyRSA} $(DESC_SecOpenVPNEasyRSA)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTAP} $(DESC_SecTAP)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenSSLUtilities} $(DESC_SecOpenSSLUtilities)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenSSLDLLs} $(DESC_SecOpenSSLDLLs)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAddPath} $(DESC_SecAddPath)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAddShortcuts} $(DESC_SecAddShortcuts)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecService} $(DESC_SecService)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssociation} $(DESC_SecFileAssociation)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Function un.onInit
  ClearErrors
  UserInfo::GetName
  IfErrors ok
  Pop $R0
  UserInfo::GetAccountType
  Pop $R1
  StrCmp $R1 "Admin" ok
    Messagebox MB_OK "Administrator privileges required to uninstall OpenVPN [$R0/$R1]"
    Abort
  ok:
FunctionEnd

Section "Uninstall"

  DetailPrint "Service REMOVE"
  nsExec::ExecToLog '"$INSTDIR\bin\openvpnserv.exe" -remove'
  Pop $R0 # return value/error/timeout

  Sleep 2000

  DetailPrint "TAP-Win32 REMOVE"
  nsExec::ExecToLog '"$INSTDIR\bin\tapinstall.exe" remove ${TAP}'
  Pop $R0 # return value/error/timeout
  DetailPrint "tapinstall remove returned: $R0"

  Push "$INSTDIR\bin"
  Call un.RemoveFromPath

  RMDir /r $SMPROGRAMS\OpenVPN

  Delete "$INSTDIR\bin\openvpn.exe"
  Delete "$INSTDIR\bin\openvpnserv.exe"
  Delete "$INSTDIR\bin\libeay32.dll"
  Delete "$INSTDIR\bin\libssl32.dll"
  Delete "$INSTDIR\bin\tapinstall.exe"
  Delete "$INSTDIR\bin\addtap.bat"
  Delete "$INSTDIR\bin\deltapall.bat"

  Delete "$INSTDIR\config\README.txt"
  Delete "$INSTDIR\config\sample.${SERV_CONFIG_EXT}.txt"

  Delete "$INSTDIR\log\README.txt"

  Delete "$INSTDIR\driver\OemWin2k.inf"
  Delete "$INSTDIR\driver\${TAPDRV}"

  Delete "$INSTDIR\bin\openssl.exe"

  Delete "$INSTDIR\INSTALL-win32.txt"
  Delete "$INSTDIR\openvpn.ico"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\Uninstall.exe"

  Delete "$INSTDIR\easy-rsa\openssl.cnf.sample"
  Delete "$INSTDIR\easy-rsa\vars.bat.sample"
  Delete "$INSTDIR\easy-rsa\init-config.bat"
  Delete "$INSTDIR\easy-rsa\README.txt"
  Delete "$INSTDIR\easy-rsa\build-ca.bat"
  Delete "$INSTDIR\easy-rsa\build-dh.bat"
  Delete "$INSTDIR\easy-rsa\build-key-server.bat"
  Delete "$INSTDIR\easy-rsa\build-key.bat"
  Delete "$INSTDIR\easy-rsa\build-key-pkcs12.bat"
  Delete "$INSTDIR\easy-rsa\clean-all.bat"
  Delete "$INSTDIR\easy-rsa\index.txt.start"
  Delete "$INSTDIR\easy-rsa\revoke-key.bat"
  Delete "$INSTDIR\easy-rsa\revoke-full.bat"
  Delete "$INSTDIR\easy-rsa\serial.start"

  Delete "$INSTDIR\sample-config\*.ovpn"

  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR\driver"
  RMDir "$INSTDIR\easy-rsa"
  RMDir "$INSTDIR\sample-config"
  RMDir "$INSTDIR"

  !insertmacro DelRegKeyIfUnchanged HKCR ".${SERV_CONFIG_EXT}" "OpenVPNFile"
  DeleteRegKey HKCR "OpenVPNFile"
  DeleteRegKey HKLM SOFTWARE\OpenVPN
  DeleteRegKey HKCU "Software\${PRODUCT_NAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVPN"

  ;Messagebox MB_OK "IMPORTANT: If you intend on reinstalling OpenVPN after this uninstall, and you are running Win2K, you are strongly urged to reboot before reinstalling (this is an informational message only, pressing OK will not reboot)."

SectionEnd
