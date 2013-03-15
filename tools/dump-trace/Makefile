all: trace_pb2.py

trace_pb2.py: ../../src/plugins/tracing/trace.proto
	protoc --python_out=. --proto_path `dirname $<` $< 
