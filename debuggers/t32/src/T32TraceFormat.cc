
#include "T32TraceFormat.hpp"
#include <iostream>

using namespace std;

namespace fail {

  std::ostream& operator <<(std::ostream & os, const fail::T32TraceRecord & r) {
#ifndef __puma
    if(r.isDataWrite()){
      os << "WRITE";
    } else if (r.isDataRead()) {
      os << "READ";
    } else if( r.isProgram()) {
      os << "PROGRAM";
    } else {
      os << "UNKNOWN";
    }
    os << "\t" <<  hex << (int)(r.getAddress()) << "\t" << r.getData() << "\t";
#endif
    return os;
  }

}; // end of namespace
