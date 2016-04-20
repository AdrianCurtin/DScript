#include "game.h"

freetype::font_data freeGLfont;
extern void  ComLog(int val);

//The projection scale
GLfloat gProjectionScale = 1.f;

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
int Height,Width;
DWORD startTime;
double fontFactor=1;

DWORD frameTimer;
GLuint bgTex;
PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;


bool WGLExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported


    return true;
}





#define FRAMELIMIT 10000
extern PresentationFrame pFrames[FRAMELIMIT];
PresentationFrame currentFrame;
int frameIndex=0;

bool finished=false;
extern bool fAbort;
extern int fontsize;
extern bool bShowTimer;
extern GLuint test;
extern bool bFullscreen;
extern float bgColor[3];
extern float distractColor[3];
extern float fontColor[3];
extern char* fWorkingDirectory;
extern bool renderFrame;

DWORD lastFrameTime=0;
DWORD lastFrameUpdate;
bool lastFrameNoResponse=false;


extern AudioDictionary curAudioDict; //defined in main.cpp


bool initGL()
{
	if(bFullscreen)
		glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE); 

	fontFactor=fontsize/12;
	BuildFont();
	LoadTextures();
	currentFrame.done=true;	
	startTime=GetQPC();
	EventLog(-100,0,0,"Start");
	Height=SCREEN_HEIGHT;
	Width=SCREEN_WIDTH;
	hWnd=GetActiveWindow();
	hDC=GetDC(hWnd);
    //Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0 );
	glutIdleFunc(0); //turns off idle function

    //Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping 
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	            // Clear The Screen And The Depth Buffer

	glDisable(GL_LIGHTING);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	//glEnable(GL_DEPTH_TEST);							// Enables Depth Testing							
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// For Nice Perspective Calculations
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //Initialize clear color
    glClearColor( 0.f, 0.f, 0.f, 0.5f );

	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(0);
		// this is another function from WGL_EXT_swap_control extension
		//    wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) LogGetProcAddress("wglGetSwapIntervalEXT");
	}

	

    //Check for error
    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        printf( "Error initializing OpenGL! %s\n", gluErrorString( error ) );
        return false;
    }

    return true;
}

void LoadTextures()
{
	TextureDictionaryItem* curItem=texDict.cListRoot;
	for(int i=1;i<texDict.GetLength()+1;i++)
	{
		curItem=curItem->next;
		curItem->key=LoadTexture(curItem->fname,curItem->value);
	}
}



void BuildFont()
{
	char * pSystemRoot;
	pSystemRoot = getenv("WINDIR");

	char* fontname = "msyh.ttc";

	
	char* fontpath = new char[512];
	sprintf(fontpath, "%s\\Fonts\\%s",pSystemRoot, fontname);

	if (CheckFile(fontpath))
	{
		freeGLfont.init(fontpath, fontsize);                                  // Build The FreeType Font
		return;
	}
	
	fontpath = new char[512];
	fontname = "arial.ttf";
	sprintf(fontpath, "%s\\Fonts\\%s", pSystemRoot, fontname);

	if (CheckFile(fontpath))
		freeGLfont.init(fontpath, fontsize);                                  // Build The FreeType Font

}

int LoadTexture(char* fname, int index)
{
		int n=7;
		
		
		//Function expects fname to have 'imgs//' attatched to it as an initiating string
					

		if(fname[n-1]== ' '||strlen(fname)<4)
		{
			//do not load image..
		}
		else
		{										
			int i = texDict.GetLength();
			i++; //index for this texture...
			// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
						
			int tx=0;

			char fname2[300];
			char fname3[300];
						
			if(CheckFile(&fname[5]))
				tx=glmLoadTexture(&(fname[5]));
			else if(CheckFile(fname))
			{
				tx=glmLoadTexture(fname);
			}
			else
			{

				strcpy(fname2,fWorkingDirectory);
				strcat_s(fname2,&fname[5]);

				strcpy(fname3,fWorkingDirectory);
				strcat_s(fname3,fname);

				
				if(CheckFile(fname2))
					tx=glmLoadTexture(fname2);
				else if(CheckFile(fname3))
				{
					tx=glmLoadTexture(fname3);
				}
			}
						
			if(tx==0||tx==-1)
			{
					
					char txt[250];
					sprintf(txt,"Texture '%s' not found!",&(fname[5]));
					MessageBox(NULL,txt,0,0);
			}	
			return tx;
		}
		return 0;
}
bool newFrame=false;

