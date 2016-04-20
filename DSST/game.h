
#ifndef game_H
#define game_H



#define FREEGLUT_STATIC
#include "Libs/freeglut/GL/freeglut.h"

#include "TextureDictionary.h"
#include "PresentationFrame.h"
#include "AudioDictionary.h"
#include "libs/glmimg/glmimg.h"
#include <gl/gl.h>
#include "Libs/wglext.h"
#include "freetype.h"

//Screen constants
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int SCREEN_FPS = 2000;


bool initGL();
/*
Pre Condition:
 -A valid OpenGL context
Post Condition:
 -Initializes matrices and clear color
 -Reports to console if there was an OpenGL error
 -Returns false if there was an error in initialization
Side Effects:
 -Projection matrix is set to an orthographic matrix
 -Modelview matrix is set to identity matrix
 -Matrix mode is set to modelview
 -Clear color is set to black
*/

void update();
/*
Pre Condition:
 -None
Post Condition:
 -Does per frame logic
Side Effects:
 -None
*/

void render();
/*
Pre Condition:
 -A valid OpenGL context
 -Active modelview matrix
Post Condition:
 -Renders the scene
Side Effects:
 -Clears the color buffer
 -Swaps the front/back buffer
 -Sets matrix mode to modelview
 -Translates modelview matrix to the center of the default screen
 -Changes the current rendering color
*/

void handleKeys( unsigned char key, int x, int y );
/*
Pre Condition:
 -None
Post Condition:
 -Toggles the color mode when the user presses q
 -Cycles through different projection scales when the user presses e
Side Effects:
 -If the user presses e, the matrix mode is set to projection
*/

void handleMouse( int button, int state, int x, int y );
void handleMouseMove(int x, int y);

GLvoid glPrint(float x,float y,bool center,const char *fmt, ...);					// Custom GL "Print" Routine
GLvoid KillFont(GLvoid);	
extern void EventLog(int type,int instance, int state);
extern void EventLog(int type,int instance, int state,char* str);
extern DWORD GetQPC();
extern TextureDictionary texDict;
void drawCurrentFrame();
void close();
void LoadTextures();
int LoadTexture(char* fname, int index);
void forceFrameUpdate();
void drawDSSTimgFrame();
void BuildFont();
bool CheckFile(char* file);

#endif
