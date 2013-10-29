#! /usr/bin/env python

# create python bindings before running:
# protoc --python_out=. trace.proto

import TracePlugin_pb2
import sys
import struct
import subprocess
from gzip import GzipFile

if len(sys.argv) != 3:
    print "Usage:", sys.argv[0], "elf tracefile.pb"
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
    f = open_trace(sys.argv[2])
except IOError:
    print sys.argv[1] + ": Could not open file."
    sys.exit(-1)

acctime = 0

def get_symbol(elf, symbol):
    output = subprocess.check_output(["nm", "-t", "dec", elf])
    for line in output.split("\n"):
        if line.endswith(symbol):
            return int(line.split(" ")[0])

fail_virtual_port = get_symbol(sys.argv[1], "fail_virtual_port")

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

    if trace_event.HasField("memaddr"):
        if not trace_event.memaddr == fail_virtual_port:
            continue
        if not trace_event.accesstype == trace_event.WRITE:
            continue
        at = trace_event.trace_ext.data >> 16
        time = trace_event.trace_ext.data & 0xffff
        print time, at

f.close()
