#include "Log.h"
#include "com.h"

FILE *plog=NULL;
volatile bool LogLock=false;
extern DWORD startTime;
extern DWORD GetQPC();
extern DWORD frameTimer;

void EventLog(int type,int instance, int state)
{
	EventLog(type,instance,state,"Event");
}

bool logCom=true;
extern int comport;
void ComLog(int val)
{
	int res=ComSend(val);

	if(logCom&&LogStatus)
	{
		char msg[800];
		sprintf(msg,"%d\t%d\t%s\t%d\t%d\t%d\n",GetQPC()-startTime,GetQPC()-frameTimer,"COM",comport,val,res);
		AddToLog(msg);
	}
}


void EventLog(int type,int instance, int state, char* str)
{

	ResetLPT();

	if(LogStatus)
	{
		char msg[800];
		sprintf(msg,"%d\t%d\t%s\t%d\t%d\t%d\n",GetQPC()-startTime,GetQPC()-frameTimer,str,type,instance,state);
		AddToLog(msg);
	}

	//Type is most commonly frame type
	//instance may refer to frame count
	// state is state of event
	// string is name of event

	//Only other com logging is for correct/expected Response (listed as 200+target)
	//And Mouse clicks/ keyboard press



	switch(type)
	{
		case -100: //Start Marker, old version was 100
			ComLog(50);
			break;
		case -101: //End Marker, old version was 100
			ComLog(51);
			break;
		case 1:
			ComLog(1);//Default Frame , old version was 10
			break;
		case 10:
			ComLog(10);//Timer Frame
			break;
		case -1:
			ComLog(21);//mouse press
			break;
		case -2:
			ComLog(22);//keyboard press
			break;
		default:
			break;
	}


}


int InitLog(char file[])
{
	plog=NULL;
	plog=fopen(file,"w");
	if(plog!=NULL)
		return 1;
	else
		return 0;
}

int LogStatus()
{
	if(plog)
		return 1;
	else return 0;
}

void AddToLog(char line[])
{
	while(LogLock)
	{
	}
	LogLock=true;
	if(plog)
	{
		fprintf(plog,"%s",line);
	}
	LogLock=false;

}

void CloseLog()
{
	while(LogLock)
	{

	}
	LogLock=true;
	if(plog)
		fclose(plog);
	LogLock=false;
}

