#include "wrapper.h"
#include "sal/SALInst.hpp"
#include "simulator.hpp"
#include <vector>
#include <utility>
#include <bitset>

namespace fail {
    namespace sail {
        const std::vector<std::pair<std::bitset<32>,std::bitset<32>>> jumpOpcodes = {
            { // c.jv
                std::bitset<32>("00000000000000001110000000000011"),
                std::bitset<32>("00000000000000001010000000000001")
            },
            {   // c.jr
                std::bitset<32>("00000000000000001111000001111111"),
                std::bitset<32>("00000000000000001000000000000010")
            },
            {   // c.jal
                std::bitset<32>("00000000000000001110000000000011"),
                std::bitset<32>("00000000000000000010000000000001")
            },
            {   // c.jalr
                std::bitset<32>("00000000000000001111000001111111"),
                std::bitset<32>("00000000000000001001000000000010")
            },
            {   // jal
                std::bitset<32>("00000000000000000000000001111111"),
                std::bitset<32>("00000000000000000000000001101111")
            },
            {   // jalr
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000000000001100111")
            },
            {   // beq
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000000000001100011")
            },
            {   // bne
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000001000001100011")
            },
            {   // blt
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000100000001100011")
            },
            {   // bge
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000101000001100011")
            },
            {   // bltu
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000110000001100011")
            },
            {   // bgeu
                std::bitset<32>("00000000000000000111000001111111"),
                std::bitset<32>("00000000000000000111000001100011")
            },
            {   // c.beqz
                std::bitset<32>("00000000000000001110000000000011"),
                std::bitset<32>("00000000000000001100000000000001")
            },
            {   // c.bnez
                std::bitset<32>("00000000000000001110000000000011"),
                std::bitset<32>("00000000000000001110000000000001")
            },
            {   // mret
                std::bitset<32>("11111111111111111111111111111111"),
                std::bitset<32>("00110000001000000000000001110011")
            },
            {   // sret
                std::bitset<32>("11111111111111111111111111111111"),
                std::bitset<32>("00010000001000000000000001110011")
            },
            {   // uret
                std::bitset<32>("11111111111111111111111111111111"),
                std::bitset<32>("00000000001000000000000001110011")
            }
        };
        bool inIFetch = false;
    }
}

extern "C" {
void fail_startup(int argc, char** argv) {
    fail::simulator.startup(argc, argv);
}

int fail_onGuestSystem(char data, unsigned port) {
    #if defined(CONFIG_EVENT_GUESTSYS)
        fail::simulator.onGuestSystem(data, port);
    #endif
    return 0;
}

int fail_onMemoryRead(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr) {
    #if defined(CONFIG_EVENT_MEMREAD)
    if(!fail::sail::inIFetch) {
        static bool warned_once = false;
        if(!mpz_fits_ulong_p(data) && !warned_once) {
            std::cerr << "[sail wrapper] warning once: can't fit accessed data into 8-byte, extended trace information will be truncated!" << std::endl;
            warned_once = true;
        }

        // alternatively one could create a vector of arbitrary length here, but Trace_Event_Extended only supports
        // upto 8 byte extended trace information anyway.
        uint64_t udata = mpz_get_ui(data);
        size_t bitsize = mpz_get_ui(size);
        assert(bitsize >= mpz_sizeinbase(data, 2));

        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);

        fail::simulator.onMemoryAccess(&triggerCPU, addr, static_cast<memory_type_t>(mem_type), bitsize, udata, false, instrPtr);
    }
#endif
    return 0;
}

int fail_onMemoryWrite(uint64_t addr, mpz_t data, mpz_t size, uint64_t mem_type, uint64_t instrPtr) {
    #if defined(CONFIG_EVENT_MEMWRITE)
        static bool warned_once = false;
        if(!mpz_fits_ulong_p(data) && !warned_once) {
            std::cerr << "[sail wrapper] warning once: can't fit accessed data into 8-byte, extended trace information will be truncated!" << std::endl;
            warned_once = true;
        }

        // alternatively one could create a vector of arbitrary length here, but Trace_Event_Extended only supports
        // upto 8 byte extended trace information anyway.
        uint64_t udata = mpz_get_ui(data);
        size_t bitsize = mpz_get_ui(size);
        assert(bitsize >= mpz_sizeinbase(data, 2));
        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);

        fail::simulator.onMemoryAccess(&triggerCPU, addr, static_cast<memory_type_t>(mem_type), bitsize, udata, true, instrPtr);
    #endif
    return 0;
}

int fail_willExecuteInstruction(uint64_t instrPtr) {
#if defined(CONFIG_EVENT_BREAKPOINTS) || defined(CONFIG_EVENT_BREAKPOINTS_RANGE)
    fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
    // always address space 0
    fail::simulator.onBreakpoint(&triggerCPU, instrPtr, 0);
#endif
    return 0;
}

int fail_executeRequests() {
#if defined(CONFIG_SR_SAVE) || defined(CONFIG_SR_RESTORE)
    fail::simulator.checkTimers();
    fail::simulator.executeRequests();
#endif
    return 0;
}

int fail_didExecuteInstruction(uint64_t instrPtr, uint64_t instr) {
    #if defined(CONFIG_EVENT_JUMP)
        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
        std::bitset<32> binaryInstr(instr);

        for (const auto& p: jumpOpCodes) {
            if (binaryInstr & p.first == p.second)
                fail::simulator.onJump(&triggerCPU, false, binaryInstr)
        }
    #endif
    return 0;
}

int fail_onTrap(unsigned trapNum) {
    #if defined(CONFIG_EVENT_TRAP)
        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
        fail::simulator.onTrap(&triggerCPU, trapNum);
    #endif
    return 0;
}

int fail_onInterrupt(unsigned interruptNum, bool nmi) {
    #if defined(CONFIG_EVENT_INTERRUPT)
        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
        fail::simulator.onInterrupt(&triggerCPU, interruptNum, nmi);
    #endif
    return 0;
}

bool fail_isSuppressedInterrupt(unsigned interruptNum) {
    #if defined(CONFIG_SUPPRESS_INTERRUPTS)
        fail::ConcreteCPU& triggerCPU = fail::simulator.getCPU(0);
        return triggerCPU.isSuppressedInterrupt(interruptNum);
    #endif
    return false;
}


int fail_setInstructionFetch(bool instructionFetch) {
    fail::sail::inIFetch = instructionFetch;
    return 0;
}

}
