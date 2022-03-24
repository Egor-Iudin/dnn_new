/**
 * \author Andrey Popov
 *
 * The module defines a set of auxiliary functions.
 */

#pragma once

#include <TRandom3.h>
#include <string>

/// Global random generator
extern TRandom3 randGen;

/// Generates random names
std::string GetRandomName(bool useTime = true, unsigned postfixLength = 3);

/// Random generator for std::random_shuffle
long RandomInt(long maximum);
