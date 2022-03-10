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

    //    std::cout<<"ttttt aaaa"<<result.str()<<std::endl;
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
    //    std::cout<<"ttttt bbbbb"<<result.str()<<std::endl;

    for (unsigned i = 0; i < postfixLength; ++i)
    {
        //	std::cout<<"ttttt uuuuu"<<result.str()<<std::endl;
        if (rand() > 0.5)
        {
            //	std::cout<<"ttttt kkk"<<result.str()<<std::endl;
            result << char(rand() % 26 + 65);
        }
        else
        {
            result << char(rand() % 26 + 65);
            // std::cout<<"ttttt mmmm"<<result.str()<<std::endl;
        }
    }

    //    std::cout<<"ttttt ssss"<<result.str()<<std::endl;
    return result.str();
}

long RandomInt(long maximum)
{
    return floor(randGen.Rndm() * maximum);
}
