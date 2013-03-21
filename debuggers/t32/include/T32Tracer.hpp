#ifndef __T32TRACER_HPP__
#define __T32TRACER_HPP__

#include <string>
#include <vector>
#include "T32TraceFormat.hpp"
#include "util/Logger.hpp"

namespace fail {

class T32Tracer {
  private:
    typedef std::vector<T32TraceRecord> record_vector_t;

    bool avail;
    std::string path;
    std::string exportcmd;
    record_vector_t rvec;
    fail::Logger m_log;

  public:
    typedef record_vector_t::const_reverse_iterator const_record_iterator;

    T32Tracer( const std::string& tracefile );

    void setup(void) const;
    int evaluate();

    bool wasDataAccess(void) const ;
    const T32TraceRecord & getLatestRecord(void) const { return rvec.back(); };
    void dump(void);

    bool available(void) const { return avail; };

    // We return a reverse operator, as the trace list begins with the oldest record.
    // We just let it look like a "normal" iterator and traverse backwards starting
    // with most recent record. (Rbegin/Rend!)
    const_record_iterator begin() const { return rvec.rbegin(); };
    const_record_iterator end() const { return rvec.rend(); };
};

}; // end of namespace
#endif // __T32TRACER_HPP__

