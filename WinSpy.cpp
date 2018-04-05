/***************************************************************
Module name: WinSpy.cpp
Copyright (c) 2003 Robert Kuster

Notice:	If this code works, it was written by Robert Kuster.
		Else, I don't know who wrote it.

		Use it on your own risk. No responsibilities for
		possible damages of even functionality can be taken.
***************************************************************/

#include <windows.h>
#include <stdio.h>     // sprintf
#include <tchar.h>

#include "InjCode.h"
#include "resource.h"


//-----------------------------------------------
// global variables & forward declarations
//
HINSTANCE	hInst;

HWND		hStatic;
HWND		hWndOld;

bool		bOverPasswdEdit = false;	// cursor over edit control with 
										// ES_PASSWORD style?
HBRUSH		brush			=::CreateSolidBrush (RGB(255,255,160));

HCURSOR 	hCurCross;
HCURSOR 	hCurHot;
HCURSOR 	hCurNormal;


BOOL CALLBACK MainDlgProc (HWND,UINT,WPARAM,LPARAM);
void OnMouseMove		  (HWND hDlg, POINT &pt);

void HighlightWindow	  (HWND hwnd, BOOL fDraw);
HWND SmallestWindowFromPoint(const POINT point);


//-----------------------------------------------
// WinMain
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{

	hInst = hInstance;
	// display main dialog 
	::DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_WININFO), NULL, MainDlgProc, NULL);

	return 0;
}


//-----------------------------------------------
// MainDlgProc
// Notice: dialog procedure
//
BOOL CALLBACK MainDlgProc (HWND hDlg,	// handle to dialog box
						   UINT uMsg,      // message
						   WPARAM wParam,  // first message parameter
						   LPARAM lParam ) // second message parameter
{
	POINT pt;
	RECT rc;
	static bCapture = false;

	static HBITMAP		hBmpCross;
	static HBITMAP		hBmpBlank;
	

	switch (uMsg) {

	case WM_INITDIALOG:
		hStatic		= ::GetDlgItem(hDlg,IDC_CAPTURE);
		hBmpCross	= LoadBitmap (hInst, MAKEINTRESOURCE(IDB_CROSS));
		hBmpBlank	= LoadBitmap (hInst, MAKEINTRESOURCE(IDB_BLANK));
		hCurCross	= LoadCursor (hInst, MAKEINTRESOURCE(IDC_CROSS0));
		hCurHot		= LoadCursor (hInst, MAKEINTRESOURCE(IDC_CROSS1));
		hCurNormal  = LoadCursor (NULL, IDC_ARROW);
		return true;
		
	case WM_LBUTTONDOWN:
		pt.x = MAKEPOINTS(lParam).x;
		pt.y = MAKEPOINTS(lParam).y;		
		::ClientToScreen (hDlg, &pt);
		
		::GetWindowRect(hStatic, &rc);

		if( ::PtInRect(&rc, pt) )
		{
			::SetCursor( hCurCross );
			::SendMessage (hStatic,STM_SETIMAGE,IMAGE_BITMAP,(long)hBmpBlank);
			::SetCapture( hDlg );
			bCapture = true;
		}
		break;
	
	case WM_LBUTTONUP:
	case WM_KILLFOCUS:
		if( bCapture )
		{
			if( hWndOld )
				HighlightWindow(hWndOld,FALSE);
			hWndOld = NULL;

			::SetCursor( hCurNormal );
			::SendMessage (hStatic,STM_SETIMAGE,IMAGE_BITMAP,(long)hBmpCross);
			::ReleaseCapture();
			bCapture = false;
		}
		break;

	case WM_MOUSEMOVE:
		if (bCapture) {
			pt.x = MAKEPOINTS(lParam).x;
			pt.y = MAKEPOINTS(lParam).y;			
			::ClientToScreen (hDlg, &pt);
			OnMouseMove (hDlg, pt);
		}
		break;

	case WM_CTLCOLOREDIT:
		if (bOverPasswdEdit)
		{
			HDC hdc = (HDC) wParam;
			if (GetDlgItem (hDlg,IDC_EDIT3) == (HWND) lParam)
			{
				SetBkColor (hdc, RGB(255,255,160));
				return (BOOL) brush;
			}
		}
		break;

	case WM_CLOSE:
		::KillTimer (hDlg,100);
		::EndDialog (hDlg, 0);
		return true;		
	}

	return false;
}


