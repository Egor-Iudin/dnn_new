#include "Logger.hpp"
#include "Config.hpp"
#include "InputProcessor.hpp"
#include "FBMWrapper.hpp"
#include "CodeMaker.hpp"
#include "PrintError.hpp"
#include <CreateEventList.hpp>
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>

using std::string;
using namespace logger;

int main(int argc, char **argv)
{
    // Check the usage
    if (argc != 2)
    {
        std::cerr << "Usage: bnn-hep configFile.\n";
        return 1;
    }

    // Use the name of the configuration file to create the logger
    string const cfgFileName(argv[1]);

    if (boost::iends_with(cfgFileName.c_str(), ".txt"))
    {
        std::cerr << "Confusing configuration file name. The extension should not be \"log\".\n";
        return 1;
    }

    size_t strIndex = cfgFileName.find_last_of('/');
    string const logFileName(cfgFileName.substr(strIndex + 1,
                                                cfgFileName.find_last_of('.') - strIndex - 1) +
                             ".log");

    // Create the logger
    Logger log(-1, -1, logFileName);
    log.PrintTimestamp();

    log << info(1) << "bnn-hep started." << eom;

    // Declare the object to parse the configuration
    Config config(cfgFileName, log);

    // Process the input files
    InputProcessor inputProcessor(log, config, true);

    InputProcessor inputProcessorExam(log, config, false);

    inputProcessor.SetExamFileName(inputProcessorExam.GetExamFileName());

    DNNCodeMaker DNNcoder(log, config, inputProcessor);

    //  Perform the training
    FBMWrapper fbm(log, config, inputProcessor);

    // Write the C++ file needed to apply the BNN
    CodeMaker coder(log, config, inputProcessor, fbm);

    PrintError printError(fbm.netFiles);

    // Everything is done
    log << info(1) << "The task is completed successfully." << eom;

    return 0;
}