DWORD distractTimer;
DWORD currentTime;
DWORD frameTime;
bool distractState=false;
bool expTimerEnabled=false;
DWORD expTIMER;
DWORD expTIMEOUT;
#define DISTRACTIONBG	-10
#define SYSCLOCKFRAME		100
#define SYSCLOCKGOTO		101

void update()
{
	currentTime=GetQPC();
	
	frameTime=currentTime-frameTimer;
	DWORD lastFrameUpdateTime= lastFrameUpdate-currentTime;

	
	
	if(finished)
	{
			EventLog(-101,0,0,"End");
			glutExit();
	}
	else{

	Width=glutGet(GLUT_WINDOW_WIDTH);
	Height=glutGet(GLUT_WINDOW_HEIGHT);
	
	if(currentFrame.frameType==SYSCLOCKFRAME)
	{
		currentFrame.done=true;
		currentFrame.drawCount=1;
		expTIMER=GetQPC();
		expTIMEOUT=currentFrame.timeout;
		EventLog(300,expTIMEOUT,-1,"ExperimentTimerStarted");
		expTimerEnabled=true;
	}

	if(currentFrame.frameType==SYSCLOCKGOTO)
	{
		currentFrame.done=true;
		currentFrame.drawCount=1;
		EventLog(301,expTIMEOUT,currentTime-expTIMER,"ExperimentTimerCompletedPrematurely");
		expTIMER=0;
		expTIMEOUT=0;
		expTimerEnabled=false;
	}

	if(expTimerEnabled&&currentTime-expTIMER>=expTIMEOUT)
	{
		while(currentFrame.frameType!=SYSCLOCKGOTO&&!currentFrame.done)
		{
			currentFrame=pFrames[frameIndex];
			frameIndex++;
		}
		if(currentFrame.done)	//last frame reached if current frame is already marked done
		{
			finished=true;
		}
		EventLog(currentFrame.frameType,expTIMEOUT,currentTime-expTIMER,"ExperimentTimerCompleted");
		newFrame=false;
		currentFrame.done=true;
		currentFrame.drawCount=1;
		expTimerEnabled=false;
	}

	if(currentFrame.texID==DISTRACTIONBG)
	{
		if(currentTime-distractTimer>=100)
		{
			distractTimer=currentTime;
			distractState=!distractState;
			forceFrameUpdate();
		}
		
	}
	if(currentFrame.drawCount>0)
	{
		if(currentFrame.timeout!=0&&frameTime>=abs(currentFrame.timeout))
		{
			currentFrame.done=true;

			if(currentFrame.timeout>0)
				EventLog(102,currentFrame.correctResponse,currentFrame.response,"fTimeout");
			else
			{
				currentFrame.done=true;
				EventLog(102,currentFrame.correctResponse,-1,"fNoResponse");
				lastFrameNoResponse=true;
			}

		}
		else if(abs(currentFrame.timeout)-frameTime<10)
		{
			renderFrame=false;
		}
		else
			forceFrameUpdate();

		if(currentFrame.done)
		{
		
			forceFrameUpdate();
			
			curAudioDict.RecordStop();
			if(currentFrame.correctResponse>=1)
			{
				ComLog((currentFrame.correctResponse+200));
			}
			

			currentFrame=pFrames[frameIndex];
			newFrame=true;

			frameTimer=currentTime;
			frameIndex++;
			
			if(currentFrame.done==true) //if new frame loaded is already marked 'done'
			{
				finished=true;
			}
			else
			{
				EventLog(currentFrame.frameType,frameIndex,0,"fNext");
			}

			

			if(currentFrame.audioID==-1)
			{
				curAudioDict.RecordStart();
			}
		
		}
	}
	else{
		forceFrameUpdate();
	}
	}
	
}



