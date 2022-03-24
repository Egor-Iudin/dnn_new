#include "TrainEventList.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <map>
#include "stdio.h"

using std::fstream;
using std::string;
using std::vector;

// Converts access mode from TrainEventList to its std::fstream analogy
std::ios_base::openmode dispatchAccessMode(TrainEventList::Mode mode)
{
    switch (mode)
    {
    case TrainEventList::Mode::Append:
        return std::ios_base::app;

    case TrainEventList::Mode::Read:
        return std::ios_base::in;

    default:
        return std::ios_base::out;
    }
}

TrainEventList::TrainEventList(string const &fileName_, Mode mode_ /*= Mode::Write*/) : fileName(fileName_), fileStream(fileName.c_str(), dispatchAccessMode(mode_)), mode(mode_),
                                                                                        listRead(false), nEventsRead(0)
{
    if (mode == Mode::Read and not fileStream.good())
    {
        std::ostringstream ost;
        ost << "TrainEventList::TrainEventList: Cannot open file \"" << fileName << "\" for reading.";
        throw std::runtime_error(ost.str());
    }
}

TrainEventList::~TrainEventList()
{
    fileStream.close();
}

void TrainEventList::WriteList(string const &sampleFileName,
                               vector<unsigned long>::const_iterator const &begin,
                               vector<unsigned long>::const_iterator const &end)
{
    if (mode == Mode::Read)
        throw std::logic_error("TrainEventList::WriteList: Cannot write to file as it was opened "
                               "for read access.");

    // Copy and sort the provided vector of indices
    vector<unsigned long> events(begin, end);
    std::sort(events.begin(), events.end());

    // Count duplicates
    unsigned nDuplicates = 0;
    unsigned long prevEvent = events.front();

    for (auto const &event : events)
        if (event == prevEvent)
            ++nDuplicates;
        else
            prevEvent = event;

    // The first event has been counted as a duplicate. Correct for it
    --nDuplicates;

    // Print a header for the current file
    string const shortFileName = sampleFileName.substr(sampleFileName.find_last_of('/') + 1);
    fileStream << "###########################################################################\n";
    fileStream << "# Name of the file\n"
               << shortFileName << "\n\n";
    fileStream << "# Sample eventsSize\n"
               << trannnigSize[sampleFileName] << "\n\n";
    fileStream << "# Number of events\n"
               << events.size() - nDuplicates << "\n\n";
    fileStream << "# Events tried for training\n";

    // Print the event indices. Filter out duplicates
    prevEvent = events.front();
    unsigned nEventWritten = 1;
    fileStream << prevEvent << " ";

    for (unsigned i = 1; i < events.size(); ++i)
    {
        auto const &event = events.at(i);

        if (event not_eq prevEvent)
        {
            fileStream << event << " ";
            prevEvent = event;
            ++nEventWritten;

            if (nEventWritten % 10 == 0)
                fileStream << '\n';
        }
    }

    fileStream << "\n\n\n";

    fileStream.flush();
}

bool TrainEventList::ReadList(string const &sampleFileName)
{
    listRead = false;

    if (mode != Mode::Read)
        throw std::logic_error("TrainEventList::ReadList: Cannot read from file as it was opened "
                               "for write access.");

    // Set the get pointer to the beginning of the file and reset possible EOF bit
    fileStream.seekg(0, std::ios::beg);
    fileStream.clear(); // sic!

    //  Find the line that contains the sample file name
    string const shortFileName = sampleFileName.substr(sampleFileName.find_last_of('/') + 1);
    string line;

    do
    {
        std::getline(fileStream, line);

        if (line.find(shortFileName) != string::npos)
            break;
    } while (not fileStream.eof());

    if (fileStream.eof())
        return false;

    // Skip two lines (an empty one and a comment)
    std::getline(fileStream, line);
    std::getline(fileStream, line);
    std::getline(fileStream, line);
    std::getline(fileStream, line);
    std::getline(fileStream, line);

    // Read the number of events
    fileStream >> nEventsRead;

    // Skip three lines (the ending of the current one, an empty line and a comment)
    for (unsigned i = 0; i < 3; ++i)
        std::getline(fileStream, line);

    // Read the list of events
    eventsRead.reserve(nEventsRead);
    unsigned long ev;

    for (unsigned long i = 0; i < nEventsRead; ++i)
    {
        fileStream >> ev;
        eventsRead.push_back(ev);
        std::ostringstream ost;
        ost << "ev= " << ev;
    }

    // All done
    listRead = true;
    return true;
}

vector<unsigned long> const &TrainEventList::GetReadEvents() const
{
    return eventsRead;
}

string const &TrainEventList::GetFileName() const
{
    return fileName;
}
