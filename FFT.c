#include "stdafx.h"
#include "dataMain.h"
#include "fft.h"
#include "recordData.h"
#pragma warning(disable : 4996) //allow using fopen
#define nRateRangeSize 5
#define nHashCoef 2 
#define nULimit 300
#define nDLimit 40 
 
long nSongs = 0;
songNameMapType songNameMap; 
hashMapType genHashMap;
matchMapType matchMap; 

const char* FileNameSongLib	= "libSongPath.txt"; 
const char* FileNameGenHashMap = "GenHashMap.bin";
const char* FileSongsNameAndAuthorLib = "SongNames.txt";
const uint32_t rateRange[nRateRangeSize] = { nDLimit, 80, 120, 180, nULimit };

void InitFFT(HWND hWnd) {
	InitGenHashMap(hWnd);
	InitSongsNameMap(); 
}

int GetFileSize(FILE* file) {
	int _file_size = 0;
	if (file) {
		fseek(file, 0, SEEK_END);
		_file_size = ftell(file);
		fseek(file, 0, SEEK_SET);
	}
	return _file_size;
}

void InitGenHashMap(HWND hWnd) {
	if (genHashMap.empty()) {
		nSongs = 0;
		FILE* FileGenHashMap, * FileSongLib;  
		if ((FileGenHashMap = fopen(FileNameGenHashMap, "rb")) && GetFileSize(FileGenHashMap)) {
			while (!feof(FileGenHashMap)) {
				Song* song = new Song; 
				fread(&(song->songID), sizeof(int), 1, FileGenHashMap);
				int hashMapSize; 
				fread(&(hashMapSize), sizeof(int), 1, FileGenHashMap); 
				for (int i = 0; i < hashMapSize; i++) {
					int64_t hash; int llSize;  list lPoints = llCreate();
					fread(&(hash), sizeof(int64_t), 1, FileGenHashMap);
					fread(&(llSize), sizeof(int), 1, FileGenHashMap); 
					for (int j = 0; j < llSize; j++) {
						DataTimeSeg data;
						fread(&(data), sizeof(DataTimeSeg), 1, FileGenHashMap); 
						llAdd(lPoints, data); 
					}
					song->hashMap[hash] = lPoints; 
				}
				for (std::pair<int64_t, list> it : song->hashMap) {
					list lPoints = genHashMap[it.first];
					if (lPoints) {
						node* temp = it.second->head;
						while (temp) {
							llAdd(lPoints, temp->value);
							temp = temp->next;
						}
					} else {
						lPoints = llCreate();
						node* temp = it.second->head;
						while (temp) {
							llAdd(lPoints, temp->value);
							temp = temp->next;
						}
						genHashMap[it.first] = lPoints;
					}
				}
				nSongs++;
			}
			fclose(FileGenHashMap);
		}
		else if ((FileSongLib = fopen(FileNameSongLib, "r")) && GetFileSize(FileSongLib)) {
			char strPath[100] = { '\0' };
			char chRead;
			while (!feof(FileSongLib)) {
				fscanf_s(FileSongLib, "%c", &chRead); 
				if (chRead == '\n' || feof(FileSongLib)) {
					uint32_t nABuffSize;
					int16_t* audioBuffer = ExtractData(hWnd, strPath, &nABuffSize);
					if (audioBuffer) {
						Song* song = DFT(hWnd, audioBuffer, nABuffSize, FALSE, nSongs++);
						free(audioBuffer);
						SaveGenHashMap(song);
					}
					MessageBox(hWnd, (LPSTR)"Song is ready.", (LPSTR)"Attention!", MB_OK);
					memset(strPath, '\0', 100);
				} else {
					strPath[strlen(strPath)] = chRead;
				}
			}
			fclose(FileSongLib);
		}
	}
}

