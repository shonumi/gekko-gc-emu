// plugins.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "plugins.h"
#include "memory.h"
#include "hw/hw_pe.h"

#define				USE_DOLPHIN_PLUGIN
PluginData          plug;

////////////////////////////////////////////////////////////////////////////////

GXOPEN              GXOpen             = NULL;
GXCLOSE             GXClose            = NULL;
GXWRITEFIFO         GXWriteFifo        = NULL;
GXSETTOKENS         GXSetTokens        = NULL;
GXCONFIGURE         GXConfigure        = NULL;
GXABOUT             GXAbout            = NULL;
GXSAVELOAD          GXSaveLoad         = NULL;

PADOPEN             PADOpen            = NULL;
PADCLOSE            PADClose           = NULL;
PADREADBUTTONS      PADReadButtons     = NULL;
PADSETRUMBLE        PADSetRumble       = NULL;
PADCONFIGURE        PADConfigure       = NULL;
PADABOUT            PADAbout           = NULL;
PADSAVELOAD         PADSaveLoad        = NULL;

REGISTERPLUGIN      RegisterPlugin     = NULL;

//////////////////////////////////////////////////////////////////////

static int  ps_opened = FALSE;

static HINSTANCE hGXInst = NULL,
                 hPADInst = NULL;

//////////////////////////////////////////////////////////////////////

#define LoadAPI(lib, cast, name)								\
{																\
    ##name## = (cast)GetProcAddress(h##lib##Inst, #name);		\
    ASSERT_T(##name## == NULL, #lib"::"#name" is missing!");	\
}

//////////////////////////////////////////////////////////////////////

static void GXDolphinPluginInit(char *name, int warn)
{
    unsigned long type = 0;

    hGXInst = LoadLibrary(name);
    if(hGXInst == NULL)
    {
        MessageBox(
            NULL, 
            "Video plugin is not assigned (3D graphics unavailable).\n"
            "Please, select graphics plugin in options.", 
            "Gekko Plugin System", 
            MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION
        );
        return;
    }

    RegisterPlugin = (REGISTERPLUGIN)GetProcAddress(hGXInst, "RegisterPlugin");

    if((FARPROC)RegisterPlugin == NULL) printf("Plugin registration failed!");
    RegisterPlugin(&plug);

    if(!IS_DOL_PLUG_GX(plug.type)) printf("Illegal plugin type!");
    
	printf("GX : %s\n", plug.version);

    LoadAPI(GX, GXOPEN, GXOpen);
    LoadAPI(GX, GXCLOSE, GXClose);
    LoadAPI(GX, GXWRITEFIFO, GXWriteFifo);
    LoadAPI(GX, GXSETTOKENS, GXSetTokens);
    LoadAPI(GX, GXCONFIGURE, GXConfigure);
    LoadAPI(GX, GXABOUT, GXAbout);
    LoadAPI(GX, GXSAVELOAD, GXSaveLoad);

    GXSetTokens(&GX_PE_FINISH, &GX_PE_TOKEN, &GX_PE_TOKEN_VALUE);
}

static void PADPluginInit(char *name, int warn)
{
    unsigned long type = 0;

    hPADInst = LoadLibrary(name);
    if(hPADInst == NULL)
    {
        if(warn)
        {
            MessageBox(
                NULL, 
                "Input plugin is not assigned (no PAD support).\n"
                "Please, select input plugin in options.", 
                "Dolwin Plugin System", 
                MB_OK | MB_TOPMOST | MB_ICONEXCLAMATION
            );
        }
        return;
    }
    RegisterPlugin = (REGISTERPLUGIN)GetProcAddress(hPADInst, "RegisterPlugin");

    if((FARPROC)RegisterPlugin == NULL) printf("Plugin registration failed!");
    RegisterPlugin(&plug);

    if(!IS_DOL_PLUG_PAD(plug.type)) printf("Illegal plugin type!");
    
	printf("PAD : %s\n", plug.version);

    LoadAPI(PAD, PADOPEN, PADOpen);
    LoadAPI(PAD, PADCLOSE, PADClose);
    LoadAPI(PAD, PADREADBUTTONS, PADReadButtons);
    LoadAPI(PAD, PADSETRUMBLE, PADSetRumble);
    LoadAPI(PAD, PADCONFIGURE, PADConfigure);
    LoadAPI(PAD, PADABOUT, PADAbout);
    LoadAPI(PAD, PADSAVELOAD, PADSaveLoad);
}

static void GXGekkoPluginInit()
{
    /*unsigned long type = 0;

    RegisterPlugin = &GLXRegisterPlugin;
    RegisterPlugin(&plug);

    if(!IS_DOL_PLUG_GX(plug.type)) printf("Illegal plugin type!");
    
	printf("GX : %s\n", plug.version);

    GXOpen = &GLXOpen;
    GXClose = &GLXClose;
	GXWriteFifo = &GLXWriteFifo;
    GXSetTokens = &GLXSetTokens;
    GXConfigure = &GLXConfigure;
    GXAbout = &GLXAbout;
    GXSaveLoad = &GLXSaveLoad;

	if(GXSetTokens)
		GXSetTokens(&GX_PE_FINISH, &GX_PE_TOKEN, &GX_PE_TOKEN_VALUE);
	else
		MessageBox(wnd.hWnd, "Failed to create GXSetTokens!","ERROR",NULL);*/
}

//////////////////////////////////////////////////////////////////////

void PS_Open()
{
    if(ps_opened == TRUE) return;

    memset(&plug, 0, sizeof(PluginData));

    plug.ram = Mem_RAM;
    plug.display = (void *)&wnd.hWnd;

#ifdef USE_DOLPHIN_PLUGIN
    GXDolphinPluginInit ("GPPlugin.dll", 0);
#else
	GXGekkoPluginInit();
#endif
    PADPluginInit("PADPlugin.dll", 0);

	ps_opened = TRUE;
}

void PS_Close()
{
    if(ps_opened == FALSE) return;

    if(GXClose)  GXClose();
    if(hGXInst)  FreeLibrary(hGXInst);
    
	if(PADClose) PADClose();
    if(hPADInst) FreeLibrary(hPADInst);

    hGXInst = NULL;
    hPADInst = NULL;

    ps_opened = FALSE;
}