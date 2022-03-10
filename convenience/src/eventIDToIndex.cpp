/**
 * \author Andrey Popov
 *
 * The file defines the starting point for a program which converts a list of event IDs into indices
 * in ROOT files. The calling syntax is
 *
 *   eventIDToIndex ids.txt indices.txt file1.root file2.root ...
 *
 * The ROOT files must contain a tree called "Vars" and "run", "lumiSection", "event", each of type
 * ULong64_t. File indices.txt is created and must not exist.
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
#include <sstream>
#include <memory>
#include <algorithm>

using namespace std;
using namespace boost::algorithm;

int main(int argc, char const **argv)
{
    // Check the input arguments
    if (argc < 4)
    {
        cout << "Usage: eventIDToIndex ids.txt indices.txt file1.root file2.root ...\n";
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

    // Open the source file with event IDs into a map
    ifstream eventIDsFile(argv[1]);

    if (not eventIDsFile.good())
    {
        cout << "Error: cannot open source file \"" << argv[1] << "\". Exit.\n";
        return 1;
    }

    // Read it into a map
    map<string, vector<EventID>> eventIDsAllFiles;
    string line;
    bool stop = false;

    while (not stop)
    {
        // Read until a file name is found
        while (true)
        {
            getline(eventIDsFile, line);

            if (starts_with(line, "# Name of the file"))
                break;

            if (eventIDsFile.eof()) // no more files mentioned in eventIDsFile
            {
                stop = true;
                break;
            }
        }

        if (stop)
            // Skip the rest of the loop
            continue;

        // The next line is the file name
        getline(eventIDsFile, line);
        auto &eventIDsCurFile = eventIDsAllFiles[line];

        // Skip two lines and read the number of events
        for (unsigned i = 0; i < 3; ++i)
            getline(eventIDsFile, line);

        istringstream ist(line);
        unsigned long nEntries;
        ist >> nEntries;

        eventIDsCurFile.reserve(nEntries);

        // Skip two lines
        getline(eventIDsFile, line);
        getline(eventIDsFile, line);

        // Read IDs
        while (true)
        {
            getline(eventIDsFile, line);

            if (eventIDsFile.eof() or line.length() == 0)
                break;

            ist.clear();
            ist.str(line); // should contain three numbers separated by semicolon

            unsigned long run, lumiSection, event;
            ist >> run;
            ist.ignore();
            ist >> lumiSection;
            ist.ignore();
            ist >> event;

            eventIDsCurFile.emplace_back(run, lumiSection, event);
        }
    }

    eventIDsFile.close();

    // An object to write lists of indices
    TrainEventList trainList(argv[2], TrainEventList::Mode::Write);

    // Loop over the provided ROOT files and fill vectors with IDs
    for (int iFile = 3; iFile < argc; ++iFile)
    {
        string const fileName(argv[iFile]);
        string const shortFileName = fileName.substr(fileName.find_last_of('/') + 1);

        // Check if there event IDs available for the current file
        if (eventIDsAllFiles.find(shortFileName) == eventIDsAllFiles.end())
        {
            cout << "Warning: ROOT file \"" << fileName << "\" is not mentioned in \"" << argv[1] << "\" and is skipped.\n";
            continue;
        }

        // A short-cut
        auto const &eventIDsCurFile = eventIDsAllFiles[shortFileName];

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

        // Vector with indices to be filled
        vector<unsigned long> eventIndicesCurFile;
        eventIndicesCurFile.reserve(eventIDsCurFile.size());

        // Loop over the whole tree
        for (unsigned long ev = 0; ev < nEntries; ++ev)
        {
            srcTree->GetEntry(ev);

            if (find(eventIDsCurFile.begin(), eventIDsCurFile.end(),
                     EventID(run, lumiSection, event)) not_eq eventIDsCurFile.end())
                eventIndicesCurFile.push_back(ev);
        }

        // Write the IDs to the target file
        trainList.WriteList(shortFileName, eventIndicesCurFile.begin(), eventIndicesCurFile.end());
    }

    return 0;
}