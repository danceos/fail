#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <stdint.h>
#include <sal/Memory.hpp>
#include "BaseFaultSpace.hpp"


namespace fail {

class MemoryArea;

class MemoryElement : public FaultSpaceElement {
public:

	MemoryElement(FaultSpaceArea* area, fsp_offset_t addr)
		: FaultSpaceElement(area, addr) { }

	virtual std::ostream&  dump(std::ostream & os) const override;
	virtual injector_result inject(injector_fn injector) override;
	guest_address_t get_guest_address() const;
};



class MemoryArea: public FaultSpaceArea {
private:
	MemoryManager* m_mm;
	guest_address_t m_base;
	size_t m_size;
public:
	MemoryArea(std::string name, guest_address_t base, size_t size)
		: FaultSpaceArea(name), m_mm(nullptr), m_base(base), m_size(size) { }

	fail::MemoryManager * get_manager() const { return m_mm; }
	void  set_manager(MemoryManager *mm) { m_mm = mm; }

	virtual std::unique_ptr<FaultSpaceElement> decode(fsp_address_t addr) override;

	guest_address_t get_base() const  { return m_base; }
	virtual const size_t get_size() const override { return m_size; }


	// For the Importers, we translate memory ranges to memory elements
	std::vector<MemoryElement> translate(guest_address_t from, guest_address_t to,
										 std::function<bool(guest_address_t)> filter);


};

} /* end-of-namespace fail */


