/**
 * \file EventID.hpp
 * \author Andrey Popov
 *
 * The module defines an aggregate to store the event ID information.
 */

#pragma once

/**
 * \class EventID
 * \brief The class aggregates event ID information
 */
class EventID
{
public:
    /// Default constructor
    EventID();

    /// Constructor to specify the ID
    EventID(unsigned long runNumber_, unsigned long lumiBlockNumber_,
            unsigned long eventNumber_);

    /**
     * \brief Constructor from run number solely
     *
     * Consult the documentation for method Set(unsigned long, bool) for the description.
     */
    EventID(unsigned long runNumber_, bool minimal = true);

    /// Default copy constructor
    EventID(EventID const &) = default;

    /// Default assignment operator
    EventID &operator=(EventID const &) = default;

public:
    /// Sets the ID
    void Set(unsigned long runNumber_, unsigned long lumiBlockNumber_,
             unsigned long eventNumber_);

    /**
     * \brief Sets the ID with the run number only
     *
     * Sets the specified run number. If the second argument is true (default), the
     * lumiBlockNumber and eventNumber are set to 0, hence this instance of EventID is
     * guaranteed to be smaller or equal than all the events with the same run number. Otherwise
     * lumiBlockNumber and eventNumber are set to (-1), hence this event is greater than any
     * one with the same run number. This method is useful if the user wants to specify some
     * range based on the run numbers only.
     */
    void Set(unsigned long runNumber_, bool minimal = true);

    /// Comparison operator
    bool operator<(EventID const &rhs) const;

    /// Equality operator
    bool operator==(EventID const &rhs) const;

    /// Unstrict comparison operator
    bool operator<=(EventID const &rhs) const;

    /// Get the run number
    unsigned long Run() const;

    /// Get the luminosity block number
    unsigned long LumiBlock() const;

    /// Get the event number
    unsigned long Event() const;

private:
    unsigned long runNumber;       ///< The run number
    unsigned long lumiBlockNumber; ///< The luminosity block number
    unsigned long eventNumber;     ///< The event number
};