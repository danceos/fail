#include "experiment.hpp"
#include "sal/SALInst.hpp"

void instantiateFiascoFailExperiment()
{
	fail::simulator.addFlow(new FiascoFailExperiment);
}
