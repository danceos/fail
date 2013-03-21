#include "T32Tracer.hpp"
#include "T32TraceFormat.hpp"
#include <fstream>
#include <iostream>
#include "t32.h"

using namespace std;

namespace fail {

  T32Tracer::T32Tracer( const std::string& tfile ) : path(tfile), m_log("T32Tracer", false) {
  // TODO Check if tracing is available:
    avail = true;
    exportcmd = "Trace.export " + path;
    m_log << "Trace file: " << path.c_str() << endl;
  }


  void T32Tracer::setup() const {
    T32_Cmd("Trace.SIZE 16."); // trace 16 records
    T32_Cmd("Trace.METHOD Analyzer");
    T32_Cmd("Trace.autoarm on");
    T32_Cmd("Trace.autoinit on");
    T32_Cmd("Trace.state");
  }

  int T32Tracer::evaluate(){
    // clear old records
    rvec.clear();

    // export trace to file
    //T32_Cmd(static_cast<char*>(exportcmd.c_str()));
    T32_Cmd((char*)(exportcmd.c_str()));
    // open trace file.
    ifstream tracefile;
    tracefile.open(path.c_str(), ios::in | ios::binary);

    if(tracefile.is_open()){
      // evaluate trace.
      T32TraceHeader hdr;
      tracefile.seekg(0, ios::beg);
      tracefile.read(reinterpret_cast<char*>(&hdr), sizeof(T32TraceHeader));

      T32TraceRecord r;
      for(size_t i = 0; i < hdr.number_of_records; ++i) {
        tracefile.read(reinterpret_cast<char*>(&r), sizeof(T32TraceRecord));
        rvec.push_back(r); // add trace record to vector
      }
      return rvec.size();
    }
    return -1;
  }

  bool T32Tracer::wasDataAccess(void) const {
    // TODO if rvec.size() != 0 !!
    return rvec.back().isDataAccess();
  }


  void T32Tracer::dump(void){
    for(size_t i = 0; i < rvec.size(); ++i) {
      m_log << "[" << dec << i << "] " << rvec[i] << endl;
    }
  }

}; // end of namespace fail