//-----------------------------------------------
// OnMouseMove
// Notice: updates edit controls
//
void OnMouseMove (HWND hDlg, POINT &pt)
{
//	HWND hWnd = ::WindowFromPoint (pt);
	HWND hWnd = SmallestWindowFromPoint (pt);

	DWORD PID, TID;
	TID = ::GetWindowThreadProcessId (hWnd, &PID);

	if (GetCurrentProcessId () == PID)
		return;	
	 
	if (hWndOld == hWnd) return;	// prevent flickering
	if(hWndOld) 
		HighlightWindow(hWndOld,FALSE);
	HighlightWindow(hWnd,TRUE);
	hWndOld = hWnd;

	_TCHAR ch[128];         // text buffer

	/////////////////////////////////////////////////////////////
	//
	// Insert data into edit ctrls
	//
	// 1. parent window
	//
	HWND hParent = ::GetParent (hWnd);
	if (hParent != NULL) 
	{
		_stprintf (ch, __TEXT("0x%08X"), hParent);
		::SetDlgItemText (hDlg, IDC_EDIT9, ch);		// parent handle

		::GetWindowText (hParent, ch, 128);
		::SetDlgItemText (hDlg, IDC_EDIT10, ch);    // parent text

		::GetClassName (hParent, ch, 128);
		::SetDlgItemText (hDlg, IDC_EDIT11, ch);    // parent class name
	}
	else
	{
		::SetDlgItemText (hDlg, IDC_EDIT9,  __TEXT("N/A"));
		::SetDlgItemText (hDlg, IDC_EDIT10, __TEXT("N/A"));
		::SetDlgItemText (hDlg, IDC_EDIT11, __TEXT("N/A"));
	}
	//	

	//**//
	//
	// 2. window under cursor
	//
	_stprintf (ch, __TEXT("0x%08X"), hWnd);
	::SetDlgItemText (hDlg, IDC_EDIT1, ch);      // window handle

	int nID = GetWindowLong(hWnd, GWL_ID);
	_stprintf (ch, __TEXT("%d"), nID);
	::SetDlgItemText (hDlg, IDC_EDIT2, ch);      // control ID
	
	DWORD style = (DWORD)::GetWindowLong (hWnd, GWL_STYLE);
	_stprintf (ch, __TEXT("0x%08X"), style);	
	::SetDlgItemText (hDlg, IDC_EDIT5, ch);      // window style

	RECT rc; 
	::GetWindowRect(hWnd, &rc);
	_stprintf (ch, __TEXT("(%d, %d)-(%d, %d) %dx%d"), rc.left, rc.top, rc.right, rc.bottom,
													  rc.right - rc.left, rc.bottom - rc.top);
	::SetDlgItemText (hDlg, IDC_EDIT6, ch);      // RECT

	_stprintf (ch, __TEXT("0x%08X"), TID);
	::SetDlgItemText (hDlg, IDC_EDIT7, ch);      // thread ID

	_stprintf (ch, __TEXT("0x%08X"), PID);
	::SetDlgItemText (hDlg, IDC_EDIT8, ch);      // process ID


	::GetClassName (hWnd, ch, 128);
	::SetDlgItemText (hDlg, IDC_EDIT4, ch);      // class name
	
	if (GetCurrentProcessId () != PID)
	{
		if ((!_tcsicmp(ch,__TEXT("EDIT"))	|| 
			!_tcsicmp(ch, __TEXT("TEDIT"))	|| 
			!_tcsicmp(ch, __TEXT("ThunderTextBox")) ||
			!_tcsicmp(ch, __TEXT("ThunderRT6TextBox")) ||
			!_tcsicmp(ch, __TEXT("Iris.Password"))) &&			
			style & ES_PASSWORD)
		{	// cursor over Edit Control with ES_PASSWORD
			bOverPasswdEdit = true;
			::SetCursor( hCurHot );
			
			HANDLE hProcess = 
			OpenProcess(
				PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
				FALSE, PID);		

			if (hProcess != NULL) {
				MessageBeep(MB_OK);
				GetWindowTextRemote (hProcess,hWnd,ch);
				CloseHandle( hProcess );
			}
			else 
				_tcscpy (ch,__TEXT(""));
		}
		else {
			::SendMessage (hWnd, WM_GETTEXT, 128, (LPARAM)ch);
			bOverPasswdEdit = false;
			::SetCursor( hCurCross );
		}
	}
	::SetDlgItemText (hDlg, IDC_EDIT3,ch );         // window text
}
 