void SaveGenHashMap(Song *song) {
	/*
	------------------------------
	songID				 | 4
	num of elem in map   | 4 
	hash			     | 8
	num of elem in list  | 4
	dataTimeSeg items    | 8 (sizeof(DataTimeSeg))
	------------------------------
	*/
	FILE* FileGenHashMap;
	int size, position;
	if ((FileGenHashMap = fopen(FileNameGenHashMap, "ab"))) {
		fwrite(&(song->songID), sizeof(int), 1, FileGenHashMap);
		int size = song->hashMap.size(); 
		fwrite(&(size), sizeof(int), 1, FileGenHashMap);
		for (std::pair<int64_t, list> it : song->hashMap) {
			fwrite(&(it.first), sizeof(int64_t), 1, FileGenHashMap); 
			fwrite(&(it.second->size), sizeof(uint32_t), 1, FileGenHashMap);
			node* temp = it.second->head;
			while (temp) {
				fwrite(&(temp->value), sizeof(DataTimeSeg), 1, FileGenHashMap);
				temp = temp->next;
			}
		}
		fclose(FileGenHashMap);
	}
}

void InitSongsNameMap() {
	FILE *File; 
	int i = 0; 
	if ((File = fopen(FileSongsNameAndAuthorLib, "r")) && GetFileSize(File)) {
		char songName[MaxStringSize] = { '\0' };
		char chRead;
		while (!feof(File)) {
			fscanf_s(File, "%c", &chRead);
			if (chRead == '\n' || feof(File)) {
				songNameMap[i] = (char *)malloc(MaxStringSize * sizeof(char)); 
				strcpy(songNameMap[i], songName); 
				i++; 
				memset(songName, '\0', 100);
			}
			else {
				songName[strlen(songName)] = chRead;
			}
		}
		fclose(File);
	}
}


int GetRateIndex(int rate) {
	int i = 0;
	while (rate > rateRange[i]) {
		i++;
	}
	if (i >= nRateRangeSize) {
		i = -1; 
	}
	return i;
}

int64_t GetHash(int64_t p1, int64_t p2, int64_t p3, int64_t p4) {
	return (p4 - (p4 % nHashCoef)) * 100000000 + 
		   (p3 - (p3 % nHashCoef)) * 100000 + 
		   (p2 - (p2 % nHashCoef)) * 100 + 
		   (p1 - (p1 % nHashCoef));
}

Song* GetSong(_Dcomplex** audioBuffer, const uint32_t nABuffSize, BOOL isRecordFile, long songID) { 
	matchMap.clear(); 
	Song* resultSong = new Song;
	resultSong->songID = songID;

	double** recordPoints = (double**)malloc(sizeof(double*) * nABuffSize);
	double** highscores   = (double**)malloc(sizeof(double*) * nABuffSize);
	long**	 points		  = (long**  )malloc(sizeof(long*  ) * nABuffSize);
	for (int i = 0; i < nABuffSize; i++) {
		recordPoints[i] = (double*)malloc(sizeof(double) * nULimit);
		highscores[i]   = (double*)malloc(sizeof(double) * nRateRangeSize);
		points[i]       = (long  *)malloc(sizeof(long)   * nRateRangeSize);
		for (int j = 0; j < nRateRangeSize || j < nULimit; j++) {
			if (j < nULimit) {
				recordPoints[i][j] = (DOUBLE)0;
			}
			if (j < nRateRangeSize) {
				highscores[i][j] = (DOUBLE)0;
				points[i][j] = (LONG)0;
			}
		}
	}

	for (int i = 0; i < nABuffSize; i++) {
		for (int j = nDLimit; j < nULimit; j++) {
			double magnitude = log(cabs(audioBuffer[i][j]) + 1);
			int index = GetRateIndex(j);
			if (magnitude > highscores[i][index]) {
				highscores[i][index] = magnitude;
				recordPoints[i][j] = 1;
				points[i][index] = j;
			}
		}
		int64_t hash = GetHash(points[i][0], points[i][1], points[i][2], points[i][3]);
		list lPoints = genHashMap[hash];
		if (isRecordFile) {
			if (lPoints) {
				node* temp = lPoints->head;
				while (temp) {
					int timeOfPoint = temp->value.timeInterval; 
					long songIFWI = temp->value.songID;
					int offset = abs(timeOfPoint - i);
					offsetMapType tempMap = matchMap[temp->value.songID];
					if (tempMap.empty()) {
						tempMap[offset] = 1;
					}
					else {
						tempMap[offset]++; 
					}
					matchMap[temp->value.songID] = tempMap;
					temp = temp->next;
				}

			}
		}
		else {
			DataTimeSeg point;
			point.songID = songID; 
			point.timeInterval = i; 
			if (lPoints) { 
				llAdd(lPoints, point); 
			}
			else {
				lPoints = llCreate(); 
				llAdd(lPoints, point);
				genHashMap[hash] = lPoints;
				(resultSong->hashMap)[hash] = lPoints;
			}
		}
	}
	for (int i = 0; i < nABuffSize; i++) 
	{
		free(recordPoints[i]); 
		free(highscores[i]);
		free(points[i]); 
	}
	return resultSong; 
}

