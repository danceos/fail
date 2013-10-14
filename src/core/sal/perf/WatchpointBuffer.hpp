#ifndef __WATCHPOINT_BUFFER_HPP__
#define __WATCHPOINT_BUFFER_HPP__

#include "BufferInterface.hpp"

namespace fail {

class MemAccessEvent;

/**
 * Concrete implementation of the PerfVector class for \c MemAccessListener.
 */
class PerfVectorWatchpoints : public DefPerfVector<MemAccessEvent> {
public:
	ResultSet& gather(MemAccessEvent* pData);
};

} // end-of-namespace: fail

#endif // __WATCHPOINT_BUFFER_HPP__
