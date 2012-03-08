#include "platform.hpp"
#include "../../OVPStatusRegister.hpp"

extern ARM_Cortex_M3 arm;

/**
 * \class CortexM3StatusRegister
 * Status register for ARM Cortex M3
 */
class CortexM3StatusRegister : public OVPStatusRegister {

	private:
		icmRegInfoP cpsr; //!< pointer to status register
		icmProcessorP cpuP;

	public:
		CortexM3StatusRegister() : OVPStatusRegister(32, icmGetRegByIndex(arm.getProcessorP(), 16)) {
			cpuP = arm.getProcessorP();
			
			// status register is index 16
			cpsr = icmGetRegByIndex(cpuP, 16);
		}

		~CortexM3StatusRegister() {}

		bool getFlag(int pos) const {
			uint32_t val;
			uint32_t bit;
			icmReadRegInfoValue(cpuP, cpsr, (void *)&val);

			// put bit to correct place
			bit = 1 << pos;

			// Flag at position pos
			val = val & bit;
			val = val >> pos;

			return val;
		}

		void setFlag(int pos, bool newval) {
			uint32_t val;
			uint32_t bit = 1 << pos;

			icmReadRegInfoValue(cpuP, cpsr, (void *)&val);

			if(newval == 0) {
				// AND with 0xFFFEFFFF (e.g.)
				bit = ~bit; // invert bits
				
				val = val & bit;
			} else {
				// OR with 0x00010000 (e.g.)
				val = val | bit;
			}
		}

		bool getSignFlag() const {
			return getFlag(31);
		}

		bool getZeroFlag() const {
			return getFlag(30);
		}

		bool getCarryFlag() const {
			return getFlag(29);
		}

		bool getOverflowFlag() const {
			return getFlag(28);
		}

		void setSignFlag(bool b) {
			setFlag(31, b);
		}

		void setZeroFlag(bool b) {
			setFlag(30, b);
		}

		void setCarryFlag(bool b) {
			setFlag(29, b);
		}

		void setOverflowFlag(bool b) {
			setFlag(28, b);
		}

};
