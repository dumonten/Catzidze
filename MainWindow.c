#include "stdafx.h"
#include "resource.h"

#include "dataMain.h"
#include "recordData.h"
#include "fft.h"

/*-----------------------window section*/
#define WS_MAINWINDOW ((WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~(WS_SIZEBOX | WS_MAXIMIZEBOX))
#define ButtonSize		 50

#define	hEditTitleBarID  1000 
#define hEditFoundSongID 1001
#define hButtonStartID	 1002
#define hProgressBarID	 1003

/*interaction with the main window*/
HWND	 hwndMain;
HWND     hwndTextWindow; 
HDC      hdcBack;
HBITMAP  hbmBack;
RECT     rcClient;

INT	     HEIGHT   = 200,
		 WIDTH    = 600, 
		 chHEIGHT = 600,
		 chWIDTH  = 600;

INT		 MOUSEACTIVE = 0, xOption, yOption;


COLORREF clt_TITLE_BAR = RGB(246, 241, 249);

HBRUSH   hbr_TITLE_BAR_COLOR = CreateSolidBrush(clt_TITLE_BAR);

RECT     pRctTitleBarButtons[3],
		 rctTitleBar,
		 rctEditTitleBar,
		 rctButtonStart,
		 rctEditFoundSong,
		 rctProgressBar;

HWND	 hwndEditTitleBar,
		 hwndEditFoundSong,
		 hwndButtonStartMatch,
		 hwndProgressBar;

char     matchFileName[MaxStringSize] = { '\0' }, 
		 matchedSongName[MaxStringSize] = { '\0' },
		 fileName[MaxStringSize] = { '\0' };

BOOL     isStoredSong =  FALSE; 
/*-----------------------child text window section*/
#define	 hEditChildText  1000 
RECT	 rctEditChildText;
HWND 	 hwndEditChildText; 
/*-----------------------program section*/
int16_t	 *buff;
uint32_t nBuffSize = 0;

void FillEditWithMatchInf(HWND hwnd) {
	FILE *f = fopen("BestMatches.txt", "r"); 
	int fileSize = GetFileSize(f);
	if (fileSize == 0)
		return;
	char *s = NULL, tempChar, *tempStr;
	int i = 0;
	while (!feof(f)) {
		fscanf_s(f, "%c", &tempChar);
		if (tempChar != '\n') {
			tempStr = (char *)realloc(s, sizeof(char)*(i + 1));
			s = tempStr;
			s[i] = tempChar;
			i++;
		}
		else {
			tempStr = (char *)realloc(s, sizeof(char)*(i + 2));
			s = tempStr;
			s[i] = '\r'; s[i + 1] = '\n';
			i += 2;
		}
	}
	tempStr = (char *)realloc(s, sizeof(char)*(i + 1));
	s = tempStr;
	s[i] = '\0';
	SetWindowText(hwnd, s);
	fclose(f);
}

void FillEditWithHistoryInf(HWND hwnd) {
	FILE *f = fopen("History.txt", "r");
	int fileSize = GetFileSize(f);
	if (fileSize == 0)
		return; 
	char *s = NULL, tempChar, *tempStr;
	int i = 0;
	while (!feof(f)) {
		fscanf_s(f, "%c", &tempChar);
		if (tempChar != '\n') {
			tempStr = (char *)realloc(s, sizeof(char)*(i + 1));
			s = tempStr;
			s[i] = tempChar;
			i++; 
		}
		else { 
			tempStr = (char *)realloc(s, sizeof(char)*(i + 2));
			s = tempStr;
			s[i] = '\r'; s[i + 1] = '\n'; 
			i += 2; 
		}
	}
	tempStr = (char *)realloc(s, sizeof(char)*(i + 1));
	s = tempStr;
	s[i] = '\0'; 
	SetWindowText(hwnd, s);
	fclose(f);
}

void SaveHistory(char *str) {
	FILE *f = fopen("History.txt", "a");
	SYSTEMTIME st; 
	GetLocalTime(&st);
	fprintf(f, "%02d.%02d.%04d %02d:%02d Found Song - %s\n", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, str); 
	fclose(f);
}

char *CheckOrReformatFilePath(char *fileName) {
	char *result, buff[5] = { '\0' }, pattern[5] = { '.', 'w', 'a', 'v', '\0' };

	memmove(buff, &fileName[strlen(fileName) - 4], 4);
	if (strcmp(buff, pattern) == 0)
		return fileName;
	result = (char *)malloc(sizeof(char) * (strlen(fileName) + 4));
	strcpy(result, fileName);
	strcat(result, pattern);
	return result;
}

