#include "FBMWrapper.hpp"
#include "utility.hpp"

#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <omp.h>

using namespace logger;
using namespace std;

FBMWrapper::FBMWrapper(Logger &log_, Config const &config_, InputProcessor const &inputProcessor_) : log(log_), config(config_), inputProcessor(inputProcessor_),
																									 FBMPath(config.GetFBMPath()), BNNFileName(config.GetBNNFileName())
{
	log << info(1) << "Training started. FBM binary file: \"" << config.GetBNNFileName() << "\"." << eom;

	// Save the neural network architecture for future use
	NNArchitecture.reserve(3);
	NNArchitecture.push_back(inputProcessor.GetDim());
	NNArchitecture.push_back(config.GetBNNNumberNeurons());
	NNArchitecture.push_back(1);

	// Perform training
	netFiles = TrainBNN();
	for (int i = 0; i < netFiles.size(); ++i)
	{
		// string ExamEventFile =	GetTaskName() + "_ExamEvents.txt";

		log << info(1) << "nEntries=" << netFiles[i].c_str() << eom; //= ExamEventFile;
	}
	// getchar();
	log << info(1) << "Training is completed11." << eom;
}

FBMWrapper::~FBMWrapper()
{
	if (not config.GetKeepTempFiles())
	{
		//       remove(BNNFileName.c_str());
		log << info(2) << "Temporary file \"" << BNNFileName << "\" removed." << eom;
	}
}

