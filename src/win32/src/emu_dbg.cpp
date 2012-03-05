// emu_dbg.cpp
// (c) 2005,2006 Gekko Team

//#define WINVER 0x0400
#include "emu.h"
#include <windowsx.h>
#include "hw/hw_pi.h"
#include "hw/hw_vi.h"

//

emuWnd	dbg;
emuWnd	dbgRegs;
u32		dbgRegsTab;
emuWnd	dbgIrq;

//

#include "debugger/CtrlMemView.h"
#include "debugger/CtrlDisAsmView.h"

//

#define YOFFS 20
#define BUTTONOFFS 25;

CtrlMemView *cMemView;
CtrlDisAsmView *cDisView;

char szBuf[256];
char *xs( u32 x ) { sprintf(szBuf, "%X", x); return szBuf; }
char *fls( float f ) { sprintf(szBuf, "%f", f); return szBuf; }
char *fld( double f ) { sprintf(szBuf, "%lf", f); return szBuf; }
char *x8s( u32 x ) { sprintf(szBuf, "%08X", x); return szBuf; }
char *x8sb( u32 x ) { if(x){sprintf(szBuf, "set");}else{sprintf(szBuf, "clear");} return szBuf; }

//

char *GPRList[32] = 
{
 "R0" , "SP" , "SD1", "R3" , "R4" , "R5" , "R6" , "R7" , 
 "R8" , "R9" , "R10", "R11", "R12", "SD2", "R14", "R15", 
 "R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23", 
 "R24", "R25", "R26", "R27", "R28", "R29", "R30", "R31"
};

char *SPRList[32] = 
{
	"PC", "LR", "SRR0", "SRR1", "SPRG0", "SPRG1", "SPRG2", "SPRG3",
	"HID0", "HID1", "HID2", "WPAR", "DMAL", "DMAU", "DSISR", "DAR",
	"MSR", "FPSCR", "CR", "XER", "CTR", "DEC", "EAR", "PVR",
	"", "", "", "", "", "", "", ""
};

u32 ConvertFromHex(char *HexStr)
{
	int i;
	u32 Result = 0;
	u8	NewVal;

	//do a fast convert of a hex string
	for(i=0; i < strlen(HexStr); i++)
	{
		Result <<= 4;
		NewVal = HexStr[i];
		if(NewVal > 0x39)
			NewVal &= 0xDF;

		NewVal = NewVal - 0x30;
		if(NewVal > 9)
			NewVal -= 7;
		Result |= NewVal;
	}

	return Result;
}

void RefreshGPR(void)
{
	u32 i;

	for( i=0; i<32; i++ )
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)x8s(ireg_gpr(i)) );
}

void RefreshFPR(void)
{
	u32 i;

	for( i=0; i<32; i++ )
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)fld(ireg_fpr_64(i, 0)) );
}

void RefreshFPR_PS0(void)
{
	u32 i;

	for( i=0; i<32; i++ )
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)fld(ireg_fpr_32(i, 0)) );
}

void RefreshFPR_PS1(void)
{
	u32 i;

	for( i=0; i<32; i++ )
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)fld(ireg_fpr_32(i, 1)) );
}

void RefreshFPR0Hex(void)
{
	u32 i;
	f32	data;
	for( i=0; i<32; i++ )
	{
		data = (f32)ireg_fpr_64(i, 0);
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)x8s(*(u32 *)&data) );
	}
}

void RefreshFPR1Hex(void)
{
	u32 i;
	f32	data;
	for( i=0; i<32; i++ )
	{
		data = (f32)ireg_fpr_64(i, 1);
		SetDlgItemText( dbgRegs.hWnd, IDS_R0+i, (LPCWSTR)x8s(*(u32 *)&data) );
	}
}

