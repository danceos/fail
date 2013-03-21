#ifndef __T32TRACEFORMAT_H__
#define __T32TRACEFORMAT_H__
#include <string>
#include <stdint.h>
namespace fail {

  struct T32TraceHeader {
    uint8_t export_file_header_string[32];
    uint8_t reserved_32;
    uint8_t cpu_code;
    uint8_t timestamp_available;
    uint8_t prestore_mode;
    uint8_t trigger_unit_available;
    uint8_t port_analyzer_avaiable_mode;
    uint8_t analyzer_type;
    uint8_t reserved_39;

    uint32_t record_length; // in bytes
    uint32_t number_of_records; // in the file
    uint32_t last_record;
    uint32_t referenced_record;
    uint8_t reserved_56_63[8];
  };

  struct T32TraceRecord {
    struct  {
      uint32_t data_cycle :1; // bit 0
      uint32_t program_cycle :1; // bit 1
      uint32_t reserved_2_5 : 4; // bits 2-5
      uint32_t write_cycle : 1; // bit 6
      uint32_t reserved_7_20 : 14 ;// bits 7-20
      uint32_t flow_error : 1; // bit 21
      uint32_t reserved_22_24 : 3; //bits 22-24
      uint32_t fifo_overflow : 1; // bit 25
      uint32_t reserved_26_30 : 5; // bits 26-30
      uint32_t ownership_cycle :1; // bit 31
    } cycle_information;

    // data byte enable mask
    uint8_t data_byte_valid; // bit0 : byte0 valid, ...

    struct {
      uint8_t exec_signal : 1;
      uint8_t thumb_mode : 1;
      uint8_t arm_mode : 1;
      uint8_t reserved_3_4 : 2;
      uint8_t not_executed : 1;
      uint8_t executed : 1;
      uint8_t reserved_6_7 : 1;
    } cpu_info;

    uint8_t reserved_6;

    uint8_t core_number; // only on SMP targets
    uint32_t bus_data_address; // bus/data
    uint32_t pflow_address; // or upper part
    uint64_t data;
    uint64_t timestamp; // relative to ZERO in ns

    bool isDataAccess(void) const { return cycle_information.data_cycle == 1; };
    bool isProgram(void) const { return cycle_information.program_cycle == 1; };
    bool isDataRead(void) const   { return cycle_information.data_cycle && !cycle_information.write_cycle; };
    bool isDataWrite(void) const  { return cycle_information.data_cycle && cycle_information.write_cycle;  };
    uint32_t getAddress(void) const { return bus_data_address; };
    uint32_t getData(void) const  { return data; };


  };

  //<! This allows to print a record via Logger or cout
  std::ostream& operator <<(std::ostream & os, const fail::T32TraceRecord & r);

};

#endif // __T32TRACEFORMAT_H__

