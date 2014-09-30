@echo off
PATH=%PATH%;"C:\Program Files\Estonian ID Card"
pkcs15-tool --reader %1 --unblock-pin --auth-id 2 --puk 17258403 --new-pin 00015
if %ERRORLEVEL% EQU 0 pkcs15-tool --reader %1 --change-pin --auth-id 2 --pin 00015 --new-pin 01497