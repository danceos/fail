#ifndef FLAKY_MEMORY_HPP
#define FLAKY_MEMORY_HPP

#include "icm/icmCpuManager.hpp"

using namespace icmCpuManager;

/** Creates flaky memory by attaching an MMC for the specified address range.
 * @param processor The processor on which to set up the MMC
 * @param loAddr The lowest address for which to do on-the-fly manipulation.
 * @param hiAddr The highest address for which to do manipulation.
 */
void createFlakyMem(
    icmProcessorObject processor,
    Addr loAddr,
    Addr hiAddr,
    const char *vlnvRoot = 0
);


#endif