char* GetBestMatchSong(HWND hWnd, int SongID) {
	FILE *f = fopen("BestMatches.txt", "w");
	songMInf *arr = (songMInf *)malloc(sizeof(songMInf) * SongID);
	int bestCount = 0, bestSong = -1;
	for (int i = 0; i < SongID; i++) {
		offsetMapType tempMap = matchMap[i];
		int bestCountForSong = 0;
		for (std::pair<int, int> it : tempMap) {
			if (it.second > bestCountForSong) {
				bestCountForSong = it.second;
			}
		} 
		arr[i] = { songNameMap[i], bestCountForSong };
		if (bestCountForSong > bestCount) {
			bestCount = bestCountForSong;
			bestSong = i;
		}
	}
	for (int i = 0; i < SongID - 1; i++) {
		for (int j = i + 1; j < SongID; j++) {
			if (arr[i].nMatch < arr[j].nMatch) {
				songMInf temp = arr[i]; 
				arr[i] = arr[j]; 
				arr[j] = temp; 
			}
		}
	}
	for (int i = 0; i < SongID; i++) {
		fprintf(f, "%d. %s - %d\n", i+1, arr[i].songName, arr[i].nMatch); 
	}
	char result[MaxStringSize] = {'\0'};
	strcpy(result, songNameMap[bestSong]);
	free(arr); 
	fclose(f); 
	return result; 
}

void FFT(_Dcomplex* x, uint32_t N) {
	if (N <= 1) {
		return;
	}
	_Dcomplex* even = (_Dcomplex*)malloc(N / 2 * sizeof(_Dcomplex));
	_Dcomplex* odd  = (_Dcomplex*)malloc(N / 2 * sizeof(_Dcomplex));
	for (int i = 0; i < N / 2; i++) {
		even[i] = x[2 * i];
		odd[i] = x[2 * i + 1];
	}
	FFT(even, N / 2);
	FFT(odd,  N / 2);
	for (int i = 0; i < N / 2; i++) {
		double argW = -2 * i * M_PI / N;
		_Dcomplex w = _Cbuild(cos(argW), sin(argW));
		x[i] = _Caddcc(even[i], _Cmulcc(w, odd[i]));
		x[N / 2 + i] = _Csubcc(even[i], _Cmulcc(w, odd[i]));
	}
	free(even);
	free(odd); 
}

Song* DFT(HWND hWnd, int16_t* audioBuffer, uint32_t nABuffSize, BOOL isRecordFile, long SongID) {
	uint32_t nFrame = nABuffSize / nFrameSize;
	if (nFrame > 1000) {
		nFrame = 1000;
	}
	_Dcomplex** result = (_Dcomplex**)malloc(nFrame * sizeof(_Dcomplex*));
	for (uint32_t i = 0; i < nFrame; i++) {
		_Dcomplex* row = (_Dcomplex*)malloc(nFrameSize * sizeof(_Dcomplex));
		for (uint32_t j = 0; j < nFrameSize; j++) {
			double real = (double)audioBuffer[i * nFrameSize + j];
			row[j] = _Cbuild(real, 0);
		}
		FFT(row, nFrameSize);
		result[i] = row; 
		SendMessage(hwndProgressBar, PBM_STEPIT, 0, 0);
	}
	Song *song = GetSong(result, nFrame, isRecordFile, SongID);
	for (int i = 0; i < nFrame; i++) {
		free(result[i]);
	}
	free(result); 
	return song; 
}


_Dcomplex _Caddcc(_Dcomplex a, _Dcomplex b)
{
	double real = creal(a) + creal(b);
	double imag = cimag(a) + cimag(b);
	_Dcomplex result = _Cbuild(real, imag);
	return result;
}

_Dcomplex _Csubcc(_Dcomplex a, _Dcomplex b)
{
	double real = creal(a) - creal(b);
	double imag = cimag(a) - cimag(b);
	_Dcomplex result = _Cbuild(real, imag);
	return result;
}