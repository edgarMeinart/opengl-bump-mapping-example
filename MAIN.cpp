// LightEngine.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"

#include "WindowSDL.h"
CWindowSDL window;

int _tmain(int argc, _TCHAR* argv[])
{



	window.Init(700,700);
	window.Run();
	window.End();


	return 0;
}