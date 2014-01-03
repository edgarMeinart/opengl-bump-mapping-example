
#include "FrameBufferObject.h"

FrameBufferObject::FrameBufferObject(void)
{
	
fboId = 0;
texId = 0;
rboId = 0;
}

FrameBufferObject::~FrameBufferObject(void)
{
	glDeleteTextures(1,&texId);
	glDeleteFramebuffers(1,&fboId);
	glDeleteRenderbuffers(1,&rboId);
}

void FrameBufferObject::InitFBO(int W,int H)
{
		glewInit();
		WWidth = W;
		WHeight = H;

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 700 , 700 , 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glBindTexture(GL_TEXTURE_2D,0);

	
		


		glGenRenderbuffers(1,&rboId);
		glBindRenderbuffer(GL_RENDERBUFFER,rboId);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,700,700);
		glBindRenderbuffer(GL_RENDERBUFFER,0);


		glGenFramebuffers(1,&fboId);
		glBindFramebuffer(GL_FRAMEBUFFER,fboId);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texId,0);
		

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
void FrameBufferObject::fboBegin()
{
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	glClearColor(0,0,0,0.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport(0,0,700,700);
}
void FrameBufferObject::fboEnd()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

