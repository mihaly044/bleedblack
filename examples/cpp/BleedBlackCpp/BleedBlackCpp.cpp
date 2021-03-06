// BleedBlackCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
// ReSharper disable CppClangTidyCppcoreguidelinesNarrowingConversions

#define _USE_MATH_DEFINES
#include <Windows.h>

//
// Functions for I/O
//
#include <iostream>

//
// The BleedBlack API
//
#include <ntddmou.h>
#include <bleedblack/bleedblack.h>

HANDLE g_hBleedBlack = nullptr;

void handle_exit()
{
	Bleedblack_Destroy(g_hBleedBlack);
}

auto main() -> int
{
	std::atexit(handle_exit);
	
	//
	// Set up connection with the Bleedblack driver
	//
	g_hBleedBlack = Bleedblack_Create();
	if(g_hBleedBlack == nullptr)
	{
		std::cout << "Error!!! Bleedblack_Create returned null" << std::endl;
		return -1;
	}

	auto status = Bleedblack_Init(g_hBleedBlack);
	if(status < 0)
	{
		std::cout << "Error!!! Bleedblack_Init has failed with code "
				  << std::hex << status << std::endl;
		return -2;
	}

	//
	// Move the cursor in a circle
	//
	std::cout << "Watch the cursor do a full circle." << std::endl;

	//
	// For simplicity, mouse events will execute in the context of our own process.
	//
	const auto myPid = GetCurrentProcessId();
	const auto radius = 15;
	for (auto i = 0.0; i < 2 * M_PI; i += 0.1)
	{
		int x = static_cast<int>(radius) * cos(i);
		int y = static_cast<int>(radius) * sin(i);

		Bleedblack_Move(g_hBleedBlack, myPid, x, y);
		Sleep(5);
	}

	//
	// Left click
	//
	std::cout << "Left click in 3 seconds ..." << std::endl;
	Sleep(3000);
	Bleedblack_Click(g_hBleedBlack, myPid, MOUSE_LEFT_BUTTON_DOWN, 30);

	//
	// Right click
	//
	std::cout << "Right click in 3 seconds ..." << std::endl;
	Sleep(3000);
	Bleedblack_Click(g_hBleedBlack, myPid, MOUSE_RIGHT_BUTTON_DOWN, 30);
	
	//
	// Make sure you call this function when your app quits
	//
	// Bleedblack_Destroy(g_hBleedBlack);
}