void RefreshSPR(void)
{
	u32 i;

	SetDlgItemText( dbgRegs.hWnd, IDS_R0, (LPCWSTR)x8s(ireg_PC()) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R1, (LPCWSTR)x8s(ireg_spr(I_LR)) );
	for(i=0; i < 2; i++)
		SetDlgItemText( dbgRegs.hWnd, IDS_R2+i, (LPCWSTR)x8s(ireg_spr(I_SRR0 + i)) );
	for(i=0; i < 4; i++)
		SetDlgItemText( dbgRegs.hWnd, IDS_R4+i, (LPCWSTR)x8s(ireg_spr(272 + i)) );
	for(i=0; i < 3; i++)
		SetDlgItemText( dbgRegs.hWnd, IDS_R8+i, (LPCWSTR)x8s(ireg_spr(I_HID0 + i)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R11, (LPCWSTR)x8s(ireg_spr(921)) );
	SetDlgItemText (dbgRegs.hWnd, IDS_R12, (LPCWSTR)x8s(ireg_spr(I_DMAL)) );
	SetDlgItemText (dbgRegs.hWnd, IDS_R13, (LPCWSTR)x8s(ireg_spr(I_DMAU)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R14, (LPCWSTR)x8s(ireg_spr(18)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R15, (LPCWSTR)x8s(ireg_spr(19)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R16, (LPCWSTR)x8s(ireg_MSR()) );	
	SetDlgItemText( dbgRegs.hWnd, IDS_R17, (LPCWSTR)x8s(ireg_FPSCR()) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R18, (LPCWSTR)x8s(ireg_CR()) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R19, (LPCWSTR)x8s(ireg_spr(I_XER)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R20, (LPCWSTR)x8s(ireg_spr(I_CTR)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R21, (LPCWSTR)x8s(ireg_spr(I_DEC)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R22, (LPCWSTR)x8s(ireg_spr(282)) );
	SetDlgItemText( dbgRegs.hWnd, IDS_R23, (LPCWSTR)x8s(ireg_spr(287)) );
}

void RefreshRegs(void)
{
	switch(dbgRegsTab)
	{
		case 0:		//GPR
			RefreshGPR();
			break;
		case 1:		//SPR
			RefreshSPR();
			break;
		case 2:		//FPR
			RefreshFPR();
			break;
		case 3:		//FPR PS0
			RefreshFPR_PS0();
			break;
		case 4:		//FPR PS1
			RefreshFPR_PS1();
			break;
		case 5:		//FPR PS0 Hex
			RefreshFPR0Hex();
			break;
		case 6:		//FPR PS1 Hex
			RefreshFPR1Hex();
			break;
	};
}

void ChangeRegsTab(u32 NewTab)
{
	u32		i;
	char	RegName[10];

	//switch the data around for the new regs
	dbgRegsTab = NewTab;
	switch(dbgRegsTab)
	{
		case 0:		//GPR
			for(i = 0; i < 32; i++)
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)GPRList[i]);
			RefreshGPR();
			break;

		case 1:		//SPR
			for(i = 0; i < 32; i++)
			{
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)SPRList[i]);
				if(strlen(SPRList[i]) == 0)
					SetDlgItemText( dbgRegs.hWnd, IDS_R0 + i, (LPCWSTR)"");
			}
			RefreshSPR();
			break;

		case 2:		//FPR 0
			for(i = 0; i < 32; i++)
			{
				sprintf(RegName, "FPR%d", i);
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)RegName);
			}
			RefreshFPR();
			break;

		case 3:		//FPR PS 0
			for(i = 0; i < 32; i++)
			{
				sprintf(RegName, "FPR%d", i);
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)RegName);
			}
			RefreshFPR_PS0();
			break;

		case 4:		//FPR PS 1
			for(i = 0; i < 32; i++)
			{
				sprintf(RegName, "FPR%d", i);
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)RegName);
			}
			RefreshFPR_PS1();
			break;

		case 5:		//FPR0 Hex
			for(i = 0; i < 32; i++)
			{
				sprintf(RegName, "FPR%d", i);
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)RegName);
			}
			RefreshFPR0Hex();
			break;

		case 6:		//FPR1 Hex
			for(i = 0; i < 32; i++)
			{
				sprintf(RegName, "FPR%d", i);
				SetDlgItemText( dbgRegs.hWnd, IDC_REGLBL0 + i, (LPCWSTR)RegName);
			}
			RefreshFPR1Hex();
			break;

	};
}

