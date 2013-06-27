#include "experiment.hpp"
#include "sal/SALInst.hpp"

void instantiateNanoJPEGExperiment()
{
	fail::simulator.addFlow(new NanoJPEGExperiment);
}
