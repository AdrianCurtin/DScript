#include "main.h"
#include <iostream>
#include <fstream>
#include <sstream>

AudioDictionary curAudioDict;


#define FRAMELIMIT 100000
#define MACROLENGTH 100000
#define MAXFRAMESTRING 1000
#define MACROCOUNT	100
#define MAXMACROVARS	15 //number of variables allowed in macros


    char * strsep(char **sp, char *sep)
    {
        char *p, *s;
        if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
        s = *sp;
        p = s + strcspn(s, sep);
        if (*p != '\0') *p++ = '\0';
        *sp = p;
        return(s);
    }

double round(double f)
{
    return floor(f*100 + 0.5)*10;
}

class macro{
	public:
		int numDVars;
		int numSVars;
		bool initialized;
		int id = -1;



		rapidxml::xml_node<> *mNode;
		macro()
		{
			initialized=false;
			numDVars=0;
			numSVars=0;
			id = -1;
			mNode = NULL;
		}
		macro(rapidxml::xml_node<>* Node,int id_in)
		{
			initialized = true;
			mNode = Node;
			numDVars = 0;
			numSVars = 0;
			id = id_in;

			//To do: Scan for all variables and count number of variable inputs
		}

		int dVars[MAXMACROVARS];
		int sVars[MAXMACROVARS];
		char macroString[MACROLENGTH];

		macro(char* str)	//accepts entire macro and scans for variable locations
		{
			initialized=true;
			numDVars=0;
			numSVars=0;

			for(int i=0;i<strlen(str)+1;i++)
			{
				macroString[i]=str[i];	//copy initial string to macroString
				if(str[i]=='#'&&str[i+1]=='s')
				{
					numSVars++;
					sVars[numSVars-1]=i;
				}
				else if(str[i]=='#'&&str[i+1]=='d')
				{
					numDVars++;
					dVars[numDVars-1]=i;
				}

			}
		}

		char* fillIn(double dArr[],char* str)
		{
			char macroStringFilled[MACROLENGTH];
			sprintf(macroStringFilled,"");

			char macroStringCopy[MACROLENGTH];

			strcpy(macroStringCopy,macroString);

			int b=(sizeof(dArr)/sizeof(double));

			int strVarCount=0;
			if(strlen(str)>0)
			{
				strVarCount++;
				for (int i=0;i<strlen(str);i++)		//see if number of string segments matches known count
				{
					if(str[i]=='\t')
						strVarCount++;
				}
			}
			if( strVarCount!=numSVars)
				return "";

			char* pch = strsep (&str,"\t");

			char* mString=&macroStringCopy[0];

			char* sepMacroString=strsep(&mString,"#");
			char* sepMacroString2=strsep(&mString,"#");
			
			int dVar=0;
			strcat(macroStringFilled,sepMacroString);
			for (int i=0;i<numDVars+numSVars;i++)
			{
				
				if(strlen(sepMacroString2)>1&&sepMacroString2[0]=='s')
				{
					
					strcat(macroStringFilled,pch);
					
					for (int j=1;j<strlen(sepMacroString2)+1;j++)
					{
						sepMacroString[j-1]=sepMacroString2[j];
						if(sepMacroString2[j]<0)
						{
							sepMacroString[j-1]=0;
							break;
						}
					}

					strcat(macroStringFilled,sepMacroString);
					sepMacroString2=strsep(&mString,"#");
					pch = strsep (&str,"\t");
				}
				else if(strlen(sepMacroString2)>1&&sepMacroString2[0]=='d')
				{

					char dString[80];
					sprintf(dString,"%lf",dArr[dVar]);

					strcat(macroStringFilled,dString);
					for (int j=1;j<strlen(sepMacroString2)+1;j++)
					{
						sepMacroString[j-1]=sepMacroString2[j];
						if(sepMacroString2[j]<0)
						{
							sepMacroString[j-1]=0;
							break;
						}
					}
					strcat(macroStringFilled,sepMacroString);
					sepMacroString2=strsep(&mString,"#");

					dVar++;
				}
			}
			
			return macroStringFilled;
		}
};

PresentationFrame pFrames[FRAMELIMIT];
bool bFullscreen=false;


bool fAbort=false;
bool autolog=true;
int comport=-1;
int fontsize=72;
float fontColor[3];
float bgColor[3];
float distractColor[3];
int frameDivider=10;
bool bShowTimer=false;
double totalTime=0;
char symbolList[9];
extern bool finished;

bool bSendSync=true;
bool bUseLSL = true;
bool bUseLPT = false;
/*
Pre Condition:
 -Initialized freeGLUT
Post Condition:
 -Calls the main loop functions and sets itself to be called back in 1000 / SCREEN_FPS milliseconds
Side Effects:
 -Sets glutTimerFunc
*/
TextureDictionary texDict;
GLuint test;