BOOL isStored(char *fileName) {
	char buff[3] = { '\0' }, pattern[3] = { ':', '\\', '\0' };
	memmove(buff, &fileName[1], 2);
	if (strcmp(buff, pattern) == 0)
		return TRUE;
	return FALSE;
}

int  WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;
		COMDLG_FILTERSPEC ComDlgFS[1] = { { L"Wav Files Only", L"*.wav;" } };
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			pFileOpen->SetFileTypes(1, ComDlgFS);
			hr = pFileOpen->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						wsprintfA(matchFileName, "%S", pszFilePath);
						//MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return 0;
}

void InitializeBackBuffer(HWND hWnd, int w, int h)
{
	HDC hdcWindow = GetDC(hWnd);

	strcpy(fileName, "background.bmp");

	hdcBack = CreateCompatibleDC(hdcWindow);
	hbmBack = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	SaveDC(hdcBack);
	SelectObject(hdcBack, hbmBack);
	ReleaseDC(hWnd, hdcWindow);
}

void FinalizeBackBuffer()
{
	if (hdcBack)
	{
		RestoreDC(hdcBack, -1);
		DeleteObject(hbmBack);
		DeleteDC(hdcBack);
		hdcBack = 0;
	}
}

void Calculate() {
	SetRect(&rctTitleBar, 0, 0, WIDTH, 65);
	SetRect(&rctEditTitleBar, 120, 20, 530, 45);
	SetRect(&rctEditChildText, 0, 0, chWIDTH, chHEIGHT);
	SetRect(&rctEditFoundSong, 10, 75, 430, 100);
	SetRect(&rctButtonStart, 500, 75, 575, 100);
	SetRect(&rctProgressBar, 10, 120, WIDTH - 25, 140);

	SetRect(&pRctTitleBarButtons[0], 5, 5, ButtonSize + 5, ButtonSize + 5);
	SetRect(&pRctTitleBarButtons[1], pRctTitleBarButtons[0].right + 5, 5, pRctTitleBarButtons[0].right + 5 + ButtonSize, ButtonSize + 5);
	SetRect(&pRctTitleBarButtons[2], WIDTH - 20 - ButtonSize, 5, WIDTH - 20, ButtonSize + 5);
}

void Draw(HDC hdc) {
	HBITMAP hbmBtn;
	HDC hdcTemp = CreateCompatibleDC(hdc);

	SaveDC(hdc);
	SelectObject(hdc, CreatePen(PS_SOLID, 0, clt_TITLE_BAR));
	SelectObject(hdc, hbr_TITLE_BAR_COLOR);
	Rectangle(hdc, rctTitleBar.left, rctTitleBar.top, rctTitleBar.right, rctTitleBar.bottom);

	strcpy(fileName, "history.bmp");
	hbmBtn = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	SelectObject(hdcTemp, hbmBtn);
	BitBlt(hdc, pRctTitleBarButtons[0].left, pRctTitleBarButtons[0].top, ButtonSize, ButtonSize, hdcTemp, 0, 0, SRCCOPY);

	strcpy(fileName, "match.bmp");
	hbmBtn = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	SelectObject(hdcTemp, hbmBtn);
	BitBlt(hdc, pRctTitleBarButtons[1].left, pRctTitleBarButtons[1].top, ButtonSize, ButtonSize, hdcTemp, 0, 0, SRCCOPY);

	strcpy(fileName, "open.bmp");
	hbmBtn = (HBITMAP)LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	SelectObject(hdcTemp, hbmBtn);
	BitBlt(hdc, pRctTitleBarButtons[2].left, pRctTitleBarButtons[2].top, ButtonSize, ButtonSize, hdcTemp, 0, 0, SRCCOPY);

	DeleteObject(hbmBtn);
	DeleteDC(hdcTemp);
	RestoreDC(hdc, -1);
}

