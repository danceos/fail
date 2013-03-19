#include "experiment.hpp"
#include "sal/SALInst.hpp"

static EcosKernelTestExperiment experiment;
void instantiateEcosKernelTestExperiment()
{
	fail::simulator.addFlow(&experiment);
}