int main( int argc, char* args[] )
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	    //Initialize FreeGLUT
    glutInit( &argc, args );

    //Create OpenGL 2.1 context
    //*glutInitContextVersion( 2, 1 );

	InitializeQPC();

	curAudioDict.Stop(0);
	curAudioDict.Clear();

	char scriptFile[800];

	bool loadfile=true;

	//args[1]="D:\\Dropbox\\SCH\\ReleaseV2\\SAT\\SAT.ds";

	if(argc>1)
		sprintf(scriptFile,"%s",args[1]);
	else if(!loadfile)
		sprintf(scriptFile,"Sample.txt");
	else
	{
		HWND hwnd = NULL; 
		OPENFILENAME ofn;

		ZeroMemory( &ofn , sizeof( ofn));
		ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0dScript\0*.ds;*.dsx\0";
		ofn.lpstrInitialDir=NULL;
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		ofn.nFilterIndex =3;
		ofn.lStructSize = sizeof ( ofn );
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = scriptFile ;
		ofn.nMaxFile = sizeof( scriptFile );
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFileTitle = 0 ;
		GetOpenFileName( &ofn );

	}
	



	int res;
	if (scriptFile[strlen(scriptFile) - 3] == '.'&&tolower(scriptFile[strlen(scriptFile) - 2]) == 'd'&&tolower(scriptFile[strlen(scriptFile) - 1]) == 's')
		res= LoadScript(scriptFile);
	else if (scriptFile[strlen(scriptFile) - 3] == 'd'&&tolower(scriptFile[strlen(scriptFile) - 2]) == 's'&&tolower(scriptFile[strlen(scriptFile) - 1]) == 'x')
		res = LoadScriptXML(scriptFile);
	
	if (res < 0)
		return true;

	DWORD baud= 9600;
	bool parity=false;
	int stopBits=0;

	if(comport<0||!ComConnect(comport,baud,parity,stopBits))
	{
		bSendSync=false;
		if(comport>=0)
			MessageBoxA(NULL,"Invalid COM port",0,0);
	}

	if (bUseLSL)
	{
		LSLConnect();
	}

	if (bUseLPT)
	{
		LPTConnect();
	}
	
	if(autolog)
	{
		char logfile[200];
		time_t rawtime;
		time ( &rawtime );
		struct tm * timeinfo;
		timeinfo = localtime ( &rawtime );
		sprintf(logfile,"LogFiles");
		CreateDirectory(logfile,NULL);
		sprintf(logfile,"%s\\autoLog_%i_%i_%i_%i_%i_%i.txt",logfile,timeinfo->tm_year-100,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
		if((strlen(logfile)>2)&&!InitLog(logfile))
		{
			MessageBox(0,"Can not create automatic log file!. Make sure the folder is accessible and you read/write permissions.","Error",MB_ICONERROR);
		}
		AddToLog(logfile);
		AddToLog("\n");
		AddToLog(scriptFile);
		AddToLog("\n");
	}

	


    //Create Double Buffered Window
    glutInitDisplayMode( GLUT_DOUBLE );

	 glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
    glutCreateWindow( "dScriptLauncher" );
	glutHideWindow();

    //Do post window/context creation initialization
    if( !initGL() )
    {
        printf( "Unable to initialize graphics library!\n" );
        return 1;
    }

    //Set keyboard handler
    glutKeyboardFunc( handleKeys );

	//Set mouse handler
    glutMouseFunc( handleMouse );

	//Set mouse movement handler
	glutPassiveMotionFunc(handleMouseMove);

    //Set rendering function
    glutDisplayFunc( render );
	
	//Set close window handler
	glutCloseFunc(close); // set the window closing function of opengl

    //Set main loop
    glutTimerFunc( 1000 / SCREEN_FPS, runMainLoop, 0 );

	glutShowWindow();

    //Start GLUT main loop
    glutMainLoop();



	CloseLog();


    return fAbort;
}

#define LINEBUFFERLIMIT 800

#define TEXTURELIBRARY 100
#define PRESENTATIONLIST 200
#define AUDIOLIBRARY 300
#define MACROLIST	400

#define LOGOPTION 26
#define FULLSCREENOPTION 10
#define COMPORTOPTION 11
#define FONTSIZEOPTION 12
#define TIMEROPTION 13
#define FONTCOLOROPTION 14
#define BGCOLOROPTION 15
#define DISTRACTCOLOROPTION 16
#define FRAMEDIVIDEROPTION 19
#define DSSTSYMBOLS	20
#define LSLOPTION 24
#define LPTOPTION 25