void forceFrameUpdate()
{
	renderFrame=true;
	lastFrameUpdate=GetQPC();
}

void close()
{
	
}

void useBgColor()
{
	if(!distractState||currentFrame.texID!=DISTRACTIONBG)
		glColor3f(bgColor[0],bgColor[1],bgColor[2]);
	else
		glColor3f(distractColor[0],distractColor[1],distractColor[2]);
}


void useFontColor()
{
	glColor3f(fontColor[0],fontColor[1],fontColor[2]);
}

void render()
{
    //Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 

	
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(0, Width, 0, Height);
	GLuint bgTex=texDict.GetKey(100);

	glBindTexture(GL_TEXTURE_2D,0);
	
		
	useBgColor();
	glBegin(GL_QUADS); //BG Color Fill Box
		glTexCoord2d(0,0); glVertex2f(0, 0);
		glTexCoord2d(1,0); glVertex2f(Width, 0);
		glTexCoord2d(1,1); glVertex2f(Width, Height);
		glTexCoord2d(0,1); glVertex2f(0, Height);
	glEnd();	

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(bgTex>0&&bgTex<5000)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, bgTex);
		glColor3f(1,1,1);							
		glBegin(GL_QUADS); //BG Texture Box
			glTexCoord2d(0,0); glVertex2f(0, 0);
			glTexCoord2d(1,0); glVertex2f(Width, 0);
			glTexCoord2d(1,1); glVertex2f(Width, Height);
			glTexCoord2d(0,1); glVertex2f(0, Height);
		glEnd();							
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,0);
	}

	drawCurrentFrame();
	currentFrame.drawCount++;

	glColor3f(0,0,1);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, Width, 0, Height);
	           // Clear The Screen And The Depth Buffer


	if(bShowTimer)
	{
		DWORD X=GetQPC();
		glPrint(0, 1 * fontFactor, false, "%.3f", (float)(X - startTime) / 1000.0);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
    //Update screen

	SwapBuffers(hDC);
	//glutSwapBuffers();

}

#define TIMERFRAME			10
#define SHOWTIMERFRAME		11
#define QUICKFEEDBACKFRAME	14
#define SATSTIMFRAME		15
#define SATRESPONSEFRAME	16
#define SATFEEDBACKFRAME	17
#define SCOREPERCENTAGEFRAME	18
#define SCORECOUNTFRAME		19
#define DSSTFRAME			20
#define DSSTIMGFRAME		21




int lastResponse=-1;
int lastWasTarget=-1;
int numResponses=0;
int numTargets=0;


extern char symbolList[9];

void FrameLogic()
{
	newFrame=false;
	currentFrame.drawCount=0;




	if(currentFrame.frameType==DSSTFRAME)
	{
		sprintf(currentFrame.str,"%c",symbolList[currentFrame.correctResponse-1]);
		numTargets++;
	}

	if(currentFrame.frameType==DSSTIMGFRAME)
	{
		numTargets++;
	}

	if(currentFrame.frameType==SATFEEDBACKFRAME)
	{
		numTargets++;
		if((lastWasTarget==1&&lastResponse==1)||(lastWasTarget==0&&lastResponse==0))
		{
			currentFrame.correctResponse=1;
			numResponses++;
		}
		else
		{
			currentFrame.audioID=0;
			currentFrame.correctResponse=0;
		}


		lastWasTarget=-1;
		lastResponse=-1;
	}

	if(currentFrame.frameType!=DSSTFRAME&&currentFrame.frameType!=DSSTIMGFRAME&&currentFrame.frameType!=QUICKFEEDBACKFRAME)
	{
		curAudioDict.Play(currentFrame.audioID,0);
	}

	if(currentFrame.frameType==SATSTIMFRAME)
	{
		lastWasTarget=currentFrame.correctResponse;
		distractTimer=GetQPC();
		distractState=false;
		if(currentFrame.correctResponse==0)	//FrameType 15 is SAT stimulus, if non-target do not display stimulus square
		{
			currentFrame.texID=0;
		}
	}

	if(currentFrame.frameType==SCOREPERCENTAGEFRAME)
	{
		sprintf(&currentFrame.str[0],"%.1f \%",(float)numResponses/(float) numTargets*100.0f);
		numTargets=0;
		numResponses=0;
	}
	if(currentFrame.frameType==SCORECOUNTFRAME)
	{
		sprintf(&currentFrame.str[0],"%.0f w/ %.0f Err",(float) numResponses,float(numTargets-numResponses-1));
		numTargets=0;
		numResponses=0;
	}

	if(currentFrame.frameType==SHOWTIMERFRAME)
	{
		if(!lastFrameNoResponse)
		{
			currentFrame.texID=0;
		}
	}
}

