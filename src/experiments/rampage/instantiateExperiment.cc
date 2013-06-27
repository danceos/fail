#include "experiment.hpp"
#include "sal/SALInst.hpp"

void instantiateRAMpageExperiment()
{
	fail::simulator.addFlow(new RAMpageExperiment);
}
