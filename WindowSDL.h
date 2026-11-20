#pragma once
#include <GL/glew.h>
#include <SDL.h>
#include "Math/Maths.h"
#include "NormalisationCubeMap.h"
#include "graphic/LoadImages.h"
#include "graphic/FrameBufferObject.h"

class CWindowSDL
{
public:

	CWindowSDL(void);
	~CWindowSDL(void);

	SDL_Window *main_window;
	SDL_GLContext gl_context;
	SDL_Event event;
	int Width;
	int Height;

	
	void Init( int Width, int Height );
	void Run();
	void End();



	structImage img1,img2; GLuint tex, tex2;
	structImage img_normal; GLuint normal_map,normal_map2;
	GLuint normalisationCubeMap;



	cShader shader;
	
	GLint i1,i2;
	GLint loc1;
	GLint loc3;
	GLint loc4;
	GLint loc2;
	GLint fvAmb;
	GLint fvDiff;
	GLint fvSpec;
	GLint fvSpec_Pov;
	




	

protected:
	void Render1(float x,float y,float z,GLuint *t1,GLuint *t2);
	void Render2(float x,float y,float z,GLuint *t1,GLuint *t2);
	void Resize( int W, int H );
	void Render();
	void ReadLine(FILE * file,char *string);


};
