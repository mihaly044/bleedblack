#pragma once
#include <ntifs.h>
#include "ob.h"
#include "ps.h"

//
// This is going to be printed with debug messages
//
#define MODULE_NAME "bleedblack"

//
// Allow the use of the deprecated function ExAllocatePool
//
#pragma warning(disable: 4996)
