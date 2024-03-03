#pragma once
VOID	 DataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount); 
BOOL     RecordData(HWND hWnd, DWORD dwMilliSeconds, char *fileName); 
int16_t* ExtractData(HWND hWnd, char* fileName, uint32_t* nABuffSize); 
INT		 CmpString(char* s1, char* s2); 