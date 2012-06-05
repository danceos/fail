#! /usr/bin/env python

# create python bindings before running:
# protoc --python_out=. trace.proto

import trace_pb2
import sys
import struct

if len(sys.argv) != 2:
    print "Usage:", sys.argv[0], "tracefile.pb"
    sys.exit(-1)

trace_event = trace_pb2.Trace_Event()


try:
	f = open(sys.argv[1], "rb")
except IOError:
    print sys.argv[1] + ": Could not open file."
    sys.exit(-1)

while 1:
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
	#print trace_event

	# More compact dump for traces:
	if not trace_event.HasField("memaddr"):
		print "IP {0:x}".format(trace_event.ip)
	else:
		print "MEM {0} {1:x} width {2:d} IP {3:x}".format(
		"R" if trace_event.accesstype == trace_pb2.Trace_Event.READ else "W",
		trace_event.memaddr, trace_event.width, trace_event.ip)

f.close()
