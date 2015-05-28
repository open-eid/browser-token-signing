#include "stdafx.h"
#include "esteidpluginie_i.h"
#include "dllmain.h"

CesteidpluginieModule _AtlModule;

//// before MFC support 
// DLL Entry Point
/*
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(hInstance);
    }
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
*/
//// before MFC support end
