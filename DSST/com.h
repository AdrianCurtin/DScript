#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

#ifndef COM_H
	#pragma once

	#include "stdio.h"

	int ComConnect(int iPort,DWORD baud,bool parity,int stopBits);
	void ComError(LPCTSTR text);
	int  ComSend(LPCTSTR strInp);
	int  ComSend(char cInp);
	int ComRead();
	void ComClose();
	int LSLConnect();
	int LPTConnect();
	DWORD WINAPI comRecThread( LPVOID lpParam);
	void ResetLPT();
#endif