BOOL CALLBACK dbgRegGPRProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return TRUE;
	case WM_CLOSE:
	case WM_DESTROY:
		dbgRegs.active = FALSE;
		DestroyWindow(hWnd);
		return TRUE; 

	case WM_NOTIFY: 
		switch (LOWORD(wParam)) 
		{
			case IDC_REGS_TAB:
				LPNMHDR nm = (LPNMHDR)lParam;
				switch(nm->code)
				{
					case TCN_SELCHANGE:
						ChangeRegsTab(TabCtrl_GetCurSel(nm->hwndFrom));
						break;
				};
		};

	default: break;
	}
	return FALSE;
}

void Debugger_Regs_Open(void)
{
	TCITEM	TabItem;
	RECT	rcTab;

	if(dbgRegs.active == false)
	{
		dbgRegs.active = true;

		dbgRegs.hWnd = CreateDialog( wnd.hInstance, MAKEINTRESOURCE(IDD_REGSVIEW), NULL, dbgRegGPRProc);
		if(dbgRegs.hWnd != NULL)
		{
			GetWindowRect( dbgRegs.hWnd, &rcTab );
			rcTab.bottom -= rcTab.top;
			rcTab.right -= rcTab.left;
			SetWindowPos(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), HWND_TOP, 0, 0, rcTab.right, rcTab.bottom, SWP_SHOWWINDOW );
			memset(&TabItem, 0, sizeof(TabItem));
			TabItem.mask = TCIF_TEXT;
			TabItem.pszText = (LPWSTR)"GPR";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 0, &TabItem);
			TabItem.pszText = (LPWSTR)"SPR";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 1, &TabItem);
			TabItem.pszText = (LPWSTR)"FPR";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 2, &TabItem);
			TabItem.pszText = (LPWSTR)"FPR PS0";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 3, &TabItem);
			TabItem.pszText = (LPWSTR)"FPR PS1";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 4, &TabItem);
			TabItem.pszText = (LPWSTR)"FPR PS0 Hex";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 5, &TabItem);
			TabItem.pszText = (LPWSTR)"FPR PS1 Hex";
			TabCtrl_InsertItem(GetDlgItem(dbgRegs.hWnd, IDC_REGS_TAB), 6, &TabItem);
			dbgRegsTab = 0;
			RefreshRegs();
		}
		else
			dbgRegs.active = false;
	}
}

////////////////////////////////////////////////////////////

//

BOOL CALLBACK dbgRegIRQProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return TRUE;
	case WM_CLOSE:
	case WM_DESTROY:
		dbgIrq.active = FALSE;
		DestroyWindow(hWnd);
		return TRUE; 
	default: break;
	}
	return FALSE;
}

void Debugger_RegIRQ_Open(void)
{
	if(dbgIrq.active == false)
	{
		dbgIrq.active = true;

		dbgIrq.hWnd = CreateDialog( wnd.hInstance, MAKEINTRESOURCE(IDD_IRQSTAT), NULL, dbgRegIRQProc);
	}
}

////////////////////////////////////////////////////////////

