#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include "emu.h"
#include "CtrlMemView.h"

#define ReadMem MEM_Read

TCHAR CtrlMemView::szClassName[] = _T("CtrlMemView");

void CtrlMemView::init()
{
    WNDCLASSEX wc;
    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = GetModuleHandle(0);
    wc.lpfnWndProc    = CtrlMemView::wndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
    wc.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.style          = 0;
    wc.cbClsExtra     = 0;
	wc.cbWndExtra     = sizeof( CtrlMemView * );
    wc.hIconSm        = 0;
	
    RegisterClassEx(&wc);
}

void CtrlMemView::deinit()
{
}

extern BOOL CopyTextToClipboard(HWND hwnd, TCHAR *text);

LRESULT CALLBACK CtrlMemView::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CtrlMemView *ccp = CtrlMemView::getFrom(hwnd);
	static bool lmbDown=false,rmbDown=false;
    switch(msg)
    {
	case WM_COMMAND:
		break;
    case WM_NCCREATE:
        // Allocate a new CustCtrl structure for this window.
        ccp = new CtrlMemView(hwnd);

		ccp->setDebugger(&di);
		ccp->setMode(MV_NORMAL);
		
        // Continue with window creation.
        return ccp != NULL;
		
		// Clean up when the window is destroyed.
    case WM_NCDESTROY:
        delete ccp;
        break;
	case WM_SETFONT:
		break;
	case WM_SIZE:
		ccp->redraw();
		break;
	case WM_PAINT:
		ccp->onPaint(wParam,lParam);
		break;	
	case WM_VSCROLL:
		ccp->onVScroll(wParam,lParam);
		break;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_KEYDOWN:
		ccp->onKeyDown(wParam,lParam);
		break;
	case WM_LBUTTONDOWN: SetFocus(hwnd); lmbDown=true; ccp->onMouseDown(wParam,lParam,1); break;
	case WM_RBUTTONDOWN: rmbDown=true; ccp->onMouseDown(wParam,lParam,2); break;
	case WM_MOUSEMOVE:   ccp->onMouseMove(wParam,lParam,(lmbDown?1:0) | (rmbDown?2:0)); break;
	case WM_LBUTTONUP:   lmbDown=false; ccp->onMouseUp(wParam,lParam,1); break;
	case WM_RBUTTONUP:   rmbDown=false; ccp->onMouseUp(wParam,lParam,2); break;
	case WM_SETFOCUS:
		SetFocus(hwnd);
		ccp->hasFocus=true;
		ccp->redraw();
		break;
	case WM_KILLFOCUS:
		ccp->hasFocus=false;
		ccp->redraw();
		break;
    default:
        break;
    }
	
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

CtrlMemView *CtrlMemView::getFrom(HWND hwnd)
{
    return (CtrlMemView *)GetWindowLong(hwnd, 0);
}

CtrlMemView::CtrlMemView(HWND _wnd)
{
	wnd=_wnd;
	SetWindowLong(wnd, 0, (LONG)this);
	SetWindowLong(wnd, GWL_STYLE, GetWindowLong(wnd,GWL_STYLE) | WS_VSCROLL);
	SetScrollRange(wnd, SB_VERT, -1,1,TRUE);
	font = CreateFont(16,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH, (LPCWSTR)"Andale Mono");
	curAddress=0;
	rowHeight=16;
	align=4;
	selecting=false;
	mode=MV_SYMBOLS;
}


CtrlMemView::~CtrlMemView()
{
	DeleteObject(font);
}

