#pragma once

#include "framework/logging/log_stream.h"
#include "framework/logging/log_exceptions.h"
#include "framework/logging/timing_log.h"

#include <utility>
#include <vector>
#include <memory>

namespace opensn
{

/**Object for controlling logging.
   *
   * ## Part A: Output logs
   * There are three levels of verbosity in OpenSn: Zero(Default), One and Two.
   * These can be set on the command line via the switch -v followed by a
   * space and the number for the verbosity (0,1 or 2).
   *
   * \code
   * ./opensn InputFile.lua -v 1
   * \endcode
   *
   * The lua command `LogSetVerbosity(int_level)` achieves the same.\n\n
   *
   * Printing a log under the auspices of a verbosity level again has
   * numerous options. Firstly, any log can be a normal log, a warning
   * or an error. Secondly, a log can be either location 0 or all the locations
   * in a parallel process environment. The log option enums defined under
   * LOG_LVL are
   - LOG_0,                      Used only for location 0
   - LOG_0WARNING,               Warning only for location 0
   - LOG_0ERROR,                 Error only for location 0
   - LOG_0VERBOSE_0,             Default verbosity level
   - LOG_0VERBOSE_1,             Used only if verbosity level equals 1
   - LOG_0VERBOSE_2,             Used only if verbosity level equals 2
   - LOG_ALL,                    Verbose level 0 all locations
   - LOG_ALLWARNING,             Warning for any location
   - LOG_ALLERROR,               Error for any location
   - LOG_ALLVERBOSE_0,     Default verbosity level
   - LOG_ALLVERBOSE_1,     Used only if verbosity level equals 1
   - LOG_ALLVERBOSE_2,     Used only if verbosity level equals 2

   * A log can be made by first connecting code with the logger. This is done
   * by including the log header and then defining an extern reference to the
   * global object.
   *
   * \code
   * #include "framework/logging/log.h"
   * extern Logger& opensn::log;
   * \endcode
   *
   * A log is then written inside a piece of code as follows:
   *
   * \code
   * void PrintSomethingToLog()
   * {
   *   opensn::log.Log() << "This is printed on location 0 only";
   *   opensn::log.Log0Warning() << "This is a warning";
   *   opensn::log.Log0Error() << "This is an error";
   * }
   * \endcode
   *
  \verbatim
  [0]  This is printed on location 0 only
  [0]  **WARNING** This is a warning
  [0]  **!**ERROR**!** This is an error
  \endverbatim
   *
   * ## Part B: Repeating events log
   * Suppose a routine or segment of code gets called multiple times. Now
  suppose
   * we want to know how many times this routine was called, how long it ran
   * (total and on-avg), or we want to know at which timestamp it ran every
  time.
   * We do this by using a repeating event. A repeating event is identified
   * by a tag. Therefore the first step is to obtain a unique id for the event
  using
   * Logger::GetRepeatingEventTag.
   *
   * \code
   * size_t tag = opensn::log.GetRepeatingEventTag(std::string("Sweep Timing"))
   * \endcode
   *
   * Events can now be logged using
   * Logger::LogEvent(size_t ev_tag, Logger::EventType ev_type). As an example,
   * consider we want to count the number of occurrences of an event. We trigger
   * multiple occurences using
   *
   * \code
   * opensn::log.LogEvent(tag,opensn::Logger::EventType::SINGLE_OCCURRENCE);
   * \endcode
   *
   * At the end of the block for which you wanted to log these events we can now
   * extract the number of occurrences using
   *
   * \code
   * double number_of_occ =
  opensn::log.ProcessEvent(tag,opensn::Logger::EventOperation::NUMBER_OF_OCCURRENCES);
   *\endcode
   *
   * Below is a complete example:
   *
  \code
  size_t tag = opensn::log.GetRepeatingEventTag(std::string());

  for (int i=0; i<5; ++i)
    opensn::log.LogEvent(tag,Logger::EventType::SINGLE_OCCURRENCE);

  opensn::log.LogAll()
    << opensn::log.ProcessEvent(tag,
  Logger::EventOperation::NUMBER_OF_OCCURRENCES); \endcode \verbatim
  6
  \endverbatim

   * Since opensn::log is a global object this functionality is really powerful for
   * use in modules or class objects where multiple methods can contribute to
   * the same event track. For example an object can have an event tag as a member
   * and initialize it at construction then all of the objects methods can
   * contribute to the event.
   *
   * ### Supplying event information
   * In addition to the Logger::EventType the user can also supply a reference to
   * a Logger::EventInfo structure. Be cautious with this though because each
   * event stored a double and a string which can look like a memory leak if
   * multiple events are pushed up with event information. Developers can supply
   * either a double or a string or both to an event info constructor to
   * instantiate an instance. The event arb_value is by default 0.0 and the event
   * arb_info is by default an empty string. An example is shown below:
   *
  \code
  size_t tag = opensn::log.GetRepeatingEventTag(std::string());

  for (int i=0; i<5; ++i)
  {
    auto ev_info = std::make_shared<opensn::Logger::EventInfo>(i*2.0);
    opensn::log.LogEvent(tag,opensn::Logger::EventType::SINGLE_OCCURRENCE,ev_info);
  }

  opensn::log.LogAll()
    << opensn::log.ProcessEvent(tag, opensn::Logger::EventOperation::AVERAGE_VALUE);
  \endcode
  \verbatim
  4.0
  \endverbatim
   *
   * Event information can also be supplied by string values but then the
   * average value is meaningless unless the average value is also supplied.
   * For example:
   *
  \code
  size_t tag = opensn::log.GetRepeatingEventTag(std::string());

  opensn::log.LogEvent(tag,
                       opensn::Logger::EventType::SINGLE_OCCURRENCE,
                       std::make_shared<opensnLogger::EventInfo>(std::string("A"),2.0));
  opensn::log.LogEvent(tag,
                       opensn::Logger::EventType::SINGLE_OCCURRENCE,
                       std::make_shared<opensn::Logger::EventInfo>(std::string("B")));
  opensn::log.LogEvent(tag,
                       opensn::Logger::EventType::SINGLE_OCCURRENCE,
                       std::make_shared<opensn::Logger::EventInfo>(std::string("C"),2.0));

  opensn::log.LogAll() << opensn::log.ProcessEvent(tag, Logger::EventOperation::AVERAGE_VALUE);
  \endcode
  \verbatim
  1.33333
  \endverbatim
   *
   * To get a string value of the event history developers can use
   * Logger::PrintEventHistory along with the event tag. Just note that it will
   * automatically be formatted for each location so no need to use opensn::log to
   * print it. Also, each event will be prepended with a program timestamp
   * in seconds.
   *
  \code
  std::cout << opensn::log.PrintEventHistory(tag);
  \endcode
  \verbatim
  1.33333
  [0]      3.813119000 EVENT_CREATED
  [0]      3.813120000 SINGLE_OCCURRENCE A
  [0]      3.813121000 SINGLE_OCCURRENCE B
  [0]      3.813122000 SINGLE_OCCURRENCE C
  \endverbatim
   * */
class Logger : public TimingLog
{
public:
  /**Logging level*/
  enum LOG_LVL
  {
    LOG_0 = 1,             ///< Used only for location 0
    LOG_0WARNING = 2,      ///< Warning only for location 0
    LOG_0ERROR = 3,        ///< Error only for location 0
    LOG_0VERBOSE_0 = 4,    ///< Default verbosity level
    LOG_0VERBOSE_1 = 5,    ///< Used only if verbosity level equals 1
    LOG_0VERBOSE_2 = 6,    ///< Used only if verbosity level equals 2
    LOG_ALL = 7,           ///< Verbose level 0 all locations
    LOG_ALLWARNING = 8,    ///< Warning for any location
    LOG_ALLERROR = 9,      ///< Error for any location
    LOG_ALLVERBOSE_0 = 10, ///< Default verbosity level
    LOG_ALLVERBOSE_1 = 11, ///< Used only if verbosity level equals 1
    LOG_ALLVERBOSE_2 = 12  ///< Used only if verbosity level equals 2
  };

private:
  DummyStream dummy_stream_;
  int verbosity_;

public:
  /**Access to the singleton*/
  static Logger& GetInstance() noexcept;

private:
  /** Default constructor*/
  Logger() noexcept;

public:
  /** Makes a log entry.*/
  LogStream Log(LOG_LVL level = LOG_0);
  /** Sets the verbosity level.*/
  void SetVerbosity(int int_level);
  /** Gets the current verbosity level.*/
  int GetVerbosity() const;