BOOL CALLBACK ChildDlgProc( HWND hChild, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return TRUE;
	default: break;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////

void RefreshIRQ(void)
{
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQRSWST, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_RSWST) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQHSP, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_HSP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDEBUG, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_DEBUG) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQCP, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_CP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQPEFINISH, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_PEFINISH) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQPETOKEN, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_PETOKEN) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQVI, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_VI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQMEM, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_MEM) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDSP, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_DSP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQAI, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_AI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQEXI, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_EXI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQSI, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_SI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDI, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_DI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQRSW, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_RSW) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQERROR, (LPCWSTR)x8sb(REGPI32(PI_INTSR) & PI_MASK_ERROR) );

	SetDlgItemText( dbgIrq.hWnd, IDS_IRQRSWST2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_RSWST) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQHSP2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_HSP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDEBUG2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_DEBUG) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQCP2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_CP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQPEFINISH2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_PEFINISH) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQPETOKEN2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_PETOKEN) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQVI2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_VI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQMEM2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_MEM) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDSP2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_DSP) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQAI2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_AI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQEXI2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_EXI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQSI2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_SI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQDI2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_DI) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQRSW2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_RSW) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQERROR2, (LPCWSTR)x8sb(REGPI32(PI_INTMR) & PI_MASK_ERROR) );

//	SetDlgItemText( dbgIrq.hWnd, IDS_VIINT0, (LPCWSTR)x8s(REGVI32(VI_DI0)) );

//	SetDlgItemText( dbgIrq.hWnd, IDS_IRQPENDING, (LPCWSTR)x8sb(cpu->gex_pending) );
//	SetDlgItemText( dbgIrq.hWnd, IDS_IRQEXTERNAL, (LPCWSTR)x8sb(cpu->gex_external) );
	SetDlgItemText( dbgIrq.hWnd, IDS_IRQMSREE, (LPCWSTR)x8sb(ireg_MSR() & MSR_BIT_EE) );
}

void RefreshDebugger()
{
	SetDlgItemText( dbg.hWnd, IDS_PC, (LPCWSTR)x8s(ireg_PC()) );
	SetDlgItemText( dbg.hWnd, IDS_IC, (LPCWSTR)x8s(ireg_IC()) );
	//SetDlgItemText( hDlg, IDS_LR, (LPCWSTR)x8s(ireg.spr[8]) );//
	SetDlgItemText( dbg.hWnd, IDS_CR, (LPCWSTR)x8s(ireg_CR()) );
	//SetDlgItemText( hDlg, IDS_CTR, (LPCWSTR)x8s(ireg.spr[9]) );//
	//SetDlgItemText( hDlg, IDS_XER, (LPCWSTR)x8s(ireg.spr[1]) );//
	SetDlgItemText( dbg.hWnd, IDS_MSR, (LPCWSTR)x8s(ireg_MSR()) );

	if(dbgIrq.active) RefreshIRQ();
	if(dbgRegs.active) RefreshRegs();
}

////////////////////////////////////////////////////////////

#define C_PAGES 3 
 
