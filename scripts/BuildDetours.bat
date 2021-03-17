@echo off
@call "%programfiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -host_arch=amd64
@call "%programfiles(x86)%\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -host_arch=amd64
nmake