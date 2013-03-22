#ifndef __DCIAO_KERNEL_IMPORTER_H__
#define __DCIAO_KERNEL_IMPORTER_H__

#include "BasicImporter.hpp"

/** \class DCiAOKernelImporter
 *
 * Strategy for importing traces into the MySQL Database, that cover
 * only kernel space data address. A single trace is only added to the
 * database, if it has the following form:
 *
 * - The memory access is after the kernel has passed the
 *   getEnterKernelAddress(), before it passes the
 *   getLeaveKernelAddress() again. Short: if the programm is in
 *   kernel space.
 * - Only memory addresses that are located in the os::data::dynamic
 *   namespace
 * - Only memory accesses to addresses that were not written in the
 *   current kernel space phase.
 */
class DCiAOKernelImporter : public BasicImporter {
protected:
	fail::address_t getEnterKernelAddress() { return m_elf->getSymbol("os::dep::KernelStructs::correct").getAddress(); }
	fail::address_t getLeaveKernelAddress() { return m_elf->getSymbol("os::dep::KernelStructs::calculate").getAddress(); }
	bool inDynamicKernelMemory(fail::address_t addr);

public:
	virtual bool copy_to_database(fail::ProtoIStream &ps);
};


#endif