std::vector<std::string> FBMWrapper::TrainBNN() const
{
	ostringstream command; // stream to keep system commands
	string const &trainFileName = inputProcessor.GetTrainFileName();
	string examFileName = inputProcessor.examFile1;

	int bbin = config.GetBNNMCMCBurnIn();

	std::vector<std::string> listFiles;
	//#pragma omp parallel
	{
		string newFile = BNNFileName.substr(0, BNNFileName.size() - 4);
		// newFileName<<BNNFileName.erese<<
		log << info(1) << newFile << eom;
		ostringstream newFileName;
		int exitCode;

		//    #pragma omp parallel for private(command)
		for (int iter = 0; iter < config.GetBNNMCMCIterations(); iter++)
		{
			// ostringstream newFileName;
			newFile = BNNFileName.substr(0, BNNFileName.size() - 4);
			newFileName.str("");
			newFileName << newFile << "_" << iter + 1 << ".net";
			log << info(1) << newFileName.str().c_str() << eom;
			newFile = newFileName.str().c_str();

			// Define the network
			command.str("");
			command << FBMPath << "net-spec " << newFile << " " << inputProcessor.GetDim() << " " << config.GetBNNNumberNeurons() << " 1 / " << config.GetBNNHyperparameters();
			log << info(1) << command.str().c_str() << eom;
			exitCode = system(command.str().c_str());
			// getchar();
			if (exitCode != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Reset the random seed
			command.str("");
			command << FBMPath << "rand-seed " << newFile << " " << RandomInt(32767);
			log << info(1) << command.str().c_str() << eom;
			exitCode = system(command.str().c_str());
			// getchar();

			if (exitCode != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Define the model
			command.str("");
			command << FBMPath << "model-spec " << newFile << " binary";
			log << info(1) << command.str().c_str() << eom;
			exitCode = system(command.str().c_str());
			// getchar();

			if (exitCode != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Define the training data
			command.str("");
			command << FBMPath << "data-spec " << newFile << " " << inputProcessor.GetDim() << " 1 2 / " << trainFileName << ":/Vars";

			for (unsigned i = 0; i < inputProcessor.GetDim(); ++i)
				// First two branches contain targets and weights, branch indices start from 1
				command << "," << i + 4;

			// Targets (they are written in the first branch)
			command << " " << trainFileName << ":/Vars,1";

			// Weights (they are written in the second branch, they are tacken as is)
			command << " weights=" << trainFileName << ":/Vars,2 rescale_weights=0";

			// Redirect the output not the litter the log
			command << "  " << examFileName << ":/Vars";

			for (unsigned i = 0; i < inputProcessor.GetDim(); ++i)
				// First two branches contain targets and weights, branch indices start from 1
				command << "," << i + 4;

			// Targets (they are written in the first branch)
			command << " " << examFileName << ":/Vars,1";

			command << " &> /dev/null";

			// No transformation of the variables is specified in data-spec, i.e. they are tacken as is
			log << info(1) << command.str().c_str() << eom;
			exitCode = system(command.str().c_str());
			// getchar();

			if (exitCode != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Generate the initial neural network

			double randv = rand() % 10;
			log << info(1) << randv << eom;
			// getchar();
			double fix = randv + 0.2;
			log << info(1) << fix << eom;
			//#pragma omp critical
			//{
			listFiles.push_back(newFile);
			//}
			command.str("");
			command << FBMPath << "net-gen " << newFile << " " << config.GetBNNGenerationParameters();

			log << info(1) << command.str().c_str() << eom;
			//	getchar();

			exitCode = system(command.str().c_str());

			if (exitCode != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Treat the first training iteration in a special way
		}

// getchar();
#pragma omp parallel for
		for (int iter = 0; iter < config.GetBNNMCMCIterations(); iter++)
		{
			// #pragma omp barrier

			int exitCode1;
			ostringstream newFileName1;
			ostringstream command1;
			auto MCMCParams = config.GetBNNMCMCParameters();
			string newFile1 = BNNFileName.substr(0, BNNFileName.size() - 4);

			newFileName1.str("");
			newFileName1 << newFile1 << "_" << iter + 1 << ".net";
			cout << "info" << newFileName1.str().c_str() << endl;
			newFile1 = newFileName1.str().c_str();

			command1.str("");
			command1 << FBMPath << "mc-spec " << newFile1 << " " << MCMCParams.first << "; ";
			command1 << FBMPath << "net-mc " << newFile1 << " "
					 << " 1";
			cout << "info" << command.str().c_str() << endl;
			// getchar();
			exitCode1 = system(command1.str().c_str());
			// getchar();

			if (exitCode1 != 0)
			{
				log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}

			// Perform the training
			command1.str("");
			command1 << FBMPath << "mc-spec " << newFile1 << " " << MCMCParams.second << "; ";
			command1 << FBMPath << "net-mc " << newFile1 << " " << bbin;

			cout << "info" << command1.str().c_str() << endl;
			exitCode1 = system(command1.str().c_str());
			// command << FBMPath << "net-display " << newFile1 << " " << config.GetBNNMCMCBurnIn();
			// getchar();
			//	getchar();

			if (exitCode1 != 0)
			{
				// log << critical << "\"" << command.str() << "\" terminated with an error." << eom;
				exit(1);
			}
			FILE *fp;
			fp = fopen((newFile1 + ".log").c_str(), "a+");
			fprintf(fp, "End File \n");
			fclose(fp);
		}
#pragma omp taskwait
		// getchar();
	}

	return listFiles;
}

void FBMWrapper::ErrorWrongOutput(string const &command) const
{
	log << error << "The output of \"" << command << "\" is badly formatted." << eom;
	exit(1);
}

NeuralNetwork FBMWrapper::ReadNN(unsigned index) const
{

	ostringstream command;
	stringstream output; // this will store the output of the command
	string line;

	std::string newFile = BNNFileName.substr(0, BNNFileName.size() - 4);
	// newFileName<<BNNFileName.erese<<
	log << info(1) << newFile << eom;
	ostringstream newFileName;
	newFileName << newFile << "_" << index << ".net";
	log << info(1) << newFileName.str().c_str() << eom;
	newFile = newFileName.str().c_str();

	// Read the NN with the given index
	command << FBMPath << "net-display -p " << newFile << " " << config.GetBNNMCMCBurnIn();

	// Execute the command
	FILE *pipe = popen(command.str().c_str(), "r");

	if (!pipe)
	{
		log << error << "\"" << command.str() << "\" terminated with an error." << eom;
		exit(1);
	}

	// Read the command's output
	char buffer[128];

	while (fgets(buffer, 128, pipe))
		output << buffer;

	pclose(pipe);

	// Create a neural network
	NeuralNetwork nn(NNArchitecture);

	// Parse the output
	// First, skip the header
	getline(output, line);
	getline(output, line);
	getline(output, line);

	// Loop over the layers (all but one)
	for (unsigned l = 1; l < NNArchitecture.size(); ++l)
	{
		getline(output, line);
		// getchar();
		if (line.find("Weights") == string::npos)
			ErrorWrongOutput(command.str());

		getline(output, line); // skip an empty line

		// Loop over the groups corresponding to the nodes in the previous layer
		for (unsigned np = 0; np < NNArchitecture.at(l - 1); ++np)
		{
			// Loop over the current layer
			for (unsigned n = 0; n < NNArchitecture.at(l); ++n)
			{
				double w;
				output >> w;

				nn.GetWeight(l, n, np) = w;
			}

			getline(output, line); // skip an empty line
		}

		getline(output, line); // skip an empty line
		getline(output, line);
		// getchar();
		if (line.find("Biases") == string::npos)
			ErrorWrongOutput(command.str());

		getline(output, line); // skip an empty line

		// Loop over the nodes in the current layer
		for (unsigned n = 0; n < NNArchitecture.at(l); ++n)
		{
			double b;
			output >> b;

			nn.GetBias(l, n) = b;
		}

		getline(output, line); // skip an empty line
		getline(output, line); // skip an empty line
	}

	return nn;
}