void CtrlMemView::onPaint(WPARAM wParam, LPARAM lParam)
{
	GetClientRect(wnd, &rect);
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(wnd, &ps);
	int width = rect.right;
	int numRows=(rect.bottom/rowHeight)/2+1;
	SetBkMode(hdc, TRANSPARENT);
	HPEN nullPen=CreatePen(0,0,0xFFFFFF);
	HPEN currentPen=CreatePen(0,0,0);
	HPEN selPen=CreatePen(0,0,0x808080);
	LOGBRUSH lbr;
	lbr.lbHatch=0; lbr.lbStyle=0; 
	lbr.lbColor=0xFFFFFF;
	HBRUSH nullBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0xFFEfE8;
	HBRUSH currentBrush=CreateBrushIndirect(&lbr);
	lbr.lbColor=0x70FF70;
	HBRUSH pcBrush=CreateBrushIndirect(&lbr);
	HPEN oldPen=(HPEN)SelectObject(hdc,nullPen);
	HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,nullBrush);
   	HFONT oldFont = (HFONT)SelectObject(hdc,(HGDIOBJ)font);

	int i;
	for (i=-numRows; i<=numRows; i++)
	{
		unsigned int address=curAddress + i*align;

		int rowY1 = rect.bottom/2 + rowHeight*i - rowHeight/2;
		int rowY2 = rect.bottom/2 + rowHeight*i + rowHeight/2;

		char temp[256];
		sprintf(temp,"%08x",address);

		Rectangle(hdc,0,rowY1,16,rowY2);

		if(selecting && address == selection)
			SelectObject(hdc,selPen);
		else
			SelectObject(hdc,i==0 ? currentPen : nullPen);

		Rectangle(hdc,16,rowY1,width,rowY2);
		SelectObject(hdc,nullBrush);
		SetTextColor(hdc,0x600000);
		TextOut(hdc,17,rowY1,(LPCWSTR)temp,strlen(temp));
		SetTextColor(hdc,0x000000);
		
		switch(mode)
		{
		case MV_NORMAL:
			sprintf(temp, "%08X  %08X  %08X  %08X",
				Memory_Read32((address+0)), Memory_Read32((address+4)), Memory_Read32((address+8)), Memory_Read32((address+12)));
			TextOut(hdc,90,rowY1,(LPCWSTR)temp,strlen(temp));

			SetTextColor(hdc,0x0033BB22);
			((u32*)temp)[0] = Memory_Read32(address);
			((u32*)temp)[1] = Memory_Read32(address+4);
			((u32*)temp)[2] = Memory_Read32(address+8);
			((u32*)temp)[3] = Memory_Read32(address+12);
			((u32*)temp)[4] = 0x00000000;
			TextOut(hdc,420,rowY1,(LPCWSTR)temp,strlen(temp));
		break;

		case MV_SYMBOLS:
			SetTextColor(hdc,0x0000FF);
			int fn = address&3;
			sprintf(temp,"MV_SYMBOLS !!!");
			TextOut(hdc,200,rowY1,(LPCWSTR)temp,strlen(temp));

			SetTextColor(hdc,0x0000000);
			UINT value = 0xBADC0D3;
			sprintf(temp, "%08x", value );
			TextOut(hdc,70,rowY1,(LPCWSTR)temp,strlen(temp));
		break;
		}
	}

	SelectObject(hdc,oldFont);
	SelectObject(hdc,oldPen);
	SelectObject(hdc,oldBrush);
	
	DeleteObject(nullPen);
	DeleteObject(currentPen);
	DeleteObject(selPen);

	DeleteObject(nullBrush);
	DeleteObject(pcBrush);
	DeleteObject(currentBrush);
	
	EndPaint(wnd, &ps);
}



void CtrlMemView::onVScroll(WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(this->wnd, &rect);
	int page=(rect.bottom/rowHeight)/2-1;

	switch (wParam & 0xFFFF)
	{
	case SB_LINEDOWN:
		curAddress+=align;
		break;
	case SB_LINEUP:
		curAddress-=align;
		break;
	case SB_PAGEDOWN:
		curAddress+=page*align;
		break;
	case SB_PAGEUP:
		curAddress-=page*align;
		break;
	default:
		return;
	}
	redraw();
}

void CtrlMemView::onKeyDown(WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	GetClientRect(this->wnd, &rect);
	int page=(rect.bottom/rowHeight)/2-1;

	switch (wParam & 0xFFFF)
	{
	case VK_DOWN:
		curAddress+=align;
		break;
	case VK_UP:
		curAddress-=align;
		break;
	case VK_NEXT:
		curAddress+=page*align;
		break;
	case VK_PRIOR:
		curAddress-=page*align;
		break;
	default:
		return;
	}
	redraw();
}

void CtrlMemView::redraw()
{
	InvalidateRect(wnd, NULL, FALSE);
	UpdateWindow(wnd); 
}

void CtrlMemView::onMouseDown(WPARAM wParam, LPARAM lParam, int button)
{	
	int x = LOWORD(lParam); 
	int y = HIWORD(lParam); 
	if (x>16)
	{
		oldSelection=selection;
		selection=yToAddress(y);
		SetCapture(wnd);
		bool oldselecting=selecting;
		selecting=true;
		if (!oldselecting || (selection!=oldSelection))
			redraw();
	}
}

void CtrlMemView::onMouseUp(WPARAM wParam, LPARAM lParam, int button)
{
	if (button==2)
	{
		POINT pt;
		GetCursorPos(&pt);
		return;
	}
	int x = LOWORD(lParam); 
	int y = HIWORD(lParam); 
	if (x>16)
	{
		curAddress=yToAddress(y);
		selecting=false;
		ReleaseCapture();
		redraw();
	}
}

void CtrlMemView::onMouseMove(WPARAM wParam, LPARAM lParam, int button)
{
	if (button&1)
	{
		int x = LOWORD(lParam); 
		int y = (signed short)HIWORD(lParam); 
		if (x>16)
		{
			if (y<0)
			{
				curAddress-=align;
				redraw();
			}
			else if (y>rect.bottom)
			{
				curAddress+=align;
				redraw();
			}
			else
				onMouseDown(wParam,lParam,1);
		}
	}
}	

int CtrlMemView::yToAddress(int y)
{
	int ydiff=y-rect.bottom/2-rowHeight/2;
	ydiff=(int)(floorf((float)ydiff / (float)rowHeight))+1;
	return curAddress + ydiff * align;
}