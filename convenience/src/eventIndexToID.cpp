/**
 * \author Andrey Popov
 *
 * The file defines the starting point for a program which converts a list of indices of events in
 * ROOT files into a list of event ID. The calling syntax is
 *
 *   eventIndexToID indices.txt ids.txt file1.root file2.root ...
 *
 * The ROOT files must contain a tree called "Vars" and "run", "lumiSection", "event", each of type
 * ULong64_t. File ids.txt is created and must not exist.
 */

#include <EventID.hpp>
#include <TrainEventList.hpp>

#include <TFile.h>
#include <TTree.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>
#include <algorithm>

using namespace std;
using namespace boost::algorithm;

int main(int argc, char const **argv)
{
    // Check the input arguments
    if (argc < 4)
    {
        cout << "Usage: eventIndexToID indices.txt ids.txt file1.root file2.root ...\n";
        return 1;
    }

    if (not ends_with(argv[1], ".txt"))
    {
        cout << "Warning: source file \"" << argv[1] << "\" has an unexpected extension. "
             << "Do you really want to proceed? (Y/n) ";

        char c;
        cin >> c;

        if (c != 'Y')
            return 1;
    }

    if (not ends_with(argv[2], ".txt"))
    {
        cout << "Warning: target file \"" << argv[2] << "\" has an unexpected extension. "
             << "Do you really want to proceed? (Y/n) ";

        char c;
        cin >> c;

        if (c != 'Y')
            return 1;
    }

    if (boost::filesystem::exists(argv[2]))
    {
        cout << "Error: target file \"" << argv[2] << "\" already exists. Exit.\n";
        return 1;
    }

    // Read event indices from the source file
    TrainEventList eventList(argv[1], TrainEventList::Mode::Read);

    // Make a map to store a vector of event IDs for each of provided ROOT files
    map<string, vector<EventID>> eventIDsAllFiles;

    // Loop over the provided ROOT files
    for (int iFile = 3; iFile < argc; ++iFile)
    {
        string const fileName(argv[iFile]);
        string const shortFileName = fileName.substr(fileName.find_last_of('/') + 1);

        if (not eventList.ReadList(fileName))
            //^ Current ROOT file is not mentioned in the file with event indices
            continue;

        // Open the ROOT file and set the buffers to read the branches
        TFile srcFile(fileName.c_str());

        if (srcFile.IsZombie())
        {
            cout << "File \"" << fileName << "\" is not found or is not a valid ROOT file. Exit.\n";
            return 1;
        }

        unique_ptr<TTree> srcTree(dynamic_cast<TTree *>(srcFile.Get("Vars")));
        unsigned long const nEntries = srcTree->GetEntries();

        ULong64_t run, lumiSection, event;
        srcTree->SetBranchAddress("run", &run);
        srcTree->SetBranchAddress("lumiSection", &lumiSection);
        srcTree->SetBranchAddress("event", &event);

        auto &eventIDsCurFile = eventIDsAllFiles[shortFileName];
        eventIDsCurFile.reserve(eventList.GetReadEvents().size());

        // Loop over all the read indices and fill the vector with IDs
        for (auto const &ev : eventList.GetReadEvents())
        {
            if (ev >= nEntries)
                break;

            srcTree->GetEntry(ev);
            eventIDsCurFile.emplace_back(run, lumiSection, event);
        }

        // Vector with event IDs for the current file has been filled. Make sure it is ordered
        sort(eventIDsCurFile.begin(), eventIDsCurFile.end());
    }

    // Write the IDs to the target file
    ofstream fileStream(argv[2]);

    for (auto const &m : eventIDsAllFiles)
    {
        fileStream << "###########################################################################\n";
        fileStream << "# Name of the file\n"
                   << m.first << "\n\n";
        fileStream << "# Number of events\n"
                   << m.second.size() << "\n\n";
        fileStream << "# Event IDs\n";

        for (auto const &id : m.second)
            fileStream << id.Run() << ":" << id.LumiBlock() << ":" << id.Event() << '\n';

        fileStream << "\n\n\n";
        fileStream.flush();
    }

    fileStream.close();

    return 0;
}