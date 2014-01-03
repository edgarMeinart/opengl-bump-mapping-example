#include "WindowSDL.h"

CWindowSDL::CWindowSDL(void)
{

}

CWindowSDL::~CWindowSDL(void)
{
	SDL_Quit();
}



void CWindowSDL::Init( int W, int H)
{
	sdlflags = SDL_HWSURFACE |SDL_OPENGL | SDL_DOUBLEBUF;
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Error: %s \n",SDL_GetError());
		exit(0);
		SDL_Quit();
	}else
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
		main_screen = SDL_SetVideoMode(W,H,32,sdlflags);
	}
	Width = W;
	Height = H;
	glewInit();

	shader.LoadShader("Data/fragment_shader.fs","Data/vertex_shader.vs");
	shader.printFragmentShaderInfoLog();
	shader.printVertexShaderInfoLog();
	shader.printProgramInfoLog();
		

	


	
	img1 = LoadBMPImage("Data/ground.bmp");
	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  img1.Width,  img1.Height, 0, GL_RGB, GL_UNSIGNED_BYTE,  img1.Data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		glBindTexture(GL_TEXTURE_2D, 0);

	img1 = LoadBMPImage("Data/Box.bmp");
	glGenTextures(1,&tex2);
	glBindTexture(GL_TEXTURE_2D, tex2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  img1.Width,  img1.Height, 0, GL_RGB, GL_UNSIGNED_BYTE,  img1.Data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		glBindTexture(GL_TEXTURE_2D, 0);
	
	img_normal = LoadBMPImage("Data/Normal Map.bmp");
	glGenTextures(1,&normal_map);
	glBindTexture(GL_TEXTURE_2D, normal_map);
	glUniform1i(i2,1);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  img_normal.Width,  img_normal.Height, 0, GL_RGB, GL_UNSIGNED_BYTE,  img_normal.Data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		glBindTexture(GL_TEXTURE_2D, 0);

	img_normal = LoadBMPImage("Data/Normal Map2.bmp");
	glGenTextures(1,&normal_map2);
	glBindTexture(GL_TEXTURE_2D, normal_map2);
	glUniform1i(i2,1);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  img_normal.Width,  img_normal.Height, 0, GL_RGB, GL_UNSIGNED_BYTE,  img_normal.Data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		glBindTexture(GL_TEXTURE_2D, 0);








		




}
float angle = 0.0;
void CWindowSDL::Run()
{
	bool quit = false;
	while (quit == false)
	{
		SDL_Delay(16);
		while(SDL_PollEvent(&event) )
		{
			switch(event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						quit = true;
					}
					if (event.key.keysym.sym == SDLK_SPACE)
					{
						angle += 0.5f;
						
					}
	
					break;
				case SDL_MOUSEMOTION: 
						int x,y;
						SDL_GetMouseState(&x,&y);


					break;
						
	
				
				case SDL_MOUSEBUTTONDOWN: 

					

					break;



				case SDL_VIDEORESIZE:
					int Width = event.resize.w;
					int Height = event.resize.h;

					if (!SDL_SetVideoMode(Width , Height,0,sdlflags))
					{
						printf("Resize failed! \n");
					}
					Resize(Width,Height);
					break;

			}
		}
		Resize(Width,Height);
		Render();

	}
}

void CWindowSDL::End()
{
	SDL_Quit();
}


void CWindowSDL::Resize(int Width, int Height)
{
	glViewport(0,0,Width,Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 10, 0, 10, -20.0f, 20.0f);
	glMatrixMode(GL_MODELVIEW);
	
	
}

void CWindowSDL::Render1(float x,float y,float z,GLuint *t1,GLuint *t2)
{
	float eyePos[3] = {0.0f,0.0f,100.0f};

	float sTangent[3] = {1.0f,0.0f,0.0f};
	float tTangent[3] = {0.0f,1.0f,0.0f};

	angle += 0.01;
	float LightPos[3] = {x ,y ,0.5};

	float fvAmbient[4] = {0.3f,0.3f,0.3f,1.0f};
	float fvDiffuse[4] = {0.8f,0.8f,0.8f,1.0f};
	float fvSpecular[4] = {0.4f,0.4f,0.4f,1.0f};
	float spec_pov = 1.0f;

	shader.enable();

	int loc1 = glGetUniformLocation(shader.program,"fvLightPosition");

	int loc2 = glGetUniformLocation(shader.program,"fvEyePosition");

	int loc3 = glGetAttribLocation(shader.program,"rm_Binormal");

	int loc4 = glGetAttribLocation(shader.program,"rm_Tangent");

	int fvAmb = glGetUniformLocation(shader.program,"fvAmbient");

	int fvDiff = glGetUniformLocation(shader.program,"fvDiffuse");

	int fvSpec = glGetUniformLocation(shader.program,"fvSpecular");

	int fvSpec_Pov = glGetUniformLocation(shader.program,"fSpecularPower");

	glUniform3fv(loc1,1,LightPos);

	glUniform3fv(loc2,1,eyePos);


	glUniform4fv(fvAmb,1,fvAmbient);


	glUniform4fv(fvDiff,1,fvDiffuse);


	glUniform4fv(fvSpec,1,fvSpecular);


	glUniform1f(fvSpec_Pov,spec_pov);

	glPushMatrix();
	
	glActiveTexture(GL_TEXTURE1);
	i1 = glGetUniformLocation(shader.program,"bumpMap");
	glUniform1i(i1,1);
	glBindTexture(GL_TEXTURE_2D, *(GLuint*) t1);
	glEnable(GL_TEXTURE_2D);


	glActiveTexture(GL_TEXTURE0);
	i2 = glGetUniformLocation(shader.program,"baseMap");
	glUniform1i(i2,0);
	glBindTexture(GL_TEXTURE_2D, *(GLuint*) t2);
	glEnable(GL_TEXTURE_2D);

	//glTranslatef(x,y,z);
	glNormal3f(0.0f,0.0f,1.0f);
	glBegin(GL_QUADS);
		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);	
		glTexCoord2f(0.0,1.0); 
		glVertex3f(-5.0f,-5.0f,0.0f);


		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(0.0,0.0); 
		glVertex3f(-5.0f,5.0f,0.0f);


		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(1.0,0.0); 
		glVertex3f(5.0f,5.0f,0.0f);

		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(1.0,1.0); 
		glVertex3f(5.0f,-5.0f,0.0f);
	glEnd();
	shader.disable();
	glPopMatrix();
}

