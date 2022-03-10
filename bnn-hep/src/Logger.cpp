#include "Logger.hpp"

#include <ctime>

using namespace logger;

Logger::Logger(unsigned stdVerbLevel_) : stdVerbLevel(stdVerbLevel_),
                                         fileVerbLevel(0),
                                         file(nullptr),
                                         curMessageClass(MessageClass::Undefined),
                                         curVerbosity(0),
                                         printTimestamp(false)
{
    textMessageTypes[MessageClass::Info] = "INFO";
    textMessageTypes[MessageClass::Warning] = "WARNING";
    textMessageTypes[MessageClass::Error] = "ERROR";
    textMessageTypes[MessageClass::CriticalError] = "CRITICAL ERROR";
    textMessageTypes[MessageClass::Undefined] = "UNDEFINED";
}

Logger::Logger(unsigned stdVerbLevel_, unsigned fileVerbLevel_, std::string const &fileName) : stdVerbLevel(stdVerbLevel_),
                                                                                               fileVerbLevel(fileVerbLevel_),
                                                                                               file(new std::ofstream(fileName)),
                                                                                               curMessageClass(MessageClass::Undefined),
                                                                                               curVerbosity(0),
                                                                                               printTimestamp(false)
{
    textMessageTypes[MessageClass::Info] = "INFO";
    textMessageTypes[MessageClass::Warning] = "WARNING";
    textMessageTypes[MessageClass::Error] = "ERROR";
    textMessageTypes[MessageClass::CriticalError] = "CRITICAL ERROR";
    textMessageTypes[MessageClass::Undefined] = "UNDEFINED";
}
// Looks like constructor delegation is not supported in GCC 4.6 =(

Logger::~Logger()
{
    // if (file)
    //     file->close();

    // delete file;
}

Logger &Logger::operator<<(EndOfMessage__ (*)())
{
    // Write the new line symbols
    if (curVerbosity < stdVerbLevel)
    {
        if (curMessageClass == MessageClass::Error or
            curMessageClass == MessageClass::CriticalError)
            std::cerr << '\n';
        else
            std::cout << '\n';
    }

    if (curVerbosity < fileVerbLevel and file)
    {
        *file << '\n';
        file->flush();
    }

    // Set the message type and verbosity to the default values
    curMessageClass = MessageClass::Undefined;
    curVerbosity = 0;

    return *this;
}

Logger &Logger::operator<<(MessageType__ type)
{
    // Set the properties of the current message
    curMessageClass = type.type;
    curVerbosity = type.verbosity;

    PrintHeader();

    return *this;
}

Logger &Logger::operator<<(MessageType__ (*fp)(unsigned))
{
    // Evaluate the manipulator (which is implemented as a function) at the default verbosity
    MessageType__ type = fp(0);

    curMessageClass = type.type;
    curVerbosity = type.verbosity;

    PrintHeader();

    return *this;
}

void Logger::PrintTimestamp(bool on /*= true*/)
{
    printTimestamp = on;
}

void Logger::SetStdVerbosity(unsigned stdVerbLevel_)
{
    stdVerbLevel = stdVerbLevel_;
}

void Logger::SetFileVerbosity(unsigned fileVerbLevel_)
{
    fileVerbLevel = fileVerbLevel_;
}

void Logger::PrintHeader()
{
    if (curMessageClass == MessageClass::Undefined)
        throw std::logic_error("The type of a message for logging was not specified");

    // Print the type
    if (curVerbosity < stdVerbLevel)
    {
        if (curMessageClass == MessageClass::Error or
            curMessageClass == MessageClass::CriticalError)
            std::cerr << textMessageTypes[curMessageClass] << ": ";
        else
            std::cout << textMessageTypes[curMessageClass] << ": ";
    }

    if (curVerbosity < fileVerbLevel and file)
    {
        *file << "[" << textMessageTypes[curMessageClass] << "]";

        // Print the timestamp
        if (printTimestamp)
        {
            std::time_t rawtime;
            struct std::tm *timeinfo;

            std::time(&rawtime);
            timeinfo = std::localtime(&rawtime);

            *file << '\t' << std::asctime(timeinfo); // it adds a newline
        }
        else
            *file << ' ';
    }
}
