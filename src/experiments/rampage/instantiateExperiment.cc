#include "experiment.hpp"
#include "sal/SALInst.hpp"

static RAMpageExperiment experiment;
void instantiateRAMpageExperiment()
{
	fail::simulator.addFlow(&experiment);
}
