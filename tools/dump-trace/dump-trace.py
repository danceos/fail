#! /usr/bin/env python

# create python bindings before running:
# protoc --python_out=. trace.proto

import TracePlugin_pb2
import sys
import struct
from gzip import GzipFile

if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], "tracefile.pb"
    sys.exit(-1)

trace_event = TracePlugin_pb2.Trace_Event()

def open_trace(filename):
    f = open(filename, "rb")
    if ord(f.read(1)) == 0x1f and ord(f.read(1)) == 0x8b:
        f.seek(0,0)
        return GzipFile(fileobj = f)
    f.seek(0,0)
    return f

try:
    f = open_trace(sys.argv[1])
except IOError:
    print sys.argv[1] + ": Could not open file."
    sys.exit(-1)

acctime = 0

while True:
    # Read trace length
    try:
        lengthNO = f.read(4)
        if len(lengthNO) == 0:
            break
    except IOError:
        print "Could not read data from file"

    # Read Trace-Event
    length = struct.unpack('!I', lengthNO)[0]
    trace_event.ParseFromString(f.read(length))

    # This works for any type of pb message:
    # print trace_event

    # More compact dump for traces:
    if trace_event.HasField("time_delta"):
        acctime += trace_event.time_delta
    # As time_delta fields are omitted where the delta is 0, every event
    # implicitly has a timestamp.
    time = " t={0}".format(acctime)
    if not trace_event.HasField("memaddr"):
        print "IP {0:x}{1}".format(trace_event.ip, time)
    else:
        ext = ""
        if trace_event.HasField("trace_ext"):
            ext = " DATA {0:x}".format(trace_event.trace_ext.data)
            for reg in trace_event.trace_ext.registers:
                ext += " REG: {0} *{1:x}={2:x}".format(reg.id, reg.value, reg.value_deref)
            if len(trace_event.trace_ext.stack) > 0:
                ext += " STACK: " + "".join(["%x"%x for x in trace_event.trace_ext.stack])
        print "MEM {0} {1:x} width {2:d} IP {3:x}{4}{5}".format(
        "R" if trace_event.accesstype == trace_event.READ else "W",
        trace_event.memaddr, trace_event.width, trace_event.ip, ext, time)

f.close()
