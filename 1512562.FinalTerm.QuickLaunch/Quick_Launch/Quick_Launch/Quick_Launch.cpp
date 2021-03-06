// Quick_Launch.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Quick_Launch.h"
#include <shellapi.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <locale>
#include <codecvt>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <msi.h>
#include <Shlobj.h>
#pragma comment(lib, "Msi.lib")
using namespace std;
#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;
#define MAX_LOADSTRING 100

struct AppInfo
{
	wstring appname;
	wstring apppath;
	int count = 0;
};
// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_hWnd;
HHOOK hook;
HWND ToolBarText;
HWND TextSearch;
HWND ListBox;
HWND btn;
WCHAR keysearch[255];
vector <AppInfo> list;
vector <AppInfo> VectorListbox;
vector<AppInfo> usageApp;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//********************************************************************
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
void ScanFile(WCHAR* link);
void SearchKeyApp(wstring keysearch);
void LoadDataToListBox(vector <AppInfo> listdata);
void ReadProgramFrequencyToFile(wstring filePath);
void WriteProgramFrequencyToFile(wstring filePath);
void UpdateFrequency();
void fillRectangle(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);

/////////////////****************************************************
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_QUICKLAUNCH, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
	
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QUICKLAUNCH));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QUICKLAUNCH));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_QUICKLAUNCH);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND hWnd;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 450, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
void LoadIcon(HWND hwnd)
{
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(nid);
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = ICONHIDE;
	wcscpy_s(nid.szTip, L"Quick Launch");
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALL));
	Shell_NotifyIcon(NIM_ADD, &nid);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case ICONHIDE:
	{
		switch (lParam)
		{
		case WM_LBUTTONUP:
			if (IsWindowVisible(hWnd) == false)
				ShowWindow(hWnd, 1);
			else
				ShowWindow(hWnd, 0);
			break;
		case WM_RBUTTONUP:
		{
			HMENU NotifyMenu = CreatePopupMenu();
			AppendMenu(NotifyMenu, MF_STRING, IDB_SCAN, L"Scan to build database");
			AppendMenu(NotifyMenu, MF_STRING, IDB_VIEW, L"View Statitistics");
			AppendMenu(NotifyMenu, MF_STRING, IDB_EXIT, L"Exit");
			POINT posMenu;
			GetCursorPos(&posMenu);
			TrackPopupMenu(NotifyMenu, TPM_RIGHTBUTTON, posMenu.x, posMenu.y, 0, hWnd, NULL);

		}
		}
	}
	break;
	case WM_CREATE:
	{
		//Tao hook Window+space
		hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, hInst, 0);
		LoadIcon(hWnd);
		///////
		InitCommonControls();
		//Create Font
		HFONT hFont;
		hFont = CreateFont(40, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		ToolBarText = CreateWindow(L"STATIC", L"Quick Launch", WS_CHILD | WS_VISIBLE, 90, 10, 225, 40, hWnd, NULL, hInst, NULL);
		SendMessage(ToolBarText, WM_SETFONT, WPARAM(hFont), true);
		hFont = CreateFont(16, 0, 0, 0,FW_NORMAL, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		TextSearch = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 60, 40 + 45, 300, 25, hWnd, (HMENU)400, hInst, NULL);
		SendMessage(TextSearch, WM_SETFONT, WPARAM(hFont), true);
		SendMessage(TextSearch, EM_SETCUEBANNER, TRUE, (LPARAM)L"Type the name of program...");
		ListBox = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, 60, 120, 300, 200, hWnd, (HMENU)401, 0, 0);
		SendMessage(ListBox, WM_SETFONT, WPARAM(hFont), true);
		HWND open = CreateWindow(L"STATIC", L"Open:", WS_CHILD | WS_VISIBLE, 20, 85, 40, 16, hWnd, NULL, hInst, NULL);
		SendMessage(open, WM_SETFONT, WPARAM(hFont), true);
		btn = CreateWindow(L"BUTTON", L"Run", WS_CHILD | WS_VISIBLE | WS_BORDER, 175, 350, 40, 20, hWnd,(HMENU)ID_BUTTON, hInst, NULL);
		SendMessage(btn, WM_SETFONT, WPARAM(hFont), true);
		//doc file
		ReadProgramFrequencyToFile(FILE_PATH);
		
		ScanFile(L"C:\\Program Files");
		//ScanFile(L"C:\\Program Files (x86)");
		UpdateFrequency();
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
            // Parse the menu selections:
			if ((HWND)lParam == TextSearch)
			{
				if (wmEvent == EN_CHANGE)
				{
					
					GetWindowText(TextSearch, keysearch, 255);
					SearchKeyApp(keysearch);
					LoadDataToListBox(VectorListbox);
				}
			}
            switch (wmId)
            {
			case ID_BUTTON:
			{
				bool check = false;
				GetWindowText(TextSearch, keysearch, 255);
				for (int i = 0; i < list.size(); i++)
				{
					if (keysearch == list[i].appname)
					{
						ShellExecute(0, 0, list[i].apppath.c_str(), 0, 0, 1);
						check = true;
						list[i].count++;
						break;
					}
				}
				if (check == false)
				{
					MessageBox(hWnd, L"Windows cannot find the app. Please try again.", L"Error", MB_OK | MB_ICONERROR);
				}
			}
				break;
			case IDB_SCAN:
			{
				
				for (int i = 0; i < VectorListbox.size(); i++)
				{
					ListBox_DeleteString(ListBox, 0);
				}
				VectorListbox.clear();
				ShowWindow(hWnd, 1);
				VectorListbox = list;
				LoadDataToListBox(list);

			}
				break;
			case IDB_VIEW:
			{
				//DialogBox(hInst, MAKEINTRESOURCE(IDD_CHART), hWnd, VeBieuDo);
			}
				break;
			case IDB_EXIT:
				DestroyWindow(hWnd);
				break;
			case 401://listbox
			{
				if (wmEvent == LBN_DBLCLK)
				{
					int pos = ListBox_GetCurSel(ListBox);
					if (pos == -1)
					{
						break;
					}
					ShellExecute(0, 0, VectorListbox[pos].apppath.c_str(), 0, 0, 1);
					for (int i = 0; i < list.size(); i++)
					{
						if (VectorListbox[pos].appname == list[i].appname)
						{
							list[i].count++;
							break;
						}
					}
				
				}
			}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
				
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		DWORD CtrlID = GetDlgCtrlID((HWND)lParam); //Window Control ID

												   //Set color for static control
												   //Set text and background
		if ((HWND)lParam == ToolBarText)
		{
			SetTextColor(hdcStatic, RGB(0,0,0));
			SetBkColor(hdcStatic, RGB(44,144,244));
		}
		else
		{
			SetTextColor(hdcStatic,RGB(0,0,0));
			SetBkColor(hdcStatic, RGB(255,255,255));
		}


		return (BOOL)GetSysColorBrush(COLOR_WINDOW);
	}
	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		WriteProgramFrequencyToFile(FILE_PATH);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if ((GetAsyncKeyState(VK_LWIN) < 0) && (GetAsyncKeyState(VK_SPACE) < 0))
	{
		if (IsWindowVisible(hWnd) == false)
		{
			ShowWindow(hWnd, 1);
		}
		else
		{
			ShowWindow(hWnd, 0);
		}
	}
	if (GetAsyncKeyState(VK_RETURN))
	{
		HWND hFocus = GetFocus();
		if (hFocus == ListBox)
		{
			int pos = ListBox_GetCurSel(ListBox);
			if (pos != -1)
			{
				ShellExecute(0, 0, VectorListbox[pos].apppath.c_str(), 0, 0, 1);
				for (int i = 0; i < list.size(); i++)
				{
					if (VectorListbox[pos].appname == list[i].appname)
					{
						list[i].count++;
						break;
					}
				}
			}
			if (hFocus == TextSearch)
			{
				bool check = false;
				GetWindowText(TextSearch, keysearch, 255);
				for (int i = 0; i < VectorListbox.size(); i++)
				{
					if (keysearch == VectorListbox[i].appname)
					{
						ShellExecute(0, 0, VectorListbox[i].apppath.c_str(), 0, 0, 1);
						check = true;
						break;
					}
				}
				if (check == false)
				{
					MessageBox(hWnd, L"Khong ton tai ten ung dung", L"Error", MB_OK | MB_ICONERROR);
				}
			}
		}
	}
	
	if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(VK_DOWN))
	{
		SetFocus(ListBox);
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

void ScanFile(WCHAR* link)
{
	WCHAR linkfull[255];
	wsprintf(linkfull, L"%s\\*.*", link);
	WIN32_FIND_DATA data;
	HANDLE h = FindFirstFile(linkfull, &data);
	if ((int)h == -1)
		return;
	do {
		if (data.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
		{
			if ((data.dwFileAttributes& FILE_ATTRIBUTE_HIDDEN) == false)
				if ((wcscmp(data.cFileName, L".")) && wcscmp(data.cFileName, L".."))
				{
					WCHAR linkcon[255];
					wsprintf(linkcon, L"%s\\%s", link, data.cFileName);
					ScanFile(linkcon);
				}
		}
		else
		{
			if (wcsstr(data.cFileName, L".exe") != NULL)
			{
				AppInfo appi;
				appi.appname = data.cFileName;
				appi.apppath = link + (wstring)(L"\\") + data.cFileName;
				list.push_back(appi);
			}
		}
	} while (FindNextFile(h, &data));
}

void SearchKeyApp(wstring keysearch)
{
	for (int i = 0; i < VectorListbox.size(); i++)
	{
		ListBox_DeleteString(ListBox, 0);
	}
	VectorListbox.clear();
	for (int i = 0; i < list.size(); i++)
	{
		if (list.at(i).appname.find(keysearch) != -1)
		{
			VectorListbox.push_back(list.at(i));
		}
	}
}

void LoadDataToListBox(vector <AppInfo> listdata)
{
	for (int i = 0; i < listdata.size(); i++)
	{
		ListBox_AddString(ListBox, listdata[i].appname.c_str());
	}
}
void ReadProgramFrequencyToFile(wstring filePath)
{
	wfstream f;
	f.open(filePath, ios::in);


	wstring buffer;
	if (f.is_open())
	{
		//Get items
		while (getline(f, buffer))
		{
			AppInfo temp;
			temp.appname = buffer;
			getline(f, buffer);
			temp.count = _wtoi64(buffer.c_str());
			temp.apppath = L"";
			usageApp.push_back(temp);
			
		}
	}

	//Close file
	f.close();
}
void WriteProgramFrequencyToFile(wstring filePath)
{
	wofstream f;
	f.open(filePath, ios::out);

	for (int i = 0; i < list.size(); i++)
	{
		if (list[i].count > 0)
		{
			f << list[i].appname << endl;
			f << list[i].count << endl;
		}
	}

	//Close file
	f.close();

}
void UpdateFrequency()
{
	for (int i = 0; i < usageApp.size(); i++)
	{
		for (int j = 0; j < list.size(); j++)
		{
			if (usageApp[i].appname == list[j].appname)
			{
				list[j].count = usageApp[i].count;
				break;
			}
		}
	}
}


void fillRectangle(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
	//Create rectangle
	RECT* rect = new RECT;
	rect->left = x1;
	rect->top = y1;
	rect->right = x2;
	rect->bottom = y2;

	//Create new brush
	HBRUSH hbrush = CreateSolidBrush(color);

	//Fill
	FillRect(hdc, rect, hbrush);
}