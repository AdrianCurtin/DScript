#include <stdio.h>

class PresentationFrame
{
public:
	PresentationFrame(void)
	{this->frameType=-100;this->done=true;this->response=false;};
	PresentationFrame(int frameType,int texID,double timeout)
	{ this->frameType=frameType; this->texID=texID; this->timeout=timeout;this->done=false;this->response=false;};
	PresentationFrame(int FrameType,int texID,double timeout,char* str)
	{ this->frameType=frameType; this->texID=texID; this->timeout=timeout; sprintf(this->str,"%s",str);this->done=false;this->response=false;};
	PresentationFrame(int FrameType,int texID,double timeout,char* str,int correctResponse,int audioID)
	{ this->frameType=frameType; this->texID=texID; this->timeout=timeout; sprintf(this->str,"%s",str); this->correctResponse=correctResponse; this->audioID=audioID;this->done=false;this->response=false;};

	int frameType;
	int timeout;
	int correctResponse;
	char str[800];
	int texID;
	int audioID;
	bool logUserEvents;
	bool done;
	int response;
	int drawCount;
};