//-----------------------------------------------
// HighlightWindow
// Notice: from MSDN Spy Sample
//
void HighlightWindow( HWND hwnd, BOOL fDraw )
{
#define DINV                3
	HDC hdc;
    RECT rc;
    BOOL bBorderOn;
    bBorderOn = fDraw;

    if (hwnd == NULL || !IsWindow(hwnd))
        return;

    hdc = ::GetWindowDC(hwnd);
    ::GetWindowRect(hwnd, &rc);
    ::OffsetRect(&rc, -rc.left, -rc.top);

    if (!IsRectEmpty(&rc))
    {
        PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, DINV,  DSTINVERT);
        PatBlt(hdc, rc.left, rc.bottom - DINV, DINV,
            -(rc.bottom - rc.top - 2 * DINV), DSTINVERT);
        PatBlt(hdc, rc.right - DINV, rc.top + DINV, DINV,
            rc.bottom - rc.top - 2 * DINV, DSTINVERT);
        PatBlt(hdc, rc.right, rc.bottom - DINV, -(rc.right - rc.left),
            DINV, DSTINVERT);
    }

    ::ReleaseDC(hwnd, hdc);
} 


//-----------------------------------------------
// SmallestWindowFromPoint
// Notice: from PasswordSpy by Brian Friesen
//
// Find the smallest window still containing the point
//
// WindowFromPoint returns the first window in the Z-order ->
// if the password control is sorounded by a Group Box or some other control,
// WindowFromPoint returns the handle to the sorounding control instead
// to the password control.
//
HWND SmallestWindowFromPoint( const POINT point )
{	
	RECT rect, rcTemp;
	HWND hParent, hWnd, hTemp;

	hWnd = ::WindowFromPoint( point );
	if( hWnd != NULL )
	{
		::GetWindowRect( hWnd, &rect );
		hParent = ::GetParent( hWnd );

		// Has window a parent?
		if( hParent != NULL )
		{
			// Search down the Z-Order
			hTemp = hWnd;
			do{
				hTemp = ::GetWindow( hTemp, GW_HWNDNEXT );

				// Search window contains the point, hase the same parent, and is visible?
				::GetWindowRect( hTemp, &rcTemp );
				if(::PtInRect(&rcTemp, point) && ::GetParent(hTemp) == hParent && ::IsWindowVisible(hTemp))
				{
					// Is it smaller?
					if(((rcTemp.right - rcTemp.left) * (rcTemp.bottom - rcTemp.top)) < ((rect.right - rect.left) * (rect.bottom - rect.top)))
					{
						// Found new smaller window!
						hWnd = hTemp;
						::GetWindowRect(hWnd, &rect);
					}
				}
			}while( hTemp != NULL );
		}
	}

	return hWnd;
}