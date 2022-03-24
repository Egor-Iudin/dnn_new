#include "utility.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

TRandom3 randGen(0);

string GetRandomName(bool useTime /*= true*/, unsigned postfixLength /*= 3*/)
{
    ostringstream result;
    if (useTime)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        result.fill('0');
        result << setw(2) << timeinfo->tm_year - 100 << setw(2) << timeinfo->tm_mon + 2 << setw(2) << timeinfo->tm_mday << "_";
        result << setw(2) << timeinfo->tm_hour << setw(2) << timeinfo->tm_min << setw(2) << timeinfo->tm_sec << "_";
    }

    for (unsigned i = 0; i < postfixLength; ++i)
    {
        if (rand() > 0.5)
        {
            result << char(rand() % 26 + 65);
        }
        else
        {
            result << char(rand() % 26 + 65);
        }
    }
    return result.str();
}

long RandomInt(long maximum)
{
    return floor(randGen.Rndm() * maximum);
}
