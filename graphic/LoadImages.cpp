
#include "LoadImages.h"

//================Image=====================
structImage::structImage() 
{

}
structImage::~structImage() 
{

}
structImage LoadBMPImage(char *path)
{
	structImage img1;
	AUX_RGBImageRec *auxImage[1];
	auxImage[0] = auxDIBImageLoad(path);

	img1.Data = auxImage[0]->data;
	img1.Width = auxImage[0]->sizeX;
	img1.Height = auxImage[0]->sizeY;
	printf("Image: %s Width: %i Height: %i \n",path,img1.Width,img1.Height);
	return img1;
}
//===========================================

void ReadLine(FILE * file,char *string)
{
	do
	{
		fgets(string,255,file);
	}while ((string[0] == '/') || (string[0] == '\n'));
	return;
}
unsigned char *ConvertImagesColor(unsigned char *data,int w,int h) // Додумать!!!!!!!!!!!!!!!! 
{
	unsigned char *convertImage;
	convertImage = new unsigned char[w * h * 4];
	unsigned int *colors[4];

	for (int i = 0; i < 4; i++) colors[i] = new unsigned int[w * h];
		
	for (int pix = 0; pix < w * h; pix++)
	{
		colors[0][pix] = data[pix * 3];
		colors[1][pix] = data[pix * 3 + 1];
		colors[2][pix] = data[pix * 3 + 2];
	}
	for (int cpix = 0; cpix < w * h; cpix++)
	{
		convertImage[cpix * 4] = colors[0][cpix];
		convertImage[cpix * 4 + 1] = colors[1][cpix];
		convertImage[cpix * 4 + 2] = colors[2][cpix];
		convertImage[cpix * 4 + 3] = 255;

		if (convertImage[cpix * 4] < 50 && convertImage[cpix * 4 + 1] > 240 && 
			convertImage[cpix * 4 + 2] < 50)
		{
			convertImage[cpix * 4 + 3] = 0;
		}
	}
	for (int i = 0; i < 4; i++) delete [] colors[i];
	return convertImage;
}
ImagesList LoadImages(char *path,char *folder_path)
{
	
	FILE *file;
	ImagesList rList;

	char oneline[255];
	int imgCount;

	file = fopen(path,"rt");


	ReadLine(file,oneline);
	sscanf(oneline,"%i Images",&imgCount);
	rList.imgCount = imgCount;
	rList.img = new structImage[imgCount];
	
	glEnable(GL_TEXTURE_2D);
	rList.tex = new GLuint[imgCount];
	glGenTextures(rList.imgCount,rList.tex);


	for (int i = 0; i < imgCount; i++)
	{
		char buff[255];
		char buff2[255];
		unsigned char *dData;
		ReadLine(file,oneline);
		sscanf(oneline,"%s",&buff);
		sprintf(buff2,"%s%s",folder_path,buff);
		
		rList.img[i] = LoadBMPImage(buff2);	
		dData = rList.img[i].Data;
		rList.img[i].Data2 = ConvertImagesColor(dData,rList.img[i].Width,rList.img[i].Height);

		glBindTexture(GL_TEXTURE_2D, rList.tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  rList.img[i].Width,  rList.img[i].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,  rList.img[i].Data2);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	fclose(file);
	return rList;
}


/////////////////////////////////////////////


void Animation::Init()
{
	Animation::num = 0;	
}
Animation::Animation() 
{
	Load = false;
	Frame = 0;
	Animation::State = 1;
	Animation::num = 0;
}
Animation::~Animation() { }
void Animation::Animate(float n)
{
	if (Load == false)
	{
		Load = true;
	}

	if (State == 1)
	{	
		num += n;
			
		if (num >= 0.5)
		{
			Frame++;
			if (Frame == list->imgCount )
			{
				Frame = 0;

			}
			num = 0;
		}
	}
	if (State == 2)
	{
		num = n;
		if (num >= 0.5)
		{
			Frame = Frame;
			if (Frame == list->imgCount )
			{
				Frame = 0;
			}
			num = 0;
		}
	}
	if (State == 3)
	{
		Frame = 0;
	}
}
void Animation::Play()
{
	State = 1;
}
void Animation::Pause()
{
	State = 2;
}
void Animation::Stop()
{
	State = 3;
}

/////////////////////////////////////////////
Timer::Timer()
{

}
Timer::~Timer()
{
	Time = 0;
	num = 0; 

}
int Timer::GetTime(int fps)
{
	num++;
	if (num == fps)
	{
		Time++;
		num = 0;
	}
	return Time;
}