REM ** To use this you need to install WiX Toolset from http://wixtoolset.org/ 
REM ** and add the install directory as an environment variable as WIX.
REM ** It creates both x86 and x64 .msi files.


call "%WIX%\bin\candle.exe" esteid-plugin-ie.wxs -dVERSION=3.11.1 -dPlatform=x86
call "%WIX%\bin\light.exe" -out ie-token-signing_3.11.1_x86.msi esteid-plugin-ie.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=dlgbmp.bmp

call "%WIX%\bin\candle.exe" esteid-plugin-ie.wxs -dVERSION=3.11.1 -dPlatform=x64
call "%WIX%\bin\light.exe" -out ie-token-signing_3.11.1_x64.msi esteid-plugin-ie.wixobj -v -ext WixUIExtension -dWixUILicenseRtf=LICENSE.LGPL.rtf -dWixUIDialogBmp=dlgbmp.bmp