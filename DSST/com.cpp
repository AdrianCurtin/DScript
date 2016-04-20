#include "com.h"
#include "Libs\\liblsl\\lsl_cpp.h"
#include "stdlib.h"

HANDLE hCom=NULL;
extern HWND	hWnd;	// Holds Our Window Handle


HANDLE comRec;


volatile int bufferVar=0;
OVERLAPPED her;

extern bool bSendSync;
bool bListen=false; //listen for cue
int triggerChar;


extern bool bSendSync;
extern bool bUseLPT;
extern bool bUseLSL;

lsl::stream_info info("DSSTMarkerStream", "Markers", 1, lsl::IRREGULAR_RATE, lsl::cf_int32, "myuniquesourceid23443");

// make a new outlet
lsl::stream_outlet outlet(info);


typedef void(__stdcall *lpOut32)(short, short);
typedef short(__stdcall *lpInp32)(short);
typedef BOOL(__stdcall *lpIsInpOutDriverOpen)(void);
typedef BOOL(__stdcall *lpIsXP64Bit)(void);

//Some global function pointers (messy but fine for an example)
lpOut32 gfpOut32;
lpInp32 gfpInp32;
lpIsInpOutDriverOpen gfpIsInpOutDriverOpen;
lpIsXP64Bit gfpIsXP64Bit;
HINSTANCE hInpOutDll;

//#define TRIGGER_CHAR1	84 //'T'
//#define TRIGGER_CHAR2	116 // 't'

int ComConnect(int iPort,DWORD baud,bool parity,int stopBits)
{

	COMMTIMEOUTS CommTimeouts;
	DCB dcb;
	BOOL fSuccess;

	char port[20];
	sprintf(port,"COM%d",iPort);

	hCom = CreateFile(port,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);

	//hCom = CreateFile(port,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,NULL,NULL);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		ComError("CreateFile");
		return 0;
	}

	fSuccess = GetCommState(hCom, &dcb);
	if (!fSuccess) 
	{
		ComError("GetComstate");
		return 0;
	}

	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	if(!parity)
		dcb.Parity = NOPARITY;
	else
		dcb.Parity=PARITY_ODD;
	if(stopBits==0)
	dcb.StopBits = ONESTOPBIT;
	else if (stopBits==1)
		dcb.StopBits=ONE5STOPBITS;
	else if (stopBits==2)
		dcb.StopBits=TWOSTOPBITS;

	fSuccess = SetCommState(hCom, &dcb);
	if(!fSuccess) 
	{
		ComError("SetComstate");
		return 0;
	}

	GetCommTimeouts (hCom, &CommTimeouts);

	// Change the COMMTIMEOUTS structure settings.
	CommTimeouts.ReadIntervalTimeout = MAXDWORD;  
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;  
	CommTimeouts.ReadTotalTimeoutConstant = 0;  

	CommTimeouts.WriteTotalTimeoutMultiplier = 10;  
	CommTimeouts.WriteTotalTimeoutConstant = 1000;    

	// Set the time-out parameters for all read and write operations
	if(!SetCommTimeouts (hCom, &CommTimeouts))
	{
		ComError("SetTimeouts");
		return 0;
	}

	PurgeComm(hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	// Direct the port to perform extended functions SETDTR and SETRTS
	// SETDTR: Sends the DTR (data-terminal-ready) signal.
	// SETRTS: Sends the RTS (request-to-send) signal. 
	EscapeCommFunction(hCom, SETDTR);
	EscapeCommFunction(hCom, SETRTS);

	memset( &her, 0, sizeof( OVERLAPPED ) ) ;
	her.hEvent = CreateEvent(
		NULL,   // default security attributes 
		FALSE,  // auto reset event 
		FALSE,  // not signaled 
		NULL    // no name 
		);

	if(bListen)
	{
		DWORD devID=0;
		comRec = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			comRecThread,       // thread function name
			NULL,          // argument to thread function 
			0,                      // use default creation flags 
			&devID);   // returns the thread identifier
	}

	return 1;
}


void ComError(LPCTSTR text)
{
	//wsprintf(d,"%s Error # : %d ",text,GetLastError());
	//AfxMessageBox(d);

	if (bUseLSL)
	{
		//std::string mrk;
		//char* m = new char[128];
		//sprintf(m, "%s", strInp);
		// now send it (note the &)
		//mrk = m;

		//outlet.push_sample(&mrk);

		//std::string mrk;
		int m = (int)255;
		//sprintf(m, "Mrk %i",cInp);
		// now send it (note the &)
		//mrk = m;

		outlet.push_sample(&m);
	}

	if (bUseLPT)
	{
		short iPort = 0x378;
		WORD wData = 255;
		gfpOut32(iPort, wData);
	}

	TCHAR d[500];

	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0, // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	wsprintf(d,"Communication Error <%s>\n%s",text,lpMsgBuf);
	//AfxMessageBox((LPCTSTR)d, MB_OK | MB_ICONINFORMATION );
	//CobiPrintText(d,error);
	MessageBox(hWnd,d,"Error",MB_OK);
	LocalFree(lpMsgBuf);
}

