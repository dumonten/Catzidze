#pragma once
#define MaxStringSize    1000
extern HWND hwndProgressBar;
char *CheckOrReformatFilePath(char *fileName); 
BOOL isStored(char *fileName); 
int  WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow); 
void InitializeBackBuffer(HWND hWnd, int w, int h); 
void FinalizeBackBuffer(); 
void Calculate(); 
void Draw(HDC hdc); 
int  GetTitleBarButtonIndex(); 
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); 
VOID WcexRegister(UINT style, WNDPROC wndProc, HCURSOR hCursor, HBRUSH hbrBackground, LPCSTR className); 
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow); 