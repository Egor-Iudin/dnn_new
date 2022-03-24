#include <CreateEventList.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "utility.hpp"
using namespace std;

CreateEventList::CreateEventList(const string config_)
{
	vector<vector<int>> ListEventList;
	vector<int> eventsList;
	vector<string> Files;
	vector<int> FilesSizes;
	ifstream file(config_ + "_trainEvents.txt");
	string content;
	bool inting = false;
	while (file >> content)
	{
		if (content == "training")
		{
			inting = true;
			continue;
		}
		if (content == "###########################################################################")
		{
			inting = false;
			if (eventsList.size() > 0)
				ListEventList.push_back(eventsList);
			eventsList.clear();
		}
		if (inting == true)
		{
			eventsList.push_back(atoi(content.c_str()));
		}
		if (content == "file")
		{
			file >> content;
			Files.push_back(content);
		}
		if (content == "eventsSize")
		{
			file >> content;
			FilesSizes.push_back(atoi(content.c_str()));
		}
	}
	ListEventList.push_back(eventsList);

	vector<vector<int>> ListEventListToWrite;
	vector<int> eventsListToWrite;

	for (int Nlist = 0; Nlist < ListEventList.size(); Nlist++)
	{
		int Nevents = 0;
		int eventNumber = 0;

		vector<int> eventsnumberlist;
		int sampleLen = FilesSizes[Nlist];

		for (int i = 0; i < sampleLen; i++)
			eventsnumberlist.push_back(i);

		std::random_shuffle(eventsnumberlist.begin(), eventsnumberlist.end(), RandomInt);

		while (Nevents < ListEventList[Nlist].size())
		{
			if (std::find(ListEventList[Nlist].begin(), ListEventList[Nlist].end(), eventsnumberlist[eventNumber]) == ListEventList[Nlist].end())
			{
				eventsListToWrite.push_back(eventsnumberlist[eventNumber]);
				Nevents++;
			}
			eventNumber++;
		}
		std::sort(eventsListToWrite.begin(), eventsListToWrite.end());
		ListEventListToWrite.push_back(eventsListToWrite);
		eventsListToWrite.clear();
	}
	std::fstream fs;
	fs.open(config_ + "_ExamEvents.txt", std::fstream::out);

	for (int Nfiles = 0; Nfiles < Files.size(); Nfiles++)
	{
		int FileSize;
		int eventNumber = 0;
		fs << "###########################################################################\n";

		fs << "# Name of the file\n"
		   << Files[Nfiles].c_str() << "\n\n";
		fs << "# Sample eventsSize\n"
		   << FilesSizes[Nfiles] << "\n\n";
		fs << "# Number of events\n"
		   << ListEventListToWrite[Nfiles].size() << "\n\n";
		fs << "# Events tried for training\n";
		int Nevents = 0;
		int sleshInt = 0;
		for (int Nevents = 0; Nevents < ListEventListToWrite[Nfiles].size(); Nevents++)
		{
			fs << ListEventListToWrite[Nfiles][Nevents] << ' ';
			sleshInt++;
			if (sleshInt == 10)
			{
				fs << '\n';
				sleshInt = 0;
			}
		}
		fs << "\n\n\n";
	}
	fs.close();
}
