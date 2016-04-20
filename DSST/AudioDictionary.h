#ifndef _INC_STDIO
	#include <stdio.h>
#endif

#ifndef _INC_MALLOC
	#include <malloc.h>
#endif

#ifndef AUDIODICT_H
#define AUDIODICT_H
#include "Libs\\fmod\\fmod.hpp"
#include "Libs\\fmod\\fmod_errors.h"



class AudioDictionaryItem
{
public:
	int index;	// index value: 0,1,2...
	char fname[128];
	volatile int playing;
	AudioDictionaryItem(void);
	AudioDictionaryItem(int index,char* fname2);
	void Play();
	void Play(bool looped);
	void Pause(bool pause);
	void Stop();
	void Release();
	FMOD::Sound *sound;
	FMOD::Channel *channel;

	AudioDictionaryItem *next;
};




class AudioDictionary
{
public:
	FMOD::System *fmodsys;
	AudioDictionary();
	AudioDictionaryItem* cListRoot;

	void Play(int index);
	void Play(int index,bool looped);
	void Stop(int index);
	void Pause(int index,bool pause);
	void Clear();
	int RecordStart();
	int RecordStop();
	int GetLength();
	AudioDictionaryItem* GetAudioItem(int index);
	int Add(int index, char* fname);
};

	void WriteWavHeader(FILE *fp, FMOD_SOUND *sound, int length);
#endif