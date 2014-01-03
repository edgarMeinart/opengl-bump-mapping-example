#include "FrameBufferObject.h"
#include <stdio.h>
cShader::cShader()
{

}
cShader::~cShader()
{

}
void cShader::LoadShader(char *f_file, char *v_file)
{	
	glewInit();
	char *vs;
	char *fs;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	vs = ReadFromFile(v_file);
	fs = ReadFromFile(f_file);

	const char *vv = vs;
	const char *ff = fs;

	glShaderSource(vertex_shader,1,&vv,NULL);
	glShaderSource(fragment_shader,1,&ff,NULL);
	free(vs);
	free(fs);

	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);

	program = glCreateProgram();


	glAttachShader(program,vertex_shader);
	glAttachShader(program,fragment_shader);

	glLinkProgram(program);

	
}
char* cShader::ReadFromFile(char *filename)
{
	FILE *fp;
	char *content = NULL;
	int count=0;
	if (filename != NULL) {
		fp = fopen(filename,"rt");
		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);
			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void cShader::enable()
{

	glUseProgram(program);
}

void cShader::disable()
{
	glUseProgram(0);
}
void cShader::printProgramInfoLog()
{
	int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
void cShader::printVertexShaderInfoLog()
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(vertex_shader, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
void cShader::printFragmentShaderInfoLog()
{
	  int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(fragment_shader, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}