int  GetTitleBarButtonIndex() {
	if (xOption <= pRctTitleBarButtons[0].right &&  xOption >= pRctTitleBarButtons[0].left
		&& yOption <= pRctTitleBarButtons[0].bottom && yOption >= pRctTitleBarButtons[0].top)
		return 0;
	else if (xOption <= pRctTitleBarButtons[1].right &&  xOption >= pRctTitleBarButtons[1].left
		&& yOption <= pRctTitleBarButtons[1].bottom && yOption >= pRctTitleBarButtons[1].top)
		return 1;
	else if (xOption <= pRctTitleBarButtons[2].right &&  xOption >= pRctTitleBarButtons[2].left
		&& yOption <= pRctTitleBarButtons[2].bottom && yOption >= pRctTitleBarButtons[2].top)
		return 2;
	return -1;
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	INT xMouse, yMouse;
	switch (uMsg)
	{
	case WM_CREATE:
		hwndEditTitleBar = CreateWindowEx(WS_EX_CLIENTEDGE, (LPSTR)"EDIT", (LPSTR)"Enter the name of the song.", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_RIGHT,
			rctEditTitleBar.left, rctEditTitleBar.top, rctEditTitleBar.right - rctEditTitleBar.left, rctEditTitleBar.bottom - rctEditTitleBar.top, hWnd, (HMENU)hEditTitleBarID, 0, NULL);

		hwndEditFoundSong = CreateWindowEx(WS_EX_CLIENTEDGE, (LPSTR)"EDIT", (LPSTR)«», WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_RIGHT | ES_READONLY | ES_CENTER,
			rctEditFoundSong.left, rctEditFoundSong.top, rctEditFoundSong.right - rctEditFoundSong.left, rctEditFoundSong.bottom - rctEditFoundSong.top, hWnd, (HMENU)hEditFoundSongID, 0, NULL);

		hwndButtonStartMatch = CreateWindowEx(WS_EX_CLIENTEDGE, (LPSTR)"BUTTON", (LPSTR)"Start", WS_CHILD | WS_VISIBLE,
			rctButtonStart.left, rctButtonStart.top, rctButtonStart.right - rctButtonStart.left, rctButtonStart.bottom - rctButtonStart.top, hWnd, (HMENU)hButtonStartID, 0, NULL);

		hwndProgressBar = CreateWindowEx(WS_EX_CLIENTEDGE, PROGRESS_CLASS, (LPSTR)NULL, WS_CHILD | WS_VISIBLE,
			rctProgressBar.left, rctProgressBar.top, rctProgressBar.right - rctProgressBar.left, rctProgressBar.bottom - rctProgressBar.top, hWnd, (HMENU)hProgressBarID, 0, NULL);
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &rcClient);
		FinalizeBackBuffer();
		InitializeBackBuffer(hWnd, rcClient.right, rcClient.bottom);
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcBack, 0, 0, SRCCOPY);
		Draw(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_SYSCOMMAND:
		if (SC_KEYMENU == (wParam & 0xFFF0))
			return 0;
		break;
	case WM_DESTROY:
		FinalizeBackBuffer();
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		if (!MOUSEACTIVE) {
			MOUSEACTIVE = 1;
			xMouse = GET_X_LPARAM(lParam);
			yMouse = GET_Y_LPARAM(lParam);
			SetCapture(hWnd);
			yOption = yMouse;
			xOption = xMouse;
		}
		break;
	case WM_LBUTTONUP: {
		ReleaseCapture();
		MOUSEACTIVE = 0;
		int num = GetTitleBarButtonIndex();
		if (num == -1) break;
		if (num == 0) {
			hwndTextWindow = CreateWindowEx(0, (LPSTR)"History", (LPCSTR)"History", WS_MAINWINDOW & ~WS_VISIBLE,
				(GetSystemMetrics(SM_CXSCREEN) - chWIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - chHEIGHT) / 2, chWIDTH, chHEIGHT, 0, 0, 0, NULL);

			ShowWindow(hwndTextWindow, SW_SHOW);
			SetActiveWindow(hwndTextWindow);
			EnableWindow(hWnd, FALSE);
		}
		if (num == 1) {
			hwndTextWindow = CreateWindowEx(0, (LPSTR)"Matches", (LPCSTR)"Matches", WS_MAINWINDOW & ~WS_VISIBLE,
				(GetSystemMetrics(SM_CXSCREEN) - chWIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - chHEIGHT) / 2, chWIDTH, chHEIGHT, 0, 0, 0, NULL);

			ShowWindow(hwndTextWindow, SW_SHOW);
			SetActiveWindow(hwndTextWindow);
			EnableWindow(hWnd, FALSE); 
		}
		if (num == 2) {
			wWinMain(NULL, NULL, NULL, NULL);
			SetWindowText(hwndEditTitleBar, matchFileName);
		}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case hButtonStartID:
			DWORD startTime, endTime = 0;
			SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendMessage(hwndProgressBar, PBM_SETPOS, 0, 0);
			SendMessage(hwndProgressBar, PBM_SETSTEP, (WPARAM)10, 0);
			
			GetWindowTextA(hwndEditTitleBar, matchFileName, MaxStringSize);
			strcpy(matchFileName, CheckOrReformatFilePath(matchFileName));
			isStoredSong = isStored(matchFileName); 
			
			if (!isStoredSong) {
				if (!RecordData(hWnd, 10000, matchFileName)) {
					break; 
				};
			}
			
			buff = ExtractData(hWnd, matchFileName, &nBuffSize);
			
			if (!isStoredSong) remove(matchFileName);
			
			if (!isStoredSong) 
				SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10*nBuffSize/nFrameSize));
			if (isStoredSong)
				SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));

			SendMessage(hwndProgressBar, PBM_SETPOS, 0, 0);
			SendMessage(hwndProgressBar, PBM_SETSTEP, (WPARAM)10, 0);
			
			Song* song = DFT(0, buff, nBuffSize, TRUE, nSongs);
			
			strcpy(matchedSongName, GetBestMatchSong(0, nSongs - 1)); 
			SetWindowText(hwndEditFoundSong, matchedSongName);
			SaveHistory(matchedSongName);
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK HistoryWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE:
		hwndEditChildText = CreateWindowEx(WS_EX_CLIENTEDGE, (LPSTR)"EDIT", (LPSTR)«», WS_CHILD | ES_READONLY | ES_MULTILINE | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL,
			rctEditChildText.left, rctEditChildText.top, rctEditChildText.right - 15, rctEditChildText.bottom - 40, hWnd, 0, 0, 0);
		FillEditWithHistoryInf(hwndEditChildText);
		break;
	case WM_DESTROY:
		ShowWindow(hWnd, SW_HIDE);
		SetActiveWindow(hwndMain);
		EnableWindow(hwndMain, TRUE);
		InvalidateRect(hwndMain, NULL, TRUE);
		break; 

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MatchesWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_CREATE: 
		hwndEditChildText = CreateWindowEx(WS_EX_CLIENTEDGE, (LPSTR)"EDIT", (LPSTR)«», WS_CHILD | ES_READONLY | ES_MULTILINE | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL,
			rctEditChildText.left, rctEditChildText.top , rctEditChildText.right - 15, rctEditChildText.bottom - 40, hWnd, 0, 0, 0);
		FillEditWithMatchInf(hwndEditChildText);
		break;
	case WM_DESTROY:
		ShowWindow(hWnd, SW_HIDE);
		SetActiveWindow(hwndMain);
		EnableWindow(hwndMain, TRUE);
		InvalidateRect(hwndMain, NULL, TRUE);
		break;

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID WcexRegister(UINT style, HINSTANCE hInstance, WNDPROC wndProc, HCURSOR hCursor, HBRUSH hbrBackground, LPCSTR className) {
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = style;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wcex.lpfnWndProc = wndProc;
	wcex.hCursor = hCursor;
	wcex.hbrBackground = hbrBackground;
	wcex.lpszClassName = className;
	RegisterClassEx(&wcex);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WcexRegister(CS_GLOBALCLASS, hInstance, MainWindowProc, LoadCursor(0, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), (LPSTR) "Catzidze");
	WcexRegister(CS_GLOBALCLASS, hInstance, HistoryWindowProc, LoadCursor(0, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), (LPSTR) "History");
	WcexRegister(CS_GLOBALCLASS, hInstance, MatchesWindowProc, LoadCursor(0, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), (LPSTR) "Matches");

	MSG msg;
	AdjustWindowRect(&rcClient, WS_MAINWINDOW, FALSE);
	
	Calculate();

	hwndMain = CreateWindowEx(0, (LPSTR)"Catzidze", (LPSTR) "Catzidze", WS_MAINWINDOW,
		(GetSystemMetrics(SM_CXSCREEN) - WIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2,
		WIDTH, HEIGHT, 0, 0, 0, NULL);

	InitFFT(hwndMain);
	fclose(fopen("BestMatches.txt", "w")); 

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}