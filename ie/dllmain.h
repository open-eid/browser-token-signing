// dllmain.h : Declaration of module class.

class CesteidpluginieModule : public ATL::CAtlDllModuleT< CesteidpluginieModule >
{
public :
	DECLARE_LIBID(LIBID_esteidpluginieLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ESTEIDPLUGINIE, "{545B4ED6-A2FE-49F1-AF8E-FA18B6261E48}")
};

extern class CesteidpluginieModule _AtlModule;
