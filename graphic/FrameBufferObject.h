#pragma once


#include <GL/glew.h>

class FrameBufferObject
{
public:
	GLuint texId;
	GLuint fboId;
	GLuint rboId;
	bool fboInit;
	GLint Width;
	GLint Height;
	GLint WWidth;
	GLint WHeight;
public:
	FrameBufferObject(void);
	~FrameBufferObject(void);
	void Setsize(GLint w,GLint h) { Width = w; Height = h;}
	void InitFBO(int W,int H);
	void fboBegin();
	void fboEnd();
};

class cShader
{
public:
	cShader();
	~cShader();

	GLuint vertex_shader; //Verwinnij wejder
	GLuint fragment_shader; // Fragmentij wejder
	GLuint program; // programma

	
	void LoadShader(const char *f_file,const char *v_file);

	void enable();
	void disable();
	void printProgramInfoLog();
	void printVertexShaderInfoLog();
	void printFragmentShaderInfoLog();
private:
	char *ReadFromFile(const char *filename);


};
