#include <assert.h>
#include <iostream>

#include <stdlib.h>
#include "experiment.hpp"
#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"

#include "sal/bochs/BochsListener.hpp"
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "../plugins/realtimelogger/RealtimeLogger.hpp"
#include "../plugins/signalgenerator/SignalGenerator.hpp"

using namespace std;
using namespace fail;

// Check if configuration dependencies are satisfied:
#if  !defined(CONFIG_EVENT_MEMREAD) || !defined(CONFIG_EVENT_MEMWRITE)
#error This experiment needs: MemRead and MemWrite. Enable these in the configuration.
#endif


void EZSLogger::setupLog(const ElfSymbol & target, const string& prefix)
{

    if( target.getAddress() == 0 ){
        m_log << target << " Memory address not found. " << std::endl;
        simulator.terminate(1);
    }

    char *u = getenv("USER");
    if (u == NULL){
        m_log << "Username not found :(" << std::endl;
        simulator.terminate(1);
    }

    /* Set output path to /tmp/'prefix'-<USERNAME>.dac */
    std::string outputfile = "/tmp/" + prefix + "-" + std::string(u) + ".txt";

    // TODO delete rl. object sometimes?
    RealtimeLogger *rl = new RealtimeLogger(target, outputfile);

    simulator.addFlow(rl);
}

bool EZSLogger::run()
{
    m_log << "STARTING EZS Logger" << endl;

    ElfReader m_elf;

    // Invalidate Hpet by setting COUNTER_CLK_PERIOD > 0x05f5E100
    // Segfaults...
    // MemoryManager &mm = simulator.getMemoryManager();
    // uint64_t hpet_cap_reg = 0xffffffff00000000ULL;
    // mm.setBytes(0xFED00000, sizeof(hpet_cap_reg), &hpet_cap_reg);

    //! Setup RealtimeLogger instances:
    setupLog(m_elf.getSymbol("ezs_tracer_register"), "ezs-trace");
    setupLog(m_elf.getSymbol("ezs_dac_out_register"), "ezs-dac");

    //! Setup Superimposed sine waves: @see Sine::SineParams_t
    Sine::SineParamsList_t plist;
    plist.push_back(Sine::SineParams_t(2,0.7));
    plist.push_back(Sine::SineParams_t(10,0.3));
    //!  Initialize and install signal generator @see SignalGenerator
    const ElfSymbol & s_adc = m_elf.getSymbol("ezs_adc_in_register");
    SignalGenerator siggen(s_adc, new Sine(plist));
    simulator.addFlow(&siggen);


    //! Let the SUT know, we are in Bochs.
    const ElfSymbol & s_bochsid = m_elf.getSymbol("FAILBOCHSID");
    m_log << "FAILBOCHSID @ " << s_bochsid << std::endl;

    MemAccessListener l_bochsid(s_bochsid);
    MemoryManager & mm = simulator.getMemoryManager();
    uint32_t bochsid = 42;

    //! Simulate forever.
    while(true) {
      if(s_bochsid.getAddress() != 0){
        simulator.addListenerAndResume(&l_bochsid);
        mm.setBytes(s_bochsid.getAddress(), sizeof(bochsid), &bochsid);
      } else {
        simulator.resume();
      }
    }

    // Explicitly terminate, or the simulator will continue to run.
    simulator.terminate();

}

