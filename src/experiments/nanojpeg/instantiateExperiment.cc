#include "experiment.hpp"
#include "sal/SALInst.hpp"

static NanoJPEGExperiment experiment;
void instantiateNanoJPEGExperiment()
{
	fail::simulator.addFlow(&experiment);
}