macro macroList[MACROCOUNT];
int itemCount=0;
char* fWorkingDirectory=new char[256];


int LoadScript(char* filename)
{
	
	strcpy(fWorkingDirectory,filename);

	texDict= TextureDictionary();
	srand(GetQPC()*time(NULL)); //seed based on time
	FILE *fp=fopen(filename,"r");
	if(fp==NULL)
	{
		char message[800];
		sprintf(message,"Couldn't open Script file!\n%s",filename);
		MessageBoxA(NULL,message,NULL,NULL);
		autolog = false;
		return -1;
	}
	else
	{
		int fDirectoryIndex=0;
		for (int i=strlen(filename);i>0;i--)
		{
			if(filename[i]=='\\')
			{
				fDirectoryIndex=i;
				break;
			}
		}
		
		fWorkingDirectory[fDirectoryIndex+1]=0;
	}

	texDict.Clear();
	int type;
	int elementLineCount;

	double inp1,inp2,inp3,inp4,inp5;
	int linenumber=1;
	char fname[800];
	char str[800];
	int index,res;
	
	bgColor[0]=0;
	bgColor[1]=0;
	bgColor[2]=0;

	fontColor[0]=1;
	fontColor[1]=1;
	fontColor[2]=1;

	distractColor[0]=0;
	distractColor[1]=0;
	distractColor[2]=0;

	char linebuf[LINEBUFFERLIMIT];
	while(NULL!=fgets(linebuf,LINEBUFFERLIMIT,fp))
	{
		if( 1 > sscanf_s(linebuf,"%lf\t%lf\n",&inp1,&inp2) )
		{
			if(linebuf[0]=='\n')
				continue;
			else if(linebuf[0]=='#')
				continue;
			else
				break;
		}
		type=inp1;
		elementLineCount=inp2;
		linenumber++;
		switch(type)
		{
			case FULLSCREENOPTION:	
				bFullscreen=inp2;
				break;
			case COMPORTOPTION:	
				comport=inp2;
				break;
			case FONTSIZEOPTION:	
				fontsize=inp2;
				break;
			case TIMEROPTION:	
				bShowTimer=inp2;
				break;
			case LSLOPTION:
				bUseLSL = inp2;
				break;
			case LPTOPTION:
				bUseLPT = inp2;
				break;
			case LOGOPTION:
				autolog = inp2;
				break;
			case BGCOLOROPTION:	
				res=sscanf_s(linebuf,"%lf\t%lf\t%lf\t%lf",&inp1,&inp2,&inp3,&inp4);
				if(res!=4) {MessageBox(NULL,"BG COLOR ERROR",0,0); return 0;}
				else{
					bgColor[0]=inp2;
					bgColor[1]=inp3;
					bgColor[2]=inp4;
				}
				break;
			case FONTCOLOROPTION:	
				res=sscanf_s(linebuf,"%lf\t%lf\t%lf\t%lf",&inp1,&inp2,&inp3,&inp4);
				if(res!=4) {MessageBox(NULL,"FONT COLOR ERROR",0,0); return 0;}
				else{
					fontColor[0]=inp2;
					fontColor[1]=inp3;
					fontColor[2]=inp4;
				}
				break;
			case DISTRACTCOLOROPTION:	
				res=sscanf_s(linebuf,"%lf\t%lf\t%lf\t%lf",&inp1,&inp2,&inp3,&inp4);
				if(res!=4) {MessageBox(NULL,"DISTRACTION COLOR ERROR",0,0); return 0;}
				else{
					distractColor[0]=inp2;
					distractColor[1]=inp3;
					distractColor[2]=inp4;
				}
				break;
			case FRAMEDIVIDEROPTION:	
				frameDivider=inp2;
				break;
			case DSSTSYMBOLS:
				res=sscanf(linebuf,"%*f\t%s\n",str);
				if(res!=1) {MessageBox(NULL,"DIGIT SYMBOL ERROR",0,0); return 0;}
				else{
					for(int i=0;i<9;i++)
					{
						symbolList[i]=str[i];
					}
				}
				break;
			case TEXTURELIBRARY:	
				while(fscanf_s(fp,"\t%d\t",&index)==1)
				{
					if(index==-1) break;
					int n=4;
						strcpy(fname,"imgs\\");
						while(fname[n]!='\n' && fname[n]!=EOF && fname[n]!='\r')
						{
							n++;
							fread(&(fname[n]),1,1,fp);
						}
						linenumber++;
						fname[n]=0;	
					texDict.Add(index,fname);
				}
				linenumber++;
				break;
			case AUDIOLIBRARY:	
					while(fscanf_s(fp,"\t%d\t",&index)==1) //searches for /t AudioID /t
					{
						char soundSTR[256];
						if(index==-1) break;
						int n=-1;
						while( n<0 || (fname[n]!='\n' && fname[n]!=EOF && fname[n]!='\r'))
						{
							n++;
							fread(&(fname[n]),1,1,fp);
						}
						linenumber++;
						fname[n]=0; //terminates the string
						if(fname[n-1]==' ')  //adding a space skips loading
						{
							sprintf(soundSTR,"");
						}
						else
						{
							char fname2[600],fname3[600];
							char audiopath[600]="audio\\";

							strcat_s(audiopath,fname);
				
							if(CheckFile(fname))
								strcpy(soundSTR,fname);
							else if(CheckFile(audiopath))
								strcpy(soundSTR,audiopath);
							else
							{
								strcpy(fname2,fWorkingDirectory);
								strcat(fname2,audiopath);

								strcpy(fname3,fWorkingDirectory);
								strcat(fname3,fname);

								if(CheckFile(fname2))
									strcpy(soundSTR,fname2);
								else if(CheckFile(fname3))
									strcpy(soundSTR,fname3);
								else
								{
									sprintf(soundSTR,"");
								}
							}

							if(strlen(soundSTR)>2)
							{
								curAudioDict.Add(index,soundSTR);
							}
							else
							{
								char txt[250];
								sprintf(txt,"Audio '%s' not found!\nLine: %i", fname,linenumber);
								MessageBoxA(NULL,txt,0,0);
							}
						
						}
					}
					linenumber++;
				break;
			case MACROLIST:
				while(fscanf_s(fp,"%i\t",&index)==1)//check frameType
				{
					if(index>400)
					{
						char str[MACROLENGTH]="";
						char miniLine[800]="";
						

						char macroname[800]="";

						fgets(macroname,LINEBUFFERLIMIT,fp);
						
						while(NULL!=fgets(linebuf,LINEBUFFERLIMIT,fp))
						{
							if((linebuf[0]=='\t'&&linebuf[1]=='\t'&&linebuf[2]!='-')&&!(linebuf[0]=='-'&&linebuf[1]=='1'))
							{
								for (int i=2;i<strlen(linebuf);i++)
								{
									miniLine[i-2]=linebuf[i];
								}
								miniLine[strlen(linebuf)-2]=0;	//make sure new line is terminated
								strcat(str,miniLine);
							}
							else
								break;
						}
						if(strlen(str)>0)
							macroList[index-400]=macro(str);
					}
					if(index==-1)
						break;
				}
				break;
			case PRESENTATIONLIST:	
				while(fscanf_s(fp,"%i\t",&index)==1)//check frameType
				{
					if(index==-1)
					{
						break;
					}
					if(index==100)	//Starts timer function
					{
						
						fgets(linebuf,LINEBUFFERLIMIT,fp);

						sscanf(linebuf,"%lf",&inp1);

						AddFrame((int)index,0,inp1,"",100,0);
					}
					else if(index==101)	//skip to line
					{
						AddFrame((int)index,0,0,"",101,0);
					}
					else if(index>400)
					{
						index=index-400;
						fgets(linebuf,LINEBUFFERLIMIT,fp);
						if(macroList[index].initialized)
						{
							int numS,numD;
							
							sscanf(linebuf,"%i\t%i",&numD,&numS);
							double dArr[10];
							char* pstr;
							char* str;

							char* macroString;
							if(numS==macroList[index].numSVars&&numD==macroList[index].numDVars) //Double check to see if variables match
							{
								pstr=strtok(linebuf,"\t");	//remove first two tabs
								pstr=strtok(NULL,"\t");
								for (int i=0;i<numD;i++)
								{
									double b;
									pstr=strtok(NULL,"\t");
									sscanf(pstr,"%lf",&b);
									dArr[i]=b;
								}

								if(numS>0)
									str=strtok(NULL,"\t\n");
								else
									str="";

								macroString=macroList[index].fillIn(dArr,str);	//Pass agruments to fill in Macro

								if(strlen(macroString)>0) //if macro is filled in then parse like regular script
								{
									pstr=strtok(macroString,"\n"); //get first macro string
									while(pstr!=NULL)
									{
										res=sscanf_s(pstr,"%lf\t%lf\t%lf\t%lf\t%lf",&inp1,&inp2,&inp3,&inp4,&inp5);
										if(res!=5) {MessageBox(NULL,"ERROR",0,0); return 0;}
										else
										{
											int tCount=0;
											int i;
											for(i=0;i<strlen(pstr);i++)
											{
												if(pstr[i]=='\t')
													tCount++;
												if(tCount==5)
												{
													i++;
													break;
													
												}
											}
											if(strlen(pstr)-i>0)
											{
													str=&pstr[i];
											}
											else
												str="";

										AddFrame((int)inp1,(int)inp4,(double)inp2,str,(int)inp3,(int)inp5);
										pstr=strtok(NULL,"\n"); //get next Macro String
									}
									}
								}
							}
						}
					}
					else
					{
						res=fscanf_s(fp,"%lf\t%lf\t%lf\t%lf",&inp2,&inp3,&inp4,&inp5);
						if(res!=4) {MessageBox(NULL,"ERROR",0,0); return 0;}
						else linenumber++;
						{
							fgets(linebuf,LINEBUFFERLIMIT,fp);

							if (linebuf[0] == '\t')
							{
								int offset = 0;
								for (int i = 1; i < strlen(linebuf); i++)
								{
									if (linebuf[i] == '\\'&&i < strlen(linebuf) - 1 && linebuf[i + 1] == 'n')
									{
										str[i - 1 - offset] = '\n';
										i++;
										offset++;
									}
									else
										str[i-1-offset] = linebuf[i];
								}

								str[strlen(linebuf) - 1-offset] = 0;

							}
							else
								str[0] = 0;
							
							
							AddFrame(index,(int)inp4,(double)inp2,str,(int)inp3,(int)inp5);
						}
					}
					
				}
				linenumber++;
				break;
		}
	}
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

int LoadListFromNode(rapidxml::xml_node<> *pRoot);
int LoadListFromNode(rapidxml::xml_node<> *pRoot, int numArgs, int stringArgs, double dArgs[], char* strArgs[]);

int LoadScriptXML(char* filename)
{


		strcpy(fWorkingDirectory, filename);

		texDict = TextureDictionary();
		srand(GetQPC()*time(NULL)); //seed based on time
		FILE *fp = fopen(filename, "r");
		if (fp == NULL)
		{
			char message[800];
			sprintf(message, "Couldn't open Script file!\n%s", filename);
			MessageBoxA(NULL, message, NULL, NULL);
			autolog = false;
			return -1;
		}
		else
		{
			int fDirectoryIndex = 0;
			for (int i = strlen(filename); i > 0; i--)
			{
				if (filename[i] == '\\')
				{
					fDirectoryIndex = i;
					break;
				}
			}

			fWorkingDirectory[fDirectoryIndex + 1] = 0;
		}

		texDict.Clear();

		bgColor[0] = 0;
		bgColor[1] = 0;
		bgColor[2] = 0;

		fontColor[0] = 1;
		fontColor[1] = 1;
		fontColor[2] = 1;

		distractColor[0] = 0;
		distractColor[1] = 0;
		distractColor[2] = 0;

		using namespace rapidxml;

		try
		{

			std::ifstream file(filename);

			xml_document<> doc;
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();
			std::string content(buffer.str());
			doc.parse<0>(&content[0]);
			xml_node<> *pRoot = doc.first_node("dScript");

			if (!pRoot)
			{
				MessageBoxA(NULL, "Invalid dScriptX file! No dScript Node located", "ERROR", MB_OK | MB_ICONERROR | MB_TASKMODAL);
				return -1;
			}

			xml_node<> *pNode;

			// Load Options
			pNode = pRoot->first_node("Options");
			if(pNode)
			{
				for (pNode = pNode->first_node(); pNode!=NULL; pNode = pNode->next_sibling())
				{
					std::string strValue = pNode->name();
					xml_attribute<> *pAttr = pNode->first_attribute("value");
					char sStr[256] = "";


					switch (str2int(strValue.c_str()))
					{
					case str2int("Fullscreen"):
						if (pAttr == NULL)
							break;
						bFullscreen = 1 == atoi(pAttr->value());
						break;
					case str2int("COMport"):
						if (pAttr == NULL)
							break;
						comport = atoi(pAttr->value());
						break;
					case str2int("FontSize"):
						if (pAttr == NULL)
							break;
						fontsize = atoi(pAttr->value());
						break;
					case str2int("ShowTimer"):
						if (pAttr == NULL)
							break;
						bShowTimer = 1 == atoi(pAttr->value());
						break;
					case str2int("EnableLSL"):
						if (pAttr == NULL)
							break;
						bUseLSL = 1 == atoi(pAttr->value());
						break;
					case str2int("EnableLPT"):
						if (pAttr == NULL)
							break;
						bUseLPT = 1 == atoi(pAttr->value());
						break;
					case str2int("EnableLog"):
						if (pAttr == NULL)
							break;
						autolog = 1 == atoi(pAttr->value());
						break;
					case str2int("FrameDivider"):
						if (pAttr)
							frameDivider = atoi(pAttr->value());
						break;
					case str2int("BGColor"):
						pAttr = pNode->first_attribute("r");
						if (pAttr)
							bgColor[0] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("g");
						if (pAttr)
							bgColor[1] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("b");
						if (pAttr)
							bgColor[2] = atoi(pAttr->value());
						break;
					case str2int("FontColor"):
						pAttr = pNode->first_attribute("r");
						if (pAttr)
							fontColor[0] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("g");
						if (pAttr)
							fontColor[1] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("b");
						if (pAttr)
							fontColor[2] = atoi(pAttr->value());
						break;
					case str2int("DistractColor"):
						pAttr = pNode->first_attribute("r");
						if (pAttr)
							distractColor[0] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("g");
						if (pAttr)
							distractColor[1] = atoi(pAttr->value());
						pAttr = pNode->first_attribute("b");
						if (pAttr)
							distractColor[2] = atoi(pAttr->value());
						break;
					case str2int("dsstSymbols"):
						for (int i = 0; i < 9; i++)
						{
							sprintf(sStr, "s%i", i);
							pAttr = pNode->first_attribute(sStr);
							if (pAttr)
								symbolList[i] = atoi(pAttr->value());
							else
								symbolList[i] = 0;
						}

						break;


					}
				}
			 }

			char fPath[800] = "";

			// Load Images
			pNode = pRoot->first_node("ImageList");
			if (pNode)
			{
				for (pNode = pNode->first_node("Image"); pNode; pNode = pNode->next_sibling("Image"))
				{
					if (!pNode)
						break;

					xml_attribute<> *pAttr = pNode->first_attribute("file");
					xml_attribute<> *pAttrID = pNode->first_attribute("id");

					std::string fname;
					int index;

					if (pAttr != NULL&&pAttrID != NULL)
					{
						fname = pAttr->value();
						index = atoi(pAttrID->value());
						strcpy(fPath, fname.c_str());

						char fname2[600], fname3[600];
						char imageSTR[800] = "";
						char imgpath[600] = "imgs\\";

						strcat_s(imgpath, fPath);

						if (CheckFile(fPath))
							strcpy(imageSTR, fPath);
						else if (CheckFile(imgpath))
							strcpy(imageSTR, imgpath);
						else
						{
							strcpy(fname2, fWorkingDirectory);
							strcat(fname2, imgpath);

							strcpy(fname3, fWorkingDirectory);
							strcat(fname3, fPath);

							if (CheckFile(fname2))
								strcpy(imageSTR, fname2);
							else if (CheckFile(fname3))
								strcpy(imageSTR, fname3);
							else
							{
								sprintf(imageSTR, "");
							}
						}

						if (strlen(imageSTR) > 2)
						{
							texDict.Add(index, imageSTR);
						}
						else
						{
							char txt[250];
							sprintf(txt, "Image '%s' not found!\n", fPath);
							MessageBoxA(NULL, txt, 0, 0);
						}

						//texDict.Add(index, (char*) fname.c_str());
					}
				}
			}

			// Load Audio
			pNode = pRoot->first_node("AudioList");
			if(pNode)
			{
				for (pNode = pNode->first_node("Audio"); pNode; pNode = pNode->next_sibling("Audio"))
				{
					if (!pNode)
						break;
					xml_attribute<> *pAttr = pNode->first_attribute("file");
					xml_attribute<> *pAttrID = pNode->first_attribute("id");

					std::string fname;
					int index;

					if (pAttr != NULL&&pAttrID != NULL)
					{
						fname = pAttr->value();
						index = atoi(pAttrID->value());

						strcpy(fPath, fname.c_str());

						char fname2[600], fname3[600];
						char soundSTR[800] = "";
						char audiopath[600] = "audio\\";

						strcat_s(audiopath, fPath);

						if (CheckFile(fPath))
							strcpy(soundSTR, fPath);
						else if (CheckFile(audiopath))
							strcpy(soundSTR, audiopath);
						else
						{
							strcpy(fname2, fWorkingDirectory);
							strcat(fname2, audiopath);

							strcpy(fname3, fWorkingDirectory);
							strcat(fname3, fPath);

							if (CheckFile(fname2))
								strcpy(soundSTR, fname2);
							else if (CheckFile(fname3))
								strcpy(soundSTR, fname3);
							else
							{
								sprintf(soundSTR, "");
							}
						}

						if (strlen(soundSTR) > 2)
						{
							curAudioDict.Add(index, soundSTR);
						}
						else
						{
							char txt[250];
							sprintf(txt, "Audio '%s' not found!\n", fPath);
							MessageBoxA(NULL, txt, 0, 0);
						}

						//curAudioDict.Add(index, (char*)fname.c_str());
					}
				}
			}
			// Load MacroList
			pNode = pRoot->first_node("MacroList");
			if(pNode)
			{
				for (pNode = pNode->first_node("Macro"); pNode; pNode = pNode->next_sibling("Macro"))
				{
					if (pNode == NULL)
						break;
					xml_attribute<> *pAttr = pNode->first_attribute("id");

					if (pAttr)
					{
						int ID = atoi(pAttr->value());
						macroList[ID - 400] = macro(pNode, ID);
						//To do: Check for already exists or not
						//To do: switch macroList to linked List
					}


				}
			}

				// Load PresentationList
				if(LoadListFromNode(pRoot->first_node("PresentationList"))<0)
					MessageBoxA(NULL, "No Presentation List detected!", "DSX Reading Error!", MB_OK | MB_ICONERROR | MB_TASKMODAL);


				doc.clear();
		}

		catch (rapidxml::parse_error &e)
		{
			char * error = new char[512];
			char * text = new char[400];
			char* txt;
			txt=(char*) e.nextText();

			for (int i = 0; i < 100; i++)
			{
				if (txt[i] == 0)
				{
					text[i] = 0;
					//text[i + 1] = 0;
					break;
				}

				text[i] = txt[i];
			}
			
			sprintf(error, "%s\n", e.what());

			if (strlen(text) > 1)
			{
				sprintf(error, "%s\nError at:\t",e.what());

				strcat(error, text);
			}
			MessageBoxA(NULL, error, "DSX Reading Error!", MB_OK | MB_ICONERROR | MB_TASKMODAL);

			return -1;
		}
		
		return 1;
}

int LoadListFromNode(rapidxml::xml_node<> *pRoot)
{
	if (pRoot)
		return LoadListFromNode(pRoot, 0, 0, NULL, NULL);
	else
		return -1;
}

class FillInVals {
public:
	int numDVars;
	int numSVars;

	int dCounter;
	int sCounter;

	double dArgs[MAXMACROVARS];
	char* strArgs[MAXMACROVARS];



	rapidxml::xml_node<> *mNode;
	FillInVals(int numD, int numS, double dArgs_in[], char* strArgs_in[])
	{
		numDVars = numD;
		numSVars = numS;
		dCounter = 0;
		sCounter = 0;

		for (int i = 0; i < MAXMACROVARS; i++)
		{
			if (i < numD)
				dArgs[i] = dArgs_in[i];
			else
				dArgs[i] = 0;
		}
		
		for (int i = 0; i < numS&&i<MAXMACROVARS; i++)
		{
			strArgs[i] = new char[MAXFRAMESTRING];
			strcpy(strArgs[i], strArgs_in[i]);
		}

//		strArgs = (char[MAXMACROVARS][MAXFRAMESTRING]) strArgs_in;
	}

	char* FillIn(std::string str)
	{
		char macroStringFilled[MAXFRAMESTRING] = "";

		char macroStringCopy[MAXFRAMESTRING]="";

		strcpy(macroStringCopy, str.c_str());

		int i2 = 0;
		for (int i = 0; i<strlen(macroStringCopy)-1; i++)
		{
			if (macroStringCopy[i] == 0)
				break;
			else if (macroStringCopy[i] == '#')
			{
				if (macroStringCopy[i + 1] == 's'&&sCounter<numSVars)
				{
					macroStringCopy[i] = 0;
					sprintf(macroStringFilled, "%s%s%s", macroStringFilled, &macroStringCopy[i2], strArgs[sCounter]);
					macroStringCopy[i] = '#';
					sCounter++;
					i2 = i + 2;
				}
				else if (macroStringCopy[i + 1] == 'd'&&dCounter<numDVars)
				{
					macroStringCopy[i] = 0;
					sprintf(macroStringFilled, "%s%s%f", macroStringFilled, &macroStringCopy[i2], dArgs[dCounter]);
					macroStringCopy[i] = '#';
					dCounter++;
					
					i2 = i + 2;
				}
			}
		}

		if (i2 < strlen(macroStringCopy))
		{
			sprintf(macroStringFilled, "%s%s", macroStringFilled, &macroStringCopy[i2]);
		}
		return macroStringFilled;
	}
};

int LoadListFromNode(rapidxml::xml_node<> *pRoot,int numArgs, int stringArgs, double dArgs[],char* strArgs[])
{
	FillInVals f = FillInVals(numArgs, stringArgs, dArgs, strArgs);

	for (rapidxml::xml_node<> *pNode = pRoot->first_node(); pNode; pNode = pNode->next_sibling())
	{
		if (!pNode)
			break;

		rapidxml::xml_attribute<> *pAttr;
		std::string strValue = pNode->name();
		//xml_attribute<> *pAttr = pNode->first_attribute("value");

		std::string text;
		char txt[MAXFRAMESTRING];
		int type, imageID, audioID, displayTime, response;
		int id, numArgsTemp, stringArgsTemp;
		char argname[256] = "";
		double dArgsTemp[MAXMACROVARS];
		char* strArgsTemp[MAXMACROVARS];

		switch (str2int(strValue.c_str()))
		{
		case str2int("frame"):
			//bFullscreen = 1 == atoi(pAttr->value());
			type = 1; imageID = 0; audioID = 0; displayTime = 0; response = 0;
			text = "";

			pAttr = pNode->first_attribute("type");
			if (pAttr)
				type = atoi(f.FillIn(pAttr->value()));
			pAttr = pNode->first_attribute("imageID");
			if (pAttr)
				imageID = atoi(f.FillIn(pAttr->value()));
			pAttr = pNode->first_attribute("audioID");
			if (pAttr)
				audioID = atoi(f.FillIn(pAttr->value()));
			pAttr = pNode->first_attribute("response");
			if (pAttr)
				response = atoi(f.FillIn(pAttr->value()));
			pAttr = pNode->first_attribute("displayTime");
			if (pAttr)
				displayTime = atoi(f.FillIn(pAttr->value()));
			pAttr = pNode->first_attribute("text");
			if (pAttr)
				text = pAttr->value();
			strcpy(txt,f.FillIn(text));
			AddFrame(type, imageID, displayTime, txt, response, audioID);
			break;
		case str2int("macro"):
			numArgsTemp = 0; stringArgsTemp = 0;
			id = -1;
			pAttr = pNode->first_attribute("id");
			if (pAttr)
				id = atoi(pAttr->value());
			pAttr = pNode->first_attribute("numArgs");
			if (pAttr)
				numArgsTemp = atoi(pAttr->value());
			pAttr = pNode->first_attribute("strArgs");
			if (pAttr)
				stringArgsTemp = atoi(pAttr->value());
			for (int i = 0; i < numArgsTemp; i++)
			{
				sprintf(argname, "numArg%i", i+1);
				pAttr = pNode->first_attribute(argname);
				dArgsTemp[i] = -1;
				if (pAttr)
				{
					strcpy(txt, f.FillIn(pAttr->value()));
					sscanf(txt, "%lf", &dArgsTemp[i]);
				}
			}
			for (int i = 0; i < stringArgsTemp; i++)
			{
				sprintf(argname, "strArg%i", i+1);
				pAttr = pNode->first_attribute(argname);
				strArgsTemp[i] = new char[MAXFRAMESTRING];
				if (pAttr)
					strcpy(strArgsTemp[i], f.FillIn(pAttr->value()));
				else
					sprintf(strArgsTemp[i], "");
			}
			if (id > 0 && macroList[id - 400].initialized&&macroList[id-400].mNode!=NULL)
			{	
				//throw error if numArgs doesn't match internal macro aspect
				//To do: fill in macro string before processing
				LoadListFromNode(macroList[id - 400].mNode, numArgsTemp, stringArgsTemp, dArgsTemp,  strArgsTemp);
			}
			break;
		}


	}
	return -1;
}

void AddFrame(int FrameType,int texID,double timeout,char* str,int correctResponse,int audioID)
{
	char fStr[MAXFRAMESTRING] = "";
	char iStr[MAXFRAMESTRING];
	strcpy(iStr, &str[0]);
	if (strlen(iStr)>0)
	{
		int offset = 0;
		for (int i = 0; i < strlen(iStr); i++)
		{
			if (iStr[i] == '\\'&&i < strlen(iStr) - 1 && iStr[i + 1] == 'n')
			{
				fStr[i - offset] = '\n';
				i++;
				offset++;
			}
			else
				fStr[i - offset] = iStr[i];
		}

		fStr[strlen(iStr) - offset] = 0;

	}
	else
		fStr[0] = 0;

	pFrames[itemCount]=PresentationFrame(FrameType,texID,round(timeout),fStr,correctResponse,audioID);
	pFrames[itemCount].frameType=FrameType;
	itemCount++;
}

bool renderFrame=false;
void runMainLoop( int val )
{
    //Frame logic
    update();
	if(!finished)
	{
		if(renderFrame)
		{
			update();
			render();
		}

		//Run frame one more time
		glutTimerFunc( 1000 / SCREEN_FPS, runMainLoop, val );
	}
	else
	{
		update();
		CloseLog();
		
		KillFont();
		
		if(fAbort)
		{
			#if _DEBUG
				//
			#else
				std::cout<<"Script Aborted";
			#endif
		}
	}
}




