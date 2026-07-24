Unicode true
RequestExecutionLevel user
SetCompressor /SOLID lzma

!define PRODUCT_NAME "Mumble Share"
!define PRODUCT_VERSION "0.1.2"
!define PRODUCT_PUBLISHER "Hava Sepehr"
!define PRODUCT_URL "https://send.havasepehr.ir/mumble/"

Name "${PRODUCT_NAME}"
OutFile "dist\MumbleShare-${PRODUCT_VERSION}-win-x64.exe"
InstallDir "$LOCALAPPDATA\MumbleShare"
InstallDirRegKey HKCU "Software\MumbleShare" "InstallDir"
VIProductVersion "0.1.2.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2026 Hava Sepehr"
VIAddVersionKey "FileDescription" "Mumble Share installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Mumble Share" SEC_MAIN
  SetShellVarContext current
  SetOutPath "$INSTDIR"

  MessageBox MB_OK|MB_ICONINFORMATION \
    "Close Mumble before installing or updating Mumble Share.$\r$\n$\r$\nAfter installation, check both Enable and KeyEvents for Mumble Share in Mumble's plugin settings."

  File "/oname=mumble-share.dll" "..\build-windows\bin\mumble-share.dll"
  File "/oname=mumble-share-helper.exe" "..\build-windows\bin\mumble-share-helper.exe"
  File "/oname=LICENSE.txt" "..\LICENSE"
  File "/oname=THIRD_PARTY_NOTICES.txt" "..\THIRD_PARTY_NOTICES.md"

  CreateDirectory "$APPDATA\Mumble\Plugins"
  ClearErrors
  CopyFiles /SILENT "$INSTDIR\mumble-share.dll" "$APPDATA\Mumble\Plugins\mumble-share.dll"
  CopyFiles /SILENT "$INSTDIR\mumble-share-helper.exe" "$APPDATA\Mumble\Plugins\mumble-share-helper.exe"
  IfErrors install_failed

  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\MumbleShare" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare" \
    "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare" \
    "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare" \
    "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare" \
    "URLInfoAbout" "${PRODUCT_URL}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare" \
    "UninstallString" '"$INSTDIR\Uninstall.exe"'

  ExecShell "open" "${PRODUCT_URL}"
  Goto install_done

install_failed:
  MessageBox MB_OK|MB_ICONSTOP \
    "Mumble Share could not update Mumble's plugin directory.$\r$\n$\r$\nClose Mumble completely and run this installer again."
  Abort

install_done:
SectionEnd

Section "Uninstall"
  SetShellVarContext current
  MessageBox MB_OK|MB_ICONINFORMATION "Close Mumble before uninstalling Mumble Share."

  Delete "$APPDATA\Mumble\Plugins\mumble-share.dll"
  Delete "$APPDATA\Mumble\Plugins\mumble-share-helper.exe"
  Delete "$INSTDIR\mumble-share.dll"
  Delete "$INSTDIR\mumble-share-helper.exe"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\THIRD_PARTY_NOTICES.txt"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MumbleShare"
  DeleteRegKey HKCU "Software\MumbleShare"
SectionEnd
