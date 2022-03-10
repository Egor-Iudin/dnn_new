#include <EventID.hpp>

EventID::EventID() : runNumber(0), lumiBlockNumber(0), eventNumber(0)
{
}

EventID::EventID(unsigned long runNumber_, unsigned long lumiBlockNumber_,
                 unsigned long eventNumber_) : runNumber(runNumber_), lumiBlockNumber(lumiBlockNumber_), eventNumber(eventNumber_)
{
}

EventID::EventID(unsigned long runNumber_, bool minimal /*= true*/)
{
    Set(runNumber_, minimal);
}

void EventID::Set(unsigned long runNumber_, unsigned long lumiBlockNumber_,
                  unsigned long eventNumber_)
{
    runNumber = runNumber_;
    lumiBlockNumber = lumiBlockNumber_;
    eventNumber = eventNumber_;
}

void EventID::Set(unsigned long runNumber_, bool minimal /*= true*/)
{
    runNumber = runNumber_;

    if (minimal)
    {
        lumiBlockNumber = 0;
        eventNumber = 0;
    }
    else
    {
        lumiBlockNumber = -1;
        eventNumber = -1;
    }
}

bool EventID::operator<(EventID const &rhs) const
{
    if (runNumber != rhs.runNumber)
        return (runNumber < rhs.runNumber);

    if (lumiBlockNumber != rhs.lumiBlockNumber)
        return (lumiBlockNumber < rhs.lumiBlockNumber);

    return (eventNumber < rhs.eventNumber);
}

bool EventID::operator==(EventID const &rhs) const
{
    return (runNumber == rhs.runNumber and lumiBlockNumber == rhs.lumiBlockNumber and
            eventNumber == rhs.eventNumber);
}

bool EventID::operator<=(EventID const &rhs) const
{
    return not(rhs < *this);
}

unsigned long EventID::Run() const
{
    return runNumber;
}

unsigned long EventID::LumiBlock() const
{
    return lumiBlockNumber;
}

unsigned long EventID::Event() const
{
    return eventNumber;
}