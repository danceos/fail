#include "tester.hpp"
#include "sal/SALInst.hpp"
#include "util/Logger.hpp"
#include "util/CommandLine.hpp"
#include "tester.hpp"
#include "tracer.hpp"


using namespace fail;
using namespace std;


static fail::Logger m_log("dOSEK FAIL* Client");

void instantiatedOSEK()
{
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] -Wf,[option] ... <BochsOptions...>\n\n");
	CommandLine::option_handle MODE = cmd.addOption("", "mode", Arg::Required,
													"--mode \t Mode: tracer, tester (default: tester)");

	if (!cmd.parse()) {
		std::cerr << "Error parsing arguments." << endl;
		simulator.terminate(-1);
	}

	std::string mode = "tester";
	if (cmd[MODE].count() > 0)
		mode = std::string(cmd[MODE].first()->arg);

	m_log << "Handing over to " << mode << " mode" << std::endl;
	if (mode == "tester") {
		fail::simulator.addFlow(new dOSEKTester);
	} else if (mode == "tracer") {
		fail::simulator.addFlow(new dOSEKTracer);
	} else {
		std::cerr << "Invalid mode: " << mode << " (available: tracer, tester)" << std::endl;
		simulator.terminate(-1);
	}
}