typedef struct tag_dlghdr { 
    HWND hTab;					// tab control 
    HWND hDisplay;				// current child dialog box 
    RECT rcDisplay;				// display rectangle for the tab control 

    DLGTEMPLATE *apRes[C_PAGES]; 
} DLGHDR; 
 
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) 
{ 
    HRSRC hrsrc = FindResource(NULL, (LPWSTR)lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(wnd.hInstance, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 
} 

////////////////////////////////////////////////////////////

void WINAPI OnSizeTab(HWND hDlg) 
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowLong(hDlg, GWL_USERDATA); 

	RECT rcTab, rcDlg;
	GetWindowRect( hDlg, &rcDlg );
	GetWindowRect( pHdr->hTab, &rcTab );
	
	UINT y = rcTab.top - rcDlg.top - YOFFS;
	UINT x = rcTab.left - rcDlg.left;
	UINT w = rcTab.right - rcTab.left;
	UINT h = rcTab.bottom - rcTab.top - BUTTONOFFS;

	HWND hCust = GetDlgItem(pHdr->hDisplay, IDC_VIEW);
	SetWindowPos( hCust, HWND_TOP, 0, 0, w-8, h, SWP_SHOWWINDOW );
	SetWindowPos( pHdr->hDisplay, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW );
} 

//
 
VOID WINAPI OnSelChanged(HWND hDlg) 
{ 
    DLGHDR *pHdr = (DLGHDR *) GetWindowLong( hDlg, GWL_USERDATA); 

    int iSel = TabCtrl_GetCurSel(pHdr->hTab); 
 
    // Destroy the current child dialog box, if any. 
    if (pHdr->hDisplay != NULL) 
        DestroyWindow(pHdr->hDisplay); 
 
    // Create the new child dialog box. 
    pHdr->hDisplay = CreateDialogIndirect(wnd.hInstance, pHdr->apRes[iSel], hDlg, ChildDlgProc); 

	switch( iSel ) {
	case 0: cDisView = CtrlDisAsmView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cMemView=NULL; break;
	case 1: cMemView = CtrlMemView::getFrom(GetDlgItem(pHdr->hDisplay,IDC_VIEW)); cDisView=NULL; break;
	case 2:
	default: cDisView = NULL; cMemView = NULL; break;
	}
	OnSizeTab(hDlg);
} 

//

void WINAPI OnDestroyTab(HWND hDlg)
{
    DLGHDR *pHdr = (DLGHDR *) GetWindowLong( hDlg, GWL_USERDATA); 

	if( IsWindow(pHdr->hDisplay) )
		DestroyWindow( pHdr->hDisplay );
	if( IsWindow(pHdr->hTab) )
		DestroyWindow( pHdr->hTab );
}

//

void WINAPI OnInitTab(HWND hDlg)
{
	TCITEM tci; 
	DLGHDR *pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR)); 

	SetWindowLong(hDlg, GWL_USERDATA, (LONG) pHdr);

	pHdr->hTab = GetDlgItem(hDlg, IDC_VIEWSEL);
	
	// Add a tab for each of the three child dialog boxes. 
    tci.mask = TCIF_TEXT | TCIF_IMAGE; 
    tci.iImage = -1; 
    tci.pszText = (LPWSTR)"DisAsm"; 
    TabCtrl_InsertItem(pHdr->hTab, 0, &tci); 
    tci.pszText = (LPWSTR)"Memory"; 
    TabCtrl_InsertItem(pHdr->hTab, 1, &tci); 
    //tci.pszText = "PVR List"; 
    //TabCtrl_InsertItem(pHdr->hTab, 2, &tci); 
 
    // Lock the resources for the three child dialog boxes. 
    pHdr->apRes[0] = DoLockDlgRes((LPCSTR)MAKEINTRESOURCE(IDD_DISVIEW)); 
    pHdr->apRes[1] = DoLockDlgRes((LPCSTR)MAKEINTRESOURCE(IDD_MEMVIEW__)); 
 
    OnSelChanged(hDlg);
}

//

typedef struct tagSEL
{
	HWND      hwndFrom;
	UINT_PTR  idFrom;
	UINT      _code;
}   SEL;

//
#include <stack>
std::stack<u32> callStack; //stores from where a function was called