int ComSend(LPCTSTR strInp)
{
	if (bUseLSL)
	{
		//std::string mrk;
		//char* m = new char[128];
		//sprintf(m, "%s", strInp);
		// now send it (note the &)
		//mrk = m;

		//outlet.push_sample(&mrk);

		//std::string mrk;
		int m = (int)255;
		//sprintf(m, "Mrk %i",cInp);
		// now send it (note the &)
		//mrk = m;

		outlet.push_sample(&m);
	}

	if (bUseLPT)
	{
		short iPort = 0x378;
		WORD wData = 255;
		gfpOut32(iPort, wData);
	}

	if(hCom==NULL||!bSendSync) return 0;
	int i=0;
	while(i<100 && strInp[i]!=NULL)
		i++;
	DWORD out=0;
	WriteFile(hCom,strInp,i,&out,&her);
	

	if(out!=i)
		return 0;
	return 1;
}

void ResetLPT()
{
	if (bUseLPT)
	{
		WORD iPort = 0x378;
		gfpOut32(iPort, 0);
	}
}

int ComSend(char cInp)
{
	ResetLPT();
	if (bUseLSL)
	{
		//std::string mrk;
		int m = (int)cInp;
		//sprintf(m, "Mrk %i",cInp);
		// now send it (note the &)
		//mrk = m;

		outlet.push_sample(&m);
	}

	if (bUseLPT)
	{
		WORD iPort = 0x378;
		WORD wData = (int)cInp;
		gfpOut32(iPort, 0);
		for (int i = 0; i < 500; i++)
		{
			gfpOut32(iPort, wData);
		}
		gfpOut32(iPort, wData);
	}


	if(hCom==NULL||!bSendSync) return 0;
	DWORD out=0;
	WriteFile(hCom,&cInp,1,&out,&her);

	if(out!=1)
		return 0;
	return 1;
}

void ComClose()
{
	if(hCom!=NULL)
	{

		CloseHandle(hCom);
		hCom = NULL;
	}

	if (hInpOutDll)
		FreeLibrary(hInpOutDll);
}

DWORD WINAPI comRecThread( LPVOID lpParam)
{

	LARGE_INTEGER now,freq;
	unsigned char buffer[2];
	char daq[50];
	char msg[800];

	//Serial port initialization
	DWORD dwEvtMask,gelen;

	gelen = SetCommMask(hCom, EV_RXCHAR);
	if (!gelen)
	{
		return 0;
	}

	while(bListen)
	{

		if(WaitCommEvent(hCom, &dwEvtMask, &her))
		{
			if (dwEvtMask & EV_RXCHAR)
			{
				do{
					ReadFile(hCom,buffer,1,&gelen,&her);
					if(GetOverlappedResult(hCom,&her,&gelen,TRUE)==0)
					{
						//CheckError("ReadFile");
					}
					if(gelen>0)
					{
						 bufferVar=buffer[0]; 
						 //if(bufferVar==TRIGGER_CHAR1 || bufferVar==TRIGGER_CHAR2)
						 if(bufferVar == triggerChar)
						 {
							 //EventLog(2,1,bufferVar);
						 }

						 //EventLog(30,0,bufferVar);

					}

				}while(gelen);                           
			}
		}
		else
		{
			WaitForSingleObject(her.hEvent,500);
			if(GetOverlappedResult(hCom,&her,&gelen,FALSE))
			{
				if (dwEvtMask & EV_RXCHAR)
				{
					do{
						ReadFile(hCom,buffer,1,&gelen,&her);
						if(GetOverlappedResult(hCom,&her,&gelen,TRUE)==0)
						{
							ComError("ReadFile");
						}

						if(gelen>0)
						{
							bufferVar=buffer[0];  
							//if(bufferVar==TRIGGER_CHAR1 || bufferVar==TRIGGER_CHAR2)
							if(bufferVar == triggerChar)
							{

							}


						}
					}while(gelen);                           
				}//if mask
			}
		}
		//Sleep(10);
		Sleep(1);
	}
	if(hCom)
	{
		CloseHandle(hCom);
		hCom=NULL;
	}
	return 1;

}

int LPTConnect()
{
	HINSTANCE hInpOutDll;
	hInpOutDll = LoadLibrary("InpOut32.DLL");	//The 32bit DLL. If we are building x64 C++ 
	//applicaiton then use InpOutx64.dll
	if (hInpOutDll != NULL)
	{
		gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "Out32");
		gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "Inp32");
		gfpIsInpOutDriverOpen = (lpIsInpOutDriverOpen)GetProcAddress(hInpOutDll, "IsInpOutDriverOpen");
		gfpIsXP64Bit = (lpIsXP64Bit)GetProcAddress(hInpOutDll, "IsXP64Bit");


	}
	else
	{
		printf("Unable to open InpOut32 Driver!\n");
		return 0;
	}


	return 1;
}

int LSLConnect()
{
	// make a new stream_info


	return 1;
}