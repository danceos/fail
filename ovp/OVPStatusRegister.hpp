#ifndef __OVPSTATUSREGISTER_HPP__
#define __OVPSTATUSREGISTER_HPP__

#include "SAL/ovp/OVPRegister.hpp"

/**
 * \class OVPStatusRegister
 * Abstract class for status register implementation
 */
class OVPStatusRegister : public sal::OVPRegister {

	protected:
	
	public:
		OVPStatusRegister(uint32_t width, void *link) 
			: sal::OVPRegister(width, link, sal::RT_ST) { }
		~OVPStatusRegister() {}

		virtual bool getSignFlag() const = 0;
		virtual bool getZeroFlag() const = 0;
		virtual bool getCarryFlag() const = 0;
		virtual bool getOverflowFlag() const = 0;
		
		virtual void setSignFlag(bool) = 0;
		virtual void setZeroFlag(bool) = 0;
		virtual void setCarryFlag(bool) = 0;
		virtual void setOverflowFlag(bool) = 0;

		/**
		 * Invert bit at specific position of status register
		 * @param pos position of bit to invert
		 */
/*		void invertBit(int pos) {
			size_t val;
			size_t chpos = 1 << pos;

			icmReadRegInfoValue(cpuP, cpsr, (void *)&val);
			
			// get bit
			bool bit = (val >> pos) & 0x1;

			if(bit == 0) {
				chpos = ~chpos;
				val = val & chpos;
			} else {
				val = val | chpos;
			}

		}*/
};

#endif
