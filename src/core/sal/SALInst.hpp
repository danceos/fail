#ifndef __SAL_INSTANCE_HPP__
#define __SAL_INSTANCE_HPP__

#include "SALConfig.hpp"
#include "config/VariantConfig.hpp"

#ifdef BUILD_BOCHS

#include "bochs/BochsController.hpp"

namespace fail {
typedef BochsController ConcreteSimulatorController; //!< concrete simulator (type)
}

#elif defined BUILD_GEM5

#include "gem5/Gem5Controller.hpp"

namespace fail {
typedef Gem5Controller ConcreteSimulatorController; //!< concrete simulator (type)
}

#elif defined BUILD_QEMU

#include "qemu/QEMUController.hpp"

namespace fail {
typedef QEMUController ConcreteSimulatorController; //!< concrete simulator (type)
}

#elif defined BUILD_T32

#include "t32/T32Controller.hpp"

namespace fail {
typedef T32Controller ConcreteSimulatorController; //!< concrete simulator (type)
}

#elif defined BUILD_PANDA

#include "panda/PandaController.hpp"

namespace fail {
typedef PandaController ConcreteSimulatorController; //!< concrete simulator (type)
}


#else
#error SAL Instance not defined
#endif

namespace fail {
extern ConcreteSimulatorController simulator; //!< the global simulator-controller instance
};

#endif // __SAL_INSTANCE_HPP__
