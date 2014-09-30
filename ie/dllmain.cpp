// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "esteidpluginie_i.h"
#include "dllmain.h"

CesteidpluginieModule _AtlModule;

//// before MFC support 
// DLL Entry Point
/*extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
	DisableThreadLibraryCalls(hInstance);
    }
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
*/
//// before MFC support end

class CEstEIDBHOApp : public CWinApp
{
public:

// Overrides
       virtual BOOL InitInstance();
       virtual int ExitInstance();

       DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CEstEIDBHOApp, CWinApp)
END_MESSAGE_MAP()

CEstEIDBHOApp theApp;

BOOL CEstEIDBHOApp::InitInstance()
{
       return CWinApp::InitInstance();
}

int CEstEIDBHOApp::ExitInstance()
{
       return CWinApp::ExitInstance();
}