void drawCurrentFrame()
{
	if(newFrame)
	{
		FrameLogic();
	}

	if(currentFrame.texID>0)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,texDict.GetKey(currentFrame.texID));
		glColor3f(1,1,1);							
		glBegin(GL_QUADS); //BG Fill Box
			glTexCoord2d(0,0); glVertex2f(0, 0);
			glTexCoord2d(1,0); glVertex2f(Width, 0);
			glTexCoord2d(1,1); glVertex2f(Width, Height);
			glTexCoord2d(0,1); glVertex2f(0, Height);
		glEnd();							
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,0);
	}

	if(currentFrame.frameType==SHOWTIMERFRAME)
	{
		if(!lastFrameNoResponse)
		{
			
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, Width, 0, Height);
			useFontColor();
			glPrint(Width / 2, Height / 2 + 30 * fontFactor,true,"%.3f", (float)(lastFrameTime / 1000.0));
		}
	}

	if(currentFrame.frameType==DSSTFRAME)
	{
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, Width, 0, Height);
			useFontColor();
			
			glPrint(Width / 2 - 145 * fontFactor, Height / 2 + 90 * fontFactor, true, "1   2   3   4   5   6   7   8   9");
			glPrint(Width / 2 - 150 * fontFactor, Height / 2 + 60 * fontFactor, true, 
				"%c   %c   %c   %c   %c   %c   %c   %c   %c", symbolList[0], symbolList[1], symbolList[2], symbolList[3], symbolList[4], symbolList[5], symbolList[6], symbolList[7], symbolList[8]);
	}

	if(currentFrame.frameType==DSSTIMGFRAME)
	{
		drawDSSTimgFrame();
	}

		useFontColor();
		glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, Width, 0, Height);
	           // Clear The Screen And The Depth Buffer
	
	glPrint(Width / 2 , Height / 2, true,
		"%s", currentFrame.str);

	//Draw Frame Timer
	if(currentFrame.frameType==0||currentFrame.frameType==TIMERFRAME)
	{
		lastFrameTime=GetQPC()-frameTimer;
		glPrint(Width / 2 - 20 * fontFactor, Height / 2 + 30 * fontFactor, true,
			"%.3f", (float)(lastFrameTime / 1000.0));
	}


}

