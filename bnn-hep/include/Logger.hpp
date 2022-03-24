/**
 * \author Anrey Popov
 *
 * The header file for a simple logger.
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <map>

namespace logger
{

    // Forward declarations
    struct EndOfMessage__;
    struct MessageType__;

    /**
     * \brief A simple logger class
     *
     * The class logs messages of different level of verbosity and severity types (info, warning,
     * error, critical error). The messages are printed to stdout (info and warning) and stderror
     * (error and critical error) and also to a specified log file. The info and warning messages
     * can be filtered by verbosity. The machinery is based on the overloaded left shift operator,
     * all the classes with this operator overloaded for std::ostream can be printed out. The needed
     * manipulators to describe the type and verbosity of the following message and the end of message
     * are provided.
     */
    class Logger
    {
    public:
        /// Supported types of messages
        enum class MessageClass : unsigned
        {
            Undefined,
            Info,
            Warning,
            Error,
            CriticalError
        };

    public:
        /**
         * \brief Constructor not supporting output to a file
         *
         * Constructs an object that does not support the output to a file.
         * \param verbLevel The verbosity level of the logger used to filter the information
         *  messages. If an information or warning message has a larger  verbosity level it is
         *  ignored. The error messages have the lowest possible verbosity (i.e. the highest rate
         *  of printing).
         */
        Logger(unsigned stdVerbLevel_);

        /**
         * \brief Constructor supporting output to a file
         *
         * Constructs a logger object that puts the messages both to stdout/stderr and to the
         * specified file. The verbosity levels for information and warning messages may be
         * different in the two cases.
         * \param stdVerdLevel_ The verbosity level used to filter the  informaion and warning
         *  messages when printing to stdout.
         * \param fileVerbLevel_ The verbosity level used to filer the  information and warning
         *  messages when printing to file.
         * \param fileName The name of the log file. It is recreated.
         * \note The user can switch off the output to stdout/stderr setting stdVerbLevel_ = 0.
         * Verbosity level (-1) means that all the messages are printed.
         */
        Logger(unsigned stdVerbLevel_, unsigned fileVerbLevel_, std::string const &fileName);

        /// Destructor
        ~Logger();

        /// The copy constructor is not allowed to be used
        Logger(Logger const &) = delete;
        /// The assignment operator is not allowed to be used
        Logger const &operator=(Logger const &) = delete;

    public:
        /**
         * \brief Marks the end of the current message
         *
         * The overloaded version of shift operator to mark the end of the current message. The
         * input parameter is a pointer to a function returning an object of type EndOfMessage__;
         * only signature is important, thefore the name of the argument is omitted.
         */
        Logger &operator<<(EndOfMessage__ (*)());

        /**
         * \brief Handles the manipulators
         *
         * This version of the shift operator handles the manipulators that control the type and
         * verbosity of the current message.
         */
        Logger &operator<<(MessageType__ type);

        /**
         * \brief Handles the manipulators that take no arguments
         *
         * This version of the shift operator handles the manipulators that take no arguements and
         * control the type and verbosity of the current message.
         */
        Logger &operator<<(MessageType__ (*fp)(unsigned));

        /**
         * \brief Performs the actual output of the message
         *
         * The method adds the specified message to the relevant stream(s).
         */
        template <typename T>
        Logger &operator<<(T const &msg)
        {
            // Check if the message type is defined
            if (curMessageClass == MessageClass::Undefined)
                throw std::logic_error("The type of a message for logging was not specified");

            if (curVerbosity < stdVerbLevel)
            {
                if (curMessageClass == MessageClass::Error or
                    curMessageClass == MessageClass::CriticalError)
                    std::cerr << msg;
                else
                    std::cout << msg;
            }

            if (curVerbosity < fileVerbLevel and file)
                *file << msg;

            return *this;
        }

        /**
         * \brief Requests the timestamps
         *
         * When the switch is set the messages in the log file are accompanied by the timestamps.
         */
        void PrintTimestamp(bool on = true);

        /// Modifies verbosity for stdout/stderr
        void SetStdVerbosity(unsigned stdVerbLevel_);

        /// Modifies verbosity for file
        void SetFileVerbosity(unsigned fileVerbLevel_);

    private:
        /**
         * \brief Prints the header for the message
         *
         * The message header contains the text code describing the type of the message (info,
         * error, etc.) and, optionally, the timestamp.
         */
        void PrintHeader();

    private:
        /// Verbosity level for printing to stdout/stderror
        unsigned stdVerbLevel;
        /// Verbosity level for printing to file
        unsigned fileVerbLevel;
        /// The log file object
        std::ofstream *file;
        /// The type of current message
        MessageClass curMessageClass;
        /// Verbosity of the current message
        unsigned curVerbosity;
        /// Indicates whether the timestamps should be printed
        bool printTimestamp;
        /// The text representation of the message classes
        std::map<MessageClass, std::string> textMessageTypes;
    };

    /// An instance of this structure is returned by the eom manipulator to indicate the end of message
    struct EndOfMessage__
    {
    };

    /**
     * Instances of this structure are returned by the manipulators to indicate the type and verbosity
     * of the following message.
     */
    struct MessageType__
    {
        Logger::MessageClass type;
        unsigned verbosity;
    };

    /// An instrance of this structure
    inline EndOfMessage__ eom()
    {
        return EndOfMessage__{};
    }

    /// Manipulator to produce the information messages
    inline MessageType__ info(unsigned verbosity = 0)
    {
        return MessageType__{Logger::MessageClass::Info, verbosity};
    }

    /// Manipulator to produce the warning messages
    inline MessageType__ warning(unsigned verbosity = 0)
    {
        return MessageType__{Logger::MessageClass::Warning, verbosity};
    }

    /// Manipulator to produce the error messages. Verbosity is always set to zero
    inline MessageType__ error(unsigned)
    {
        return MessageType__{Logger::MessageClass::Error, 0};
    }

    /// Manipulator to produce the critical error messages. Verbosity is always set to zero
    inline MessageType__ critical(unsigned)
    {
        return MessageType__{Logger::MessageClass::CriticalError, 0};
    }

}; // end of namespace logger
