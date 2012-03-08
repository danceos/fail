#! /usr/bin/env python

# create python bindings before running:
# protoc --python_out=. trace.proto

import trace_pb2
import sys

if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], "tracefile.pb"
    sys.exit(-1)

trace = trace_pb2.Trace()

# Read trace
try:
    f = open(sys.argv[1], "rb")
    trace.ParseFromString(f.read())
    f.close()
except IOError:
    print sys.argv[1] + ": Could not open file."
    sys.exit(-1)

# This works for any type of pb message:
#print trace

# More compact dump for traces:
for event in trace.event:
    if not event.HasField("memaddr"):
        print "IP {0:x}".format(event.ip)
    else:
        print "MEM {0} {1:x} width {2:d} IP {3:x}".format(
          "R" if event.accesstype == trace_pb2.Trace.Event.READ else "W",
          event.memaddr, event.width, event.ip)
