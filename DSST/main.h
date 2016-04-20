

#include <math.h>           // Header File For Mathematical Functions
#include "Libs/QPC/QPC.h"

#ifndef _INC_STDIO
	#include <stdio.h>
	#include <malloc.h>
#endif
#include "game.h"



#include "Libs/fmod/fmod.h"
#include "Log.h"
#include "AudioDictionary.h"
#include "com.h"
#include "Libs/rapidxml/rapidxml.hpp"


int LoadScript(char* filename);
int LoadScriptXML(char* filename);
void runMainLoop( int val );
void AddFrame(int FrameType,int texID,double timeout,char* str,int correctResponse,int audioID);
