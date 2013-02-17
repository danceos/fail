#include <iostream>
#include <fstream>

#include "sal/SALInst.hpp"
#include "sal/Register.hpp"
#include "sal/Listener.hpp"
#include "experiment.hpp"
#include "../plugins/tracing/TracingPlugin.hpp"

/*
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/gzip_stream.h>
*/

using namespace std;
using namespace fail;

bool TracingTest::run()
{
	cout << "[TracingTest] Setting up experiment" << endl;

#if 0
	// STEP 1: run until interesting function starts, and save state
	BPSingleListener breakpoint(0x00101658);
	simulator.addListenerAndResume(&breakpoint);
	cout << "[TracingTest] main() reached, saving" << endl;

	simulator.save("state");
#else
	// STEP 2: test tracing plugin
	simulator.restore("state");

	cout << "[TracingTest] enabling tracing" << endl;

	TracingPlugin tp;
	ofstream of("trace.pb");
	tp.setTraceFile(&of);
	// this must be done *after* configuring the plugin:
	simulator.addFlow(&tp);

	cout << "[TracingTest] tracing 1000000 instructions" << endl;
	BPSingleListener timeout(ANY_ADDR);
	timeout.setCounter(1000000);
	simulator.addListener(&timeout);

	InterruptListener ie(ANY_INTERRUPT);
	while (simulator.addListenerAndResume(&ie) != &timeout) {
		cout << "INTERRUPT #" << ie.getTriggerNumber() << "\n";
	}

	cout << "[TracingTest] tracing finished. (trace.pb)";
	simulator.removeFlow(&tp);
	of.close();

/*
	// serialize trace to file
	ofstream of("trace.pb");
	if (of.fail()) { return false; }
	trace.SerializeToOstream(&of);
	of.close();

	// serialize trace to gzip-compressed file
	int fd = open("trace.pb.gz", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (!fd) { return false; }
	google::protobuf::io::FileOutputStream fo(fd);
	google::protobuf::io::GzipOutputStream::Options options;
	options.compression_level = 9;
	google::protobuf::io::GzipOutputStream go(&fo, options);
	trace.SerializeToZeroCopyStream(&go);
	go.Close();
	fo.Close();
*/
#endif
	cout << "[TracingTest] Finished." << endl;
	simulator.clearListeners(this);
	simulator.terminate();

	return true;
}
