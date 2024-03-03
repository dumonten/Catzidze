#include "stdafx.h"
#define  MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "dataMain.h"
#include "recordData.h"
#pragma warning(disable : 4996) //allow using fopen


VOID	 DataCallback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
	/*
	* pOutput - output buffer
	* pInput - input buffer from which data is read
	* frameCount - how many frames can be written to the output buffer and read from the input buffer
	*/
	ma_encoder* pEncoder = (ma_encoder*)pDevice->pUserData;
	MA_ASSERT(pEncoder != NULL);
	ma_encoder_write_pcm_frames(pEncoder, pInput, frameCount, NULL);
}

BOOL     RecordData(HWND hWnd, DWORD dwMilliSeconds, char *fileName)
{
	ma_result  rs;
	ma_encoder encdr;
	ma_device  dev;
	ma_encoder_config cnfgEncdr;
	ma_device_config  cnfgDev;
	
	DWORD startTime, endTime = 0; 

	cnfgEncdr = ma_encoder_config_init(ma_encoding_format_wav, ma_format_s16, 2, 44100);

	if (ma_encoder_init_file(fileName, &cnfgEncdr, &encdr) != MA_SUCCESS)
	{
		MessageBox(hWnd, (LPSTR)"Failed to initialize output file.", (LPSTR)"Error", MB_ICONERROR);
		return FALSE;
	}
	cnfgDev = ma_device_config_init(ma_device_type_capture);
	cnfgDev.capture.format = encdr.config.format;
	cnfgDev.capture.channels = encdr.config.channels;
	cnfgDev.sampleRate = encdr.config.sampleRate;
	cnfgDev.dataCallback = DataCallback;
	cnfgDev.pUserData = &encdr;
	rs = ma_device_init(NULL, &cnfgDev, &dev);
	if (rs != MA_SUCCESS) 
	{
		MessageBox(hWnd, (LPSTR)"Failed to initialize capture device.", (LPSTR)"Error", MB_ICONERROR);
		return FALSE;
	}
	rs = ma_device_start(&dev);
	if (rs != MA_SUCCESS) 
	{
		MessageBox(hWnd, (LPSTR)"Failed to start device.", (LPSTR)"Error", MB_ICONERROR);
		return FALSE;
	}

	startTime = GetTickCount(); 
	for (int i = 1; i <= 10; i++) {
		endTime = GetTickCount();
		while (endTime - startTime < i * 1000) {
			endTime = GetTickCount(); 
		}
		SendMessage(hwndProgressBar, PBM_STEPIT, 0, 0);
	}
	ma_device_uninit(&dev); 
	ma_encoder_uninit(&encdr);
	return TRUE;
}

int16_t* ExtractData(HWND hWnd, char *fileName, uint32_t *nABuffSize)
{
	FILE *AudioFile;
	char  currValue[5] = { '\0' }, chRead;

	INT formatType = 0, numChannels = 0, 
		sampleRate = 0, bytesPerSecond = 0, 
		blockAlign = 0, bitsPerSecond = 0; 
	int16_t nValue; 

	if (!(AudioFile = fopen(fileName, "rb")))
	{
		MessageBox(hWnd, (LPSTR)"Failed to open file.", (LPSTR)"Error", MB_ICONERROR);
		return NULL; 
	}

	fread(currValue, 1, 4, AudioFile);
	if (CmpString(currValue, "RIFF")) { return FALSE; }

	fseek(AudioFile, 4, SEEK_CUR);

	fread(currValue, 1, 4, AudioFile);
	if (CmpString(currValue, "WAVE")) { return FALSE; }

	fread(currValue, 1, 4, AudioFile);
	if (CmpString(currValue, "fmt ")) { return FALSE; }

	fseek(AudioFile, 4, SEEK_CUR);  

	fread(&formatType,     2, 1, AudioFile);	//should be 1 
	fread(&numChannels,	   2, 1, AudioFile);	//should be 2
	fread(&sampleRate,     4, 1, AudioFile);	//should be 44100
	fread(&bytesPerSecond, 4, 1, AudioFile);	//should be 176400
	fread(&blockAlign,     2, 1, AudioFile);	//should be 4 
	fread(&bitsPerSecond,  2, 1, AudioFile);	//should be 16
	
	if (CmpString(currValue, "data")) { 
		while ((fread(&chRead, 1, 1, AudioFile)))
		{
			memmove(&currValue[0], &currValue[1], 3);
			currValue[3] = chRead; 
			if (!CmpString(currValue, "data")) 
			{
				break; 
			}
		}
		if (feof(AudioFile)) 
		{
			return NULL; 
		}
	}
	int16_t* audioBuffer;
	fread(nABuffSize, 4, 1, AudioFile);
	*nABuffSize /= 2; 
	audioBuffer = (int16_t *)malloc((*nABuffSize) * sizeof(int16_t));
	INT readData = fread(audioBuffer, 2, (*nABuffSize), AudioFile);
	if (readData != (*nABuffSize))
	{
		MessageBox(hWnd, (LPSTR)"Corrupted data in file.", (LPSTR)"Error", MB_ICONERROR);
		return NULL;
	}
	fclose(AudioFile);
	return audioBuffer;
}

INT      CmpString(char *s1, char *s2)
{
	int bound; 
	if (strlen(s1) > strlen(s2)) 
	{
		bound = strlen(s1); 
	} else
	{
		bound = strlen(s2); 
	}
	for (int i = 0; i < bound; i++)
	{
		if (s1[i] > s2[i]) return 1;
		if (s1[i] < s2[i]) return -1;
	}
	if (bound == strlen(s1) && bound == strlen(s2)) return 0;
	else if (bound == strlen(s1)) return 1;
	else return -1; 
}