  LogStream Log0() { return Log(LOG_0); }
  LogStream Log0Warning() { return Log(LOG_0WARNING); }
  LogStream Log0Error() { return Log(LOG_0ERROR); }
  LogStream Log0Verbose0() { return Log(LOG_0VERBOSE_0); }
  LogStream Log0Verbose1() { return Log(LOG_0VERBOSE_1); }
  LogStream Log0Verbose2() { return Log(LOG_0VERBOSE_2); }

  LogStream LogAll() { return Log(LOG_ALL); }
  LogStream LogAllWarning() { return Log(LOG_ALLWARNING); }
  LogStream LogAllError() { return Log(LOG_ALLERROR); }
  LogStream LogAllVerbose0() { return Log(LOG_ALLVERBOSE_0); }
  LogStream LogAllVerbose1() { return Log(LOG_ALLVERBOSE_1); }
  LogStream LogAllVerbose2() { return Log(LOG_ALLVERBOSE_2); }

private:
  class RepeatingEvent;

public:
  enum StdTags
  {
    MAX_MEMORY_USAGE = 0 ///< Tag reserved for logging process memory
  };
  enum class EventType
  {
    EVENT_CREATED = 0,     ///< Automatically signalled when event is created
    SINGLE_OCCURRENCE = 1, ///< Signals a single occurrence
    EVENT_BEGIN = 2,       ///< Signals the begin of an event
    EVENT_END = 3          ///< Signals the end of an event
  };
  enum class EventOperation
  {
    NUMBER_OF_OCCURRENCES = 0, ///< Counts creation events, single occurrences and begins
    TOTAL_DURATION = 1,        ///< Integrates times between begins and ends
    AVERAGE_DURATION = 2,      ///< Computes average time between begins and ends
    MAX_VALUE = 3,             ///< Computes the maximum of the EventInfo arb_value
    AVERAGE_VALUE = 4          ///< Computes the average of the EventInfo arb_value
  };
  struct EventInfo;
  struct Event;

private:
  std::vector<RepeatingEvent> repeating_events;

public:
  /** Returns a unique tag to a newly created repeating event.*/
  size_t GetRepeatingEventTag(std::string event_name);
  /** Returns a unique tag to the latest version of an existing repeating event.*/
  size_t GetExistingRepeatingEventTag(std::string event_name);
  /**Logs an event with the supplied event information.*/
  void LogEvent(size_t ev_tag, EventType ev_type, const std::shared_ptr<EventInfo>& ev_info);
  /**Logs an event without any event information.*/
  void LogEvent(size_t ev_tag, EventType ev_type);
  /**Returns a string representation of the event history associated with
   * the tag. Each event entry will be prepended by the location id and
   * the program timestamp in seconds. This method uses the
   * Logger::EventInfo::GetString method to append information. This allows
   * derived classes to implement more sophisticated outputs.*/
  std::string PrintEventHistory(size_t ev_tag);
  /**Processes an event given an event operation. See Logger for further
   * reference.*/
  double ProcessEvent(size_t ev_tag, EventOperation ev_operation);
};

/** */
struct Logger::EventInfo
{
  std::string arb_info;
  double arb_value = 0.0;
  EventInfo() : arb_info(std::string()) {}

  explicit EventInfo(std::string text) : arb_info(std::move(text)) {}
  explicit EventInfo(double value) : arb_value(value) {}
  explicit EventInfo(std::string text, double value) : arb_info(std::move(text)), arb_value(value)
  {
  }

  virtual ~EventInfo() = default;

  virtual std::string GetString() { return arb_info; }
};

/** Object used by repeating events.*/
struct Logger::Event
{
  const double ev_time = 0.0;
  const EventType ev_type = EventType::SINGLE_OCCURRENCE;
  std::shared_ptr<EventInfo> ev_info;

  Event(double time, EventType ev_type, std::shared_ptr<EventInfo> event_info)
    : ev_time(time), ev_type(ev_type), ev_info(std::move(event_info))
  {
  }
};

/**Repeating event object.*/
class Logger::RepeatingEvent
{
public:
  explicit RepeatingEvent(std::string& name) : name_(name) {}

  const std::string& Name() const { return name_; }

  std::vector<Event>& Events() { return events_; }
  const std::vector<Event>& Events() const { return events_; }

  bool operator==(const RepeatingEvent& other) { return this->name_ == other.name_; }

private:
  const std::string name_;
  std::vector<Event> events_;
};

} // namespace opensn
