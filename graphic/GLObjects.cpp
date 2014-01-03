#include "GLObjects.h"
#include <windows.h>
#include <math.h>
#include <gl/gl.h>
void DrawLightSource(float x, float y,float d, float r)
{
	glPushMatrix();
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex2f(x,y);

	for (float i = 0.0f; i <= 3.14f * 2.0f; i += ((3.14f*2.0f)/ 10.0f))
	{
		glColor4f(0.0, 0.0, 0.0, 1.0);
		glVertex2f(r * cos(i) + x, r * sin(i) + y);
	}
	glVertex2f(x + r,y);
	glEnd();
	glPopMatrix();
}
void DrawShadows(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float d)
{
	glPushMatrix();

	glEnable(GL_DEPTH);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_NOTEQUAL);

	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glVertex3f(x1,y1,d);
	glVertex3f(x2,y2,d);
	glVertex3f(x3,y3,d);
	glVertex3f(x4,y4,d);
	glEnd();

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH);

	glPopMatrix();
}
void DrawSprite2D(float x,float y,float d,float size_X,float size_Y,float t_size)
{
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f);
	glVertex2f(x - size_X / 2,y - size_Y / 2); 
	 glTexCoord2f(0.0f,1.0f * t_size);
	glVertex2f(x - size_X / 2,y + size_Y / 2);
	glTexCoord2f(1.0f * t_size ,1.0f * t_size );
	glVertex2f(x + size_X / 2,y + size_Y / 2); 
	glTexCoord2f(1.0f * t_size,0.0f);
	glVertex2f(x + size_X / 2,y - size_Y / 2);  
	glEnd();
	glPopMatrix();
}
void DrawFlashlight(float x,float y, float x2,float y2,float x3,float y3,float r)
{
	glPushMatrix();
		glBegin(GL_TRIANGLES);
			glColor4f(1.0, 1.0, 1.0, 1.0);
			glVertex2f(x,y);
			glColor4f(0.0, 0.0, 0.0, 0.0);
			glVertex2f(x2,y2);
			glColor4f(0.0, 0.0, 0.0, 0.0);
			glVertex2f(x3,y3);
		glEnd();
	glPopMatrix();
}