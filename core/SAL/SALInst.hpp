#ifndef __SAL_INSTANCE_HPP__
  #define __SAL_INSTANCE_HPP__

#include "SALConfig.hpp"
#include "config/variant_config.hpp"

#ifdef BUILD_BOCHS

#include "bochs/BochsController.hpp"

namespace fail {

typedef BochsController ConcreteSimulatorController; //!< concrete simulator (type)
extern ConcreteSimulatorController simulator; //!< the global simulator-controller instance

}

#elif defined BUILD_OVP

#include "ovp/OVPController.hpp"

namespace fail {

typedef OVPController ConcreteSimulatorController; //!< concrete simulator (type)
extern ConcreteSimulatorController simulator; //!< the global simulator-controller instance

}

#else
#error SAL Instance not defined
#endif

#endif // __SAL_INSTANCE_HPP__