void drawDSSTimgFrame()
{
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, Width, 0, Height);
			useFontColor();
 			double imgWidth=Height/9-6;

			for (int i=0;i<9;i++)
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texDict.GetKey(51+i));
				glColor3f(1,1,1);							
				
				glBegin(GL_QUADS); //BG Fill Box
					glTexCoord2d(0,0); glVertex2f((Width)/9*(i+0.5)-imgWidth/2, Height*5/6);
					glTexCoord2d(1,0); glVertex2f((Width)/9*(i+0.5)+imgWidth/2, Height*5/6);
					glTexCoord2d(1,1); glVertex2f((Width)/9*(i+0.5)+imgWidth/2, Height*5/6+imgWidth);
					glTexCoord2d(0,1); glVertex2f((Width)/9*(i+0.5)-imgWidth/2, Height*5/6+imgWidth);
				glEnd();	

				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,texDict.GetKey(41+i));
				glColor3f(1,1,1);							
				glBegin(GL_QUADS); //BG Fill Box
					glTexCoord2d(0,0); glVertex2f((Width)/9*(i+0.5)+imgWidth/2, Height*4/6);
					glTexCoord2d(1,0); glVertex2f((Width)/9*(i+0.5)-imgWidth/2, Height*4/6);
					glTexCoord2d(1,1); glVertex2f((Width)/9*(i+0.5)-imgWidth/2, Height*4/6+imgWidth);
					glTexCoord2d(0,1); glVertex2f((Width)/9*(i+0.5)+imgWidth/2, Height*4/6+imgWidth);
				glEnd();							

			}
			glBindTexture(GL_TEXTURE_2D,texDict.GetKey(currentFrame.correctResponse+40));
			glColor3f(1,1,1);							
			glBegin(GL_QUADS); //BG Fill Box
				glTexCoord2d(0,0); glVertex2f((Width)/9*4.5+imgWidth/2, Height/3);
				glTexCoord2d(1,0); glVertex2f((Width)/9*4.5-imgWidth/2, Height/3);
				glTexCoord2d(1,1); glVertex2f((Width)/9*4.5-imgWidth/2, Height/3+imgWidth);
				glTexCoord2d(0,1); glVertex2f((Width)/9*4.5+imgWidth/2, Height/3+imgWidth);
			glEnd();							
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,0);

}

#define K_SPACE ' '
#define K_ESC '\x1b'


void handleKeys( unsigned char key, int x, int y )
{
    //If the user a key
	EventLog(-10,key,1,"keypress"); //log any key press
	if(currentFrame.frameType==DSSTFRAME||currentFrame.frameType==DSSTIMGFRAME)
	{
		switch(key)
		{
			case '1':
				currentFrame.response=1;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '2':
				currentFrame.response=2;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '3':
				currentFrame.response=3;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '4':
				currentFrame.response=4;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '5':
				currentFrame.response=5;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '6':
				currentFrame.response=6;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '7':
				currentFrame.response=7;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '8':
				currentFrame.response=8;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			case '9':
				currentFrame.response=9;
				currentFrame.done=true;
				EventLog(100,currentFrame.correctResponse,currentFrame.response,"DSSTRespond");
				break;
			default:
				break;
		}	
		if(currentFrame.response==currentFrame.correctResponse)
		{
			curAudioDict.Play(currentFrame.audioID,0);
			numResponses++;
		}
	}
	else if(currentFrame.frameType==SATRESPONSEFRAME)
	{

	}
    else if( key == K_SPACE )
    {
			
		EventLog(-2,key,1,"space");	 //log specific key press
		currentFrame.response=1;

		if(currentFrame.frameType==QUICKFEEDBACKFRAME)
		{
			curAudioDict.Play(currentFrame.audioID);
			currentFrame.audioID=0;
		}

		if(currentFrame.timeout<=0)
		{
			currentFrame.done=true;
			EventLog(100,currentFrame.correctResponse,currentFrame.response,"fRespond");
			lastFrameNoResponse=false;
		}
	}
	
	if(key==K_ESC)
	{
		EventLog(-2,key,1,"Escape");	 //log specific key press
		currentFrame.done=true;
		EventLog(101,currentFrame.correctResponse,currentFrame.response,"fAbort");
		finished=true;
		fAbort=true;
	}
}



void handleMouse( int button, int state, int x, int y )
{
    //If the user clicks
	if( button == 0 && state== 0 && currentFrame.frameType!=SATRESPONSEFRAME&&currentFrame.frameType!=DSSTFRAME&&currentFrame.frameType!=DSSTIMGFRAME)
    {
			
		EventLog(-1,x,y,"mouseclick");
		currentFrame.response=1;

		if(currentFrame.frameType==QUICKFEEDBACKFRAME)
		{
			curAudioDict.Play(currentFrame.audioID);
			currentFrame.audioID=0;
		}

		if(currentFrame.timeout<=0)
		{
			currentFrame.done=true;
			EventLog(100,currentFrame.correctResponse,currentFrame.response,"fRespond");
			lastFrameNoResponse=false;
		}
	}

	if(currentFrame.frameType==SATRESPONSEFRAME&&state==0&&(button==0||button==2))	//if SAT signal present Y/N
	{
		if(button==0)
			lastResponse=1;
		else if(button==2)
			lastResponse=0;

		EventLog(100,lastWasTarget,button,"SATresponse");

		currentFrame.correctResponse=lastWasTarget;
		currentFrame.response=lastResponse;

		if(currentFrame.timeout<=0)
		{
			currentFrame.done=true;
			EventLog(100,currentFrame.correctResponse,currentFrame.response,"fRespond");
			lastFrameNoResponse=false;
		}
	}
}



