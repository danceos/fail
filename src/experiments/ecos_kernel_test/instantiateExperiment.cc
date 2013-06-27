#include "experiment.hpp"
#include "sal/SALInst.hpp"

void instantiateEcosKernelTestExperiment()
{
	fail::simulator.addFlow(new EcosKernelTestExperiment);
}
