#pragma once

#define nFrameSize 4096
extern long nSongs;
typedef std::unordered_map<int, int> offsetMapType;
typedef std::unordered_map<int64_t, list> hashMapType;
typedef std::unordered_map<int, char *> songNameMapType;
typedef std::unordered_map<long, offsetMapType> matchMapType;

/*SONG STRUCTURE ---------------------------------------*/
typedef struct Song {
	int songID; 
	hashMapType	 hashMap;   
} Song;

typedef struct songMInf {
	char *songName;
	int	nMatch; 
} songMInf;

int GetFileSize(FILE* file); 
void InitGenHashMap(HWND hWnd); 
void SaveGenHashMap(Song* song); 
void InitSongsNameMap(); 
void InitFFT(HWND hWnd);

int GetRateIndex(int rate); 
int64_t GetHash(int64_t p1, int64_t p2, int64_t p3, int64_t p4); 
Song* GetSong(_Dcomplex** audioBuffer, const uint32_t nABuffSize, BOOL isRecordFile, long songID); 

char* GetBestMatchSong(HWND hWnd, int SongID); 
void FFT(_Dcomplex* x, uint32_t N); 
Song* DFT(HWND hWnd, int16_t* audioBuffer, uint32_t nABuffSize, BOOL isRecordFile, long SongID); 

_Dcomplex _Caddcc(_Dcomplex a, _Dcomplex b); 
_Dcomplex _Csubcc(_Dcomplex a, _Dcomplex b);