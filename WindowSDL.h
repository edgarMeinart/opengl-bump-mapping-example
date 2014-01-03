#pragma once
#include <SDL.h>
#include <gl/Glew.h>
#include <gl/gl.h>
#include "math/maths.h"
#include "Normalisation Cube Map.h"
#include "graphic/LoadImages.h"
#include "graphic/FrameBufferObject.h"

class CWindowSDL
{
public:

	CWindowSDL(void);
	~CWindowSDL(void);

	SDL_Surface *main_screen;
	SDL_Event event;
	int sdlflags;
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
