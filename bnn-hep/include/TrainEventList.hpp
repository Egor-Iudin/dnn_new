/**
 * \author Andrey Popov
 *
 * This module is indended to save the events tried for the training set to a text file and to read
 * them back.
 */

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <map>
using namespace std;
/**
 * \brief The class saves the events tried for the training set to a text file and reads them back.
 *
 * The class saves the events tried for the training set to a text file and reads them back. If an
 * event was chosen for the training set but failed the selection criteria it still gets in the
 * list (and must not be used for the exam set).
 */
class TrainEventList
{
public:
    /// The mode of work
    enum class Mode
    {
        Write,  ///< Writes the list of events
        Append, ///< Extends an existing file with the list of events
        Read    ///< Reads an existing file
    };

public:
    /// Constructor
    TrainEventList(std::string const &fileName_, Mode mode_ = Mode::Write);

    /// Destructor
    ~TrainEventList();

    /// Copy constructor (not allowed to be used)
    TrainEventList(TrainEventList const &) = delete;

    /// Assignment operator (not allowed to be used)
    TrainEventList const &operator=(TrainEventList const &) = delete;

public:
    /**
     * \brief Writes a list of events.
     *
     * Method writes the given index numbers of the events tried for training. The list is
     * prepended by the short name of the ROOT file and the size of the list.
     * \param sampleFileName The name of the ROOT file. It may contain some directories but
     *  they are stripped off.
     * \param begin Iterator marking the starting point of the range.
     * \param end Iterator making the ending poing of the range.
     *
     * \note The list of events is sorted before writing to the file. Duplicates are removed.
     */

public:
    map<string, int> trannnigSize;
    void WriteList(std::string const &sampleFileName,
                   std::vector<unsigned long>::const_iterator const &begin,
                   std::vector<unsigned long>::const_iterator const &end);

    /**
     * \brief Reads a list of events from the text file.
     *
     * Methods searches the associated text file for the list of events corresponding to the
     * requested sample file name.
     * \param sampleFileName The name of the ROOT file which the events are tacken from. It may
     *  contain some directories but they are stripped off.
     * \return The method returns false in case of an error (e.g. the requested sample file name
     *  is not found in the associated text file) and true otherwise.
     */
    bool ReadList(std::string const &sampleFileName);

    /// Returns the vector of event indices read last
    std::vector<unsigned long> const &GetReadEvents() const;

    /// Returns the name of the associated text file
    std::string const &GetFileName() const;

private:
    std::string const fileName;            ///< Name of the input/output file
    std::fstream fileStream;               ///< Stream associated to the file
    Mode mode;                             ///< Access mode
    bool listRead;                         ///< Indicates whether a list of events has been read successfully
    unsigned long nEventsRead;             ///< Number of events in the read list
    std::vector<unsigned long> eventsRead; ///< The list of events read
};
