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

auto main() -> int
{
	//
	// Set up connection with the Bleedblack driver
	//
	auto* bleedBlackHandle = Bleedblack_Create();
	if(bleedBlackHandle == nullptr)
	{
		std::cout << "Error!!! Bleedblack_Create returned null" << std::endl;
		return -1;
	}

	auto status = Bleedblack_Init(bleedBlackHandle);
	if(status < 0)
	{
		std::cout << "Error!!! Bleedblack_Init has failed with code "
				  << std::hex << status << std::endl;
		return -2;
	}

	//
	// Move the cursor in a circle
	//
	std::cout << "Watch the cursor do a full circle with the radius of 15 pixels." << std::endl;

	//
	// For simplicity, mouse events will execute in the context of our own process.
	//
	const auto myPid = GetCurrentProcessId();
	const auto radius = 15;
	for (auto i = 0.0; i < 2 * M_PI; i += 0.1)
	{
		int x = static_cast<int>(radius) * cos(i);
		int y = static_cast<int>(radius) * sin(i);

		Bleedblack_Move(bleedBlackHandle, myPid, x, y);
		Sleep(5);
	}

	//
	// Left click
	//
	std::cout << "Left click in 3 seconds ..." << std::endl;
	Sleep(3000);
	Bleedblack_Click(bleedBlackHandle, myPid, MOUSE_LEFT_BUTTON_DOWN, 30);

	//
	// Right click
	//
	std::cout << "Right click in 3 seconds ..." << std::endl;
	Sleep(3000);
	Bleedblack_Click(bleedBlackHandle, myPid, MOUSE_RIGHT_BUTTON_DOWN, 30);
	
	//
	// Make sure you call this function when your app quits
	//
	Bleedblack_Destroy(bleedBlackHandle);
}