bool isBlr(u32 inst, u32& to)
{
	if(((inst&0xfe000000)>>26) == 19 && ((inst>>1) & 0x3FF) == 16) //bclrx
	{
		bool B = ireg_CR() & (1 << (31 - ( inst >> 16 ) & 0x1F));
		if((inst & 1) == 0) //branch taken
		{
			bool b = false;

			switch( ((( inst >> 21 ) & 0x1F) >> 1 ) & 0xf ) //branch prediction??
			{
				case 0:  if( !B && ( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 1:  if( !B && !( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 2:
				case 3:  if( !B ){ b = true; } break;
				case 4:  if( !B && ( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 5:  if( !B && !( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 6:
				case 7:  if( B ){ b = true; } break;
				case 8:  if( ( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 9:  if( !( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 10:
				case 11: b = true; break;
				case 12: if( ( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 13: if( !( ireg_spr(I_CTR) - 1 ) ){ b = true; } break;
				case 14:
				case 15: b = true; break;
			}

			if(b)
			{
				to = (ireg_spr(I_LR) & ~3);
				return true;
			}
		}
	}
	return false;
}

bool isBl(u32 inst, u32& to)
{
	if(((inst&0xfe000000)>>26) == 18 && (inst & 3) == 1) //bl
	{
		return to = ireg_PC() + (((s32)(inst&0x03FFFFFC)<<6)>>6);
	}
	return false;
}

// [F|RES]: rewritten ... but i havnt really looked to the old stuff
void dbgUseCallstack(void)
{
    HWND hwndCallstack = GetDlgItem(dbg.hWnd, IDC_CALLSTACK);
    ListBox_ResetContent(hwndCallstack);
    
    int Count = 1;
    {
        std::string SymbolPC;
        std::string SymbolLR;

        // add current PC
        {
            HLE_MapGetDebugSymbol(ireg_PC(), SymbolPC);
            int Index = ListBox_AddString(hwndCallstack, SymbolPC.c_str());
            ListBox_SetItemData(hwndCallstack, Index, ireg_PC());
        }
        
		HLE_MapGetDebugSymbol(ireg_spr(I_LR), SymbolLR);
		if (SymbolPC != SymbolLR)
		{
			int Index = ListBox_AddString(hwndCallstack, SymbolLR.c_str());
			ListBox_SetItemData(hwndCallstack, Index, ireg_spr(I_LR));
			Count++;
		}
    }
    
    //walk the stack chain
    u32 CallstackAddress = Memory_Read32(ireg_gpr(1));
    while (CallstackAddress != 0xFFFFFFFF && CallstackAddress!=0 && Count++<20)
    {
        DWORD FunctionAddress = Memory_Read32(CallstackAddress+4);

        std::string SymbolName;
        if (!HLE_MapGetDebugSymbol(FunctionAddress, SymbolName))
		{
            SymbolName = "(unknown)";
			printf(".Debugger: Callstack (unknown) Address: %08X\n",FunctionAddress);
		}

        int Index = ListBox_AddString(hwndCallstack, SymbolName.c_str());
        ListBox_SetItemData(hwndCallstack, Index, FunctionAddress);

        CallstackAddress = Memory_Read32(CallstackAddress);
    }
    ListBox_SetCurSel(hwndCallstack, 0);
}

// Desc: For Populating the HLE Func list
//

#define MAX_HLEFUNC_TOCHECK	0xFFFFF

u32 table_of_function_offsets[MAX_HLEFUNC_TOCHECK];

void dbgPopulateHLEFunctionList(HWND hWnd)
{
   int n = 0;
   std::map<u32, Function>::iterator it, end = maps.end();
   for(it = maps.begin(); it != end; ++it)
   {
       std::string name = it->second.funcName;
       u32 address = it->first;

       table_of_function_offsets[n] = address;
       n++;
       SendDlgItemMessage(hWnd, IDC_FUNCTIONLIST, LB_ADDSTRING, 0,
(LPARAM)name.c_str());
   }
}

u32 dbgGetHLEFunctionAddress(HWND hWnd)
{
	int id = SendDlgItemMessage(hWnd,IDC_FUNCTIONLIST,LB_GETCURSEL,0,0);
	return table_of_function_offsets[id];
}

// Desc: Main DBG Wind Proc
//

void dbgSetCursor(u32 addr)
{
	cDisView->gotoAddr(addr);
}

BOOL CALLBACK dbgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	char			temp[1024];
	int				iSel = 0;
	static char		szAddr[32];
	static DWORD	dwIndex = 0, dwAddr = 0, i = 0;
	int				dbg_loop = 0;
	u32				ref_pc = 0;
	DLGHDR			*pHdr;

	u32 tmp, stackHeight = 0;

	switch( uMsg )
	{
	case WM_INITDIALOG:

		OnInitTab(hWnd);
		OnSizeTab(hWnd);
		RefreshDebugger();
		dbgPopulateHLEFunctionList(hWnd);

		return true;

	case WM_COMMAND: 

		switch (LOWORD(wParam)) 
		{
		case IDC_FIND:
			MessageBox( hWnd, (LPCWSTR)"FIND", (LPCWSTR)"", MB_OK );
			return true;

		case IDB_STOP:
			printf("Debugger : Stop @ 0x%08X\n",ireg_PC());
			emu.status = EMU_HALTED;
			goto refresh;

		case IDB_START:
			emu.status = EMU_RUNNING;

			goto refresh;

		case IDB_STEPOUTT:
			// Keep getting compiler errors on this?
			cpu->step = true;
			while(!isBlr(Memory_Read32(ireg_PC()), tmp) || stackHeight > 0)
			{
				if(isBl(Memory_Read32(ireg_PC()), tmp))
				{
					callStack.push(ireg_PC());
					++stackHeight;
				}
				else if(isBlr(Memory_Read32(ireg_PC()), tmp))
				{
					if(tmp != callStack.top() + 4)
						printf("DBG FATAL: lost track of call stack. ARGH.\n");
					callStack.pop();
					--stackHeight;
				}

				cpu->execStep();
			}
			cpu->step = false;

		//fall though to IDB_STEP to execute the final BLR extruction

		case IDB_STEP:
		{
			printf("Debugger : Step @ 0x%08X\n",ireg_PC());			

            // exec next step
			cpu->step = true;
			cpu->execStep();
			cpu->step = false;
            dbgUseCallstack();

			goto refresh;
		}

		case IDB_SKIP:
			printf("Debugger : Skip @ 0x%08X\n",ireg_PC());
			set_ireg_PC(ireg_PC() + 4);
//			cpu->pPC++;
			goto refresh;

		case IDB_STEPOV:
			printf("Debugger : StepOver\n");
			ref_pc=ireg_PC()+4;

			cpu->step = true;
			for(dbg_loop=0;dbg_loop<0xFFFFF;dbg_loop++)
			{
				//XXX this doesn't work for branch instructions
				if(ireg_PC()==ref_pc) break;
				cpu->execStep();
			}
			cpu->step = false;
			goto refresh;

		case IDC_EVENT:

			break;
		case IDB_PAUSE:
			printf("Debugger : Pause @ 0x%08X\n",ireg_PC());
			cpu->pause = TRUE;
            dbgUseCallstack();
			goto refresh;

		case IDB_UNPAUSE:
			printf("Debugger : Unpause @ 0x%08X\n",ireg_PC());
			cpu->pause = FALSE;
			goto refresh;

		case IDB_MOVECURSORTOO:
			dwAddr = dbgGetHLEFunctionAddress(hWnd);
			if(dwAddr)
			{
				cDisView->gotoAddr(dwAddr);
			}
			printf("Debugger : Move Cursor to HLE Address %08X\n",dwAddr);
				
			return true;

		case IDB_RUNPCTOO:
			dwAddr = dbgGetHLEFunctionAddress(hWnd);

			cpu->step = true;
			for(dbg_loop=0;dbg_loop<0xFFFFF;dbg_loop++)
			{
				if(ireg_PC()==dwAddr) break;
				cpu->execStep();
			}
			cpu->step = false;

			printf("Debugger : Run PC Too HLE Address %08X\n",dwAddr);
				
			goto refresh;

		case IDB_SKIPPCTOO:
			dwAddr = dbgGetHLEFunctionAddress(hWnd);

			set_ireg_PC(dwAddr);

//			cpu->pPC =(u32*)&Mem_RAM[ireg_PC() & RAM_MASK];

			printf("Debugger : Skip PC Too HLE Address %08X\n",dwAddr);
				
			goto refresh;

		case IDB_REGISTERS_VIEW:
			if(!dbgRegs.active)
				Debugger_Regs_Open();
			else
			{
				dbgRegs.active = false;
				DestroyWindow(dbgRegs.hWnd);
			}

			return true;

		case IDB_INTERRUPT_STATUS:
			Debugger_RegIRQ_Open();
			return true;

		case IDB_GOTO:
			memset(szAddr, 0x30, sizeof(szAddr) - 1);
			GetDlgItemText( hWnd, IDC_ADDR, (LPWSTR)szAddr, 8);
			dwAddr = ConvertFromHex(szAddr);
			cDisView->gotoAddr(dwAddr);
			return true;

		case IDB_SETPC: //(new)
			ZeroMemory(temp,128);
			GetWindowText(GetDlgItem(hWnd,IDC_EDITPC),(LPWSTR)temp,127);
			int newpc; sscanf(temp,"0x%08X",&newpc);
			set_ireg_PC(newpc);
//			cpu->pPC = (u32*)&Mem_RAM[ireg_PC() & RAM_MASK];
			cDisView->gotoAddr(ireg_PC());
			return true;
		case IDC_VSYNC:
			REGVI16(VI_DI0) |= 0x8000;
			PI_RequestInterrupt(PI_MASK_VI);
			cpu->Exception(GekkoCPU::GEX_EXT);
			break;

		case IDB_RUNPCTO: //(new)
			ZeroMemory(temp,128);
			GetWindowText(GetDlgItem(hWnd,IDC_EDITPC),(LPWSTR)temp,127);
			sscanf(temp,"0x%08X",&dwAddr);

			printf("Debugger : Run PC To Address %08X\n",dwAddr);

			cpu->step = true;
			//for(dbg_loop=0;dbg_loop<0xFFFFF;dbg_loop++)
			while (1)
			{
				cpu->execStep();
				if(ireg_PC()==dwAddr) break;
			}
			cpu->step = false;

			printf("Debugger : Got PC To Address %08X\n",dwAddr);

			dbgUseCallstack();

			goto refresh;

		case IDB_SETCURSOR: //(new)
			GetDlgItemText( hWnd, IDC_EDITPC, (LPWSTR)szAddr, 9);
			dwAddr = ConvertFromHex(szAddr);

			pHdr = (DLGHDR *) GetWindowLong( hWnd, GWL_USERDATA); 
			iSel = TabCtrl_GetCurSel(pHdr->hTab); 
			switch(iSel)
			{
				case 0:
					cDisView->gotoAddr(dwAddr);
					break;
				case 1:
					cMemView->gotoAddr(dwAddr);
					break;
			};
			return true;

		//case IDC_XF:
		case IDC_FPSEL1:
		case IDC_FPSEL2:
		case IDC_FPSEL3:
			RefreshDebugger();
			return true;

		default: break;
		}
		break;


	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom)
		{
		case IDC_VIEWSEL:	

			if(TCN_SELCHANGE == ((SEL *)lParam)->_code)
			{
				OnSelChanged(hWnd);
				goto refresh;
			}
			break;
		}
		break;
		

	case WM_CLOSE:
	case WM_DESTROY:
		dbg.active = false;
		OnDestroyTab(hWnd);
		DestroyWindow(hWnd);
		return TRUE; 

	case WM_MOVE:
	case WM_SIZE:
		goto refresh;
	}

	return false;

refresh:
	
	if( cDisView != NULL )
		cDisView->gotoPC();
	if( cMemView != NULL )
		cMemView->gotoAddr(cMemView->getSelection());

	RefreshDebugger();
	return true;
}

////////////////////////////////////////////////////////////

void Debugger_Open(void)
{
	if(dbg.active == false)
	{
		dbg.active = true;

		CtrlMemView::init();
		CtrlDisAsmView::init();

		dbg.hWnd = CreateDialog( wnd.hInstance, MAKEINTRESOURCE(IDD_GEKKODEBUG), NULL, dbgProc);

		printf("Emu: Debugger Opened.\n");
	}
}