void handleMouseMove(int x, int y)
{

}


GLvoid KillFont(GLvoid)									// Delete The Font List
{
	freeGLfont.clean();
}

int bitvalue(unsigned char num,unsigned char bit)
{
	if (bit >= 0 && bit <= 7)
		return ((num >> (7-bit)) & 1);
	else
		return 0;
}

void ascii2utf8(wchar_t* out,char* text, const int tsize)
{
	wchar_t wtext[512]=L"";

	int wi = 0;
	int endStr = 0;

	for (int i = 0; i < tsize; i++) //search for line end
	{
		endStr++;
		if (text[i] == 0)
			break;
	}

	int utfs = 0;

	for (int i = 0; i < endStr; i++)
	{
		unsigned char c[4];
		c[0]= text[i];
		

		if (c == 0)
			break;
		bool invalidFlag = false;
		if (bitvalue(c[0], 0) == 0)	//0xxxxxxx
		{
			utfs = 0;
		}
		else if (endStr>=i+1&&(bitvalue(text[i], 2) == 0) && (bitvalue(text[i], 0) == 1))	//10xxxxxx
		{
			utfs = 1;
			c[1] = text[i+1];
		}
		else if ((endStr>=i+2)&&(bitvalue(text[i], 3) == 0)&& (bitvalue(text[i+1], 0) == 1) && (bitvalue(text[i+2], 0) == 1)) //110xxxxx
		{
			utfs = 2;
			c[1] = text[i + 1];
			c[2] = text[i+2];
		}
		else if ((endStr>=i + 3) && (bitvalue(text[i], 4) == 0) && (bitvalue(text[i+1], 0) == 1) && (bitvalue(text[i + 2], 0) == 1) && (bitvalue(text[i + 3], 0) == 1)) //1110xxxx
		{
			utfs = 3;
			c[1] = text[i + 1];
			c[2] = text[i + 2];
			c[3] = text[i + 3];
		}
		else
		{
			invalidFlag = true;//miscount or something
		}

		if (!invalidFlag)
		{


			wtext[wi] = c[utfs] - 128 * (utfs > 0) + (utfs > 1)*(c[utfs - 1] - 128)*pow(2, 6) + (utfs > 2)*(c[utfs - 2] - 128)*pow(2, 12) +
				+(utfs == 3)*(c[0] - 240)*pow(2, 18) + (utfs == 2)*(c[0] - 224)*pow(2, 12) + (utfs == 1)*(c[0] - 192)*pow(2, 6);
		
			wi++;
			i += utfs;
		}
	}
	wtext[wi] = 0;
	for (int i = 0; i <= wi; i++) //search for line end
	{
		out[i] = wtext[i];
	}
	
}

GLvoid glPrint(float x, float y, bool center,const char *fmt, ...)					// Custom GL "Print" Routine
{
	
	char text[512] = "ä½ å¥½t";										// Holds Our String
	

	va_list	ap;											// Pointer To List Of Arguments

	if (fmt == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);									// Parses The String For Variables
	    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
	va_end(ap);											// Results Are Stored In Text

	

	
	wchar_t ws[512]=L"";
	swprintf(ws, 512, L"%hs", text);

	ascii2utf8(ws, text, 512);

	//std::mbstate_t state{}; // zero-initialized to initial state

	//std::mbrtowc(ws, text, 512, &state); //convert to UNICODE?!?!?!?
	
	
	freetype::print(freeGLfont, x, y ,center,L"%s",ws);
}