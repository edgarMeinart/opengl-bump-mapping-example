#pragma once
#include <stdio.h>
#include <GL/glew.h>


struct structImage
{
	structImage();
	~structImage();
	int bpp;
	int Height;
	int Width;
	unsigned char *Data;
	unsigned char *Data2;
};
structImage LoadBMPImage(const char *path);
struct ImagesList
{
	ImagesList(){ }
	~ImagesList(){ }
	int imgCount;
	structImage *img;
	GLuint *tex;

};
ImagesList LoadImages(const char *path,const char *folder_path);

struct Animation
{
	int State; // 1 - Play, 2 - Pause, 3 - Stop
	bool Load;
	int Frame;
	float num;
	float FPS;
	ImagesList *list;
	GLuint *tex;

	void Init();
	void Stop();
	void Pause();
	void Play();
	void Animate(float n);

	Animation();
	~Animation();
};
class Timer
{
public:
	int Time;
	int num;
	int GetTime(int fps);
	Timer();
	~Timer();
};
