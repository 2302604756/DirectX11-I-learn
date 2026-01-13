/******************************************************************************************
*	Chili Direct3D Engine																  *
*	Copyright 2018 PlanetChili <http://www.planetchili.net>								  *
*																						  *
*	This file is part of Chili Direct3D Engine.											  *
*																						  *
*	Chili Direct3D Engine is free software: you can redistribute it and/or modify		  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili Direct3D Engine is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili Direct3D Engine.  If not, see <http://www.gnu.org/licenses/>.    *
******************************************************************************************/
#include "App.h"
#include <sstream>
using namespace std;

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow )
{
	if (FAILED(CoInitialize(nullptr)))
	{
		MessageBox(nullptr, "COM Initialization Failed", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	try {		
		return App{}.Go();
	}
	catch (const Window::HrException& e)
	{
		MessageBoxA(nullptr, e.what(),
			e.GetType(), MB_OK | MB_ICONERROR);
	}
	catch (const ChiliException& e)
	{
		MessageBoxA(nullptr, e.what(),
			e.GetType(), MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		MessageBoxA(nullptr,
			"Unknown Exception",
			"Fatal Error", MB_OK | MB_ICONERROR);
	}
	return 0;
}