void CWindowSDL::Render2(float x,float y,float z,GLuint *t1,GLuint *t2)
{
	float eyePos[3] = {0.0f,0.0f,100.0f};

	float sTangent[3] = {1.0f,0.0f,0.0f};
	float tTangent[3] = {0.0f,1.0f,0.0f};



	float fvAmbient[4] = {0.3f,0.3f,0.3f,1.0f};
	float fvDiffuse[4] = {0.8f,0.8f,0.8f,1.0f};
	float fvSpecular[4] = {0.4f,0.4f,0.4f,1.0f};
	float spec_pov = 1.0f;

	float LightPos[3] = {x ,y ,0.5};
	shader.enable();

	int loc1 = glGetUniformLocation(shader.program,"fvLightPosition");

	int loc2 = glGetUniformLocation(shader.program,"fvEyePosition");

	int loc3 = glGetAttribLocation(shader.program,"rm_Binormal");

	int loc4 = glGetAttribLocation(shader.program,"rm_Tangent");

	int fvAmb = glGetUniformLocation(shader.program,"fvAmbient");

	int fvDiff = glGetUniformLocation(shader.program,"fvDiffuse");

	int fvSpec = glGetUniformLocation(shader.program,"fvSpecular");

	int fvSpec_Pov = glGetUniformLocation(shader.program,"fSpecularPower");

	glUniform3fv(loc1,1,LightPos);

	glUniform3fv(loc2,1,eyePos);


	glUniform4fv(fvAmb,1,fvAmbient);


	glUniform4fv(fvDiff,1,fvDiffuse);


	glUniform4fv(fvSpec,1,fvSpecular);


	glUniform1f(fvSpec_Pov,spec_pov);

	glPushMatrix();
	
	glActiveTexture(GL_TEXTURE1);
	i1 = glGetUniformLocation(shader.program,"bumpMap");
	glUniform1i(i1,1);
	glBindTexture(GL_TEXTURE_2D, *(GLuint*) t1);
	glEnable(GL_TEXTURE_2D);


	glActiveTexture(GL_TEXTURE0);
	i2 = glGetUniformLocation(shader.program,"baseMap");
	glUniform1i(i2,0);
	glBindTexture(GL_TEXTURE_2D, *(GLuint*) t2);
	glEnable(GL_TEXTURE_2D);

	//glTranslatef(x,y,z);
	glNormal3f(0.0f,0.0f,1.0f);
	glBegin(GL_QUADS);
		
		
		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);	
		glTexCoord2f(0.0,1.0); 
		glVertex3f(-5.0f,-5.0f,0.0f);


		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(0.0,0.0); 
		glVertex3f(-5.0f,5.0f,0.0f);


		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(1.0,0.0); 
		glVertex3f(5.0f,5.0f,0.0f);

		glVertexAttrib3fv(loc3,tTangent);
		glVertexAttrib3fv(loc4,sTangent);
		glTexCoord2f(1.0,1.0); 
		glVertex3f(5.0f,-5.0f,0.0f);
	glEnd();
	shader.disable();
	glPopMatrix();


}
void CWindowSDL::Render()
{	

	glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);


	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	
	glPushMatrix();
	glTranslatef(5,5,0);
	//glMatrixMode(GL_TEXTURE);

	Render1(8,4,0.0,&normal_map,&tex);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	Render2(4,4,0.0,&normal_map,&tex);


	Render2(0,0,0.0,&normal_map,&tex);	

	//glBlendFunc(GL_ONE, GL_ONE);

	

	glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();



	SDL_GL_SwapBuffers();
}