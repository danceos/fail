#pragma once

#include <map>
#include <algorithm>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <cassert>


namespace fail {

typedef uint64_t fsp_address_t;
typedef uint64_t fsp_offset_t;


class FaultSpaceElement;

/**
 * Abstraction for a specific area inside the fault space. The given
 * implementation can translate existing fault space elements which
 * belong to this area into absolute fault space addresses. When
 * subclassed, the specific subclass needs to provide a way to
 * generate fault space elements, which then can be mapped through the
 * base class. Additionally the subclass must provide function for
 * decoding an absolute fault space address to a injectable fault
 * space element and a method for getting its size.
 */

class FaultSpaceArea {
private:
protected:
	const std::string m_name;
	fsp_offset_t m_offset;
public:
	FaultSpaceArea(const std::string name): m_name(std::move(name)) { }
	FaultSpaceArea(FaultSpaceArea&) = delete;
	FaultSpaceArea(const FaultSpaceArea&) = delete;
	virtual ~FaultSpaceArea() { }

	void         set_offset(fsp_offset_t offset) { m_offset = offset; }
	fsp_offset_t get_offset() const              { return m_offset; }

	const std::string& get_name() const { return m_name; }

	// Returns the size of this area.
	virtual const size_t get_size() const = 0;

	// recreate a previsouly encoded element from the fault space address
	// the address passed to this function will be relative to the fault areas
	// mapping.
	virtual std::unique_ptr<FaultSpaceElement> decode(fsp_offset_t addr) = 0;
};

/**
 * Abstraction for a single element inside the fault space.
 * It belongs to an area, and saves additional information to the
 * point in the fault space such as.
 * NOTE: This is copy constructible, but this functionality should only
 *       be used, if the element is not used for injecting since the inject
 *       method is virtual and overriden in specific subclasses.
 * every fault space element is 1 byte or 8 bit in length, since fault
 * space memory is byte-addressed.
 * It may however only inject a subset of these 8 bits, more
 * specifically, all bits that are set in m_mask
 */

class FaultSpaceElement {
protected:
	FaultSpaceArea* m_area;
	const fsp_offset_t m_offset;

public:
	FaultSpaceElement(FaultSpaceArea* area, fsp_offset_t offset)
		: m_area(area), m_offset(offset)
		{ }

	// copy + move constructor
	FaultSpaceElement(const FaultSpaceElement& e) = default;
	FaultSpaceElement(FaultSpaceElement&& e) = default;
	FaultSpaceElement() = delete;

	virtual ~FaultSpaceElement() { }

	FaultSpaceArea* get_area() const { return m_area; };
	void  set_area(FaultSpaceArea* a) { m_area = a; }

	fsp_offset_t get_offset() const { return m_offset; }

	// !< Calculate the absolute fault space address
	fsp_address_t get_address() const {
		return this->get_area()->get_offset() + this->get_offset();
	}

	inline bool operator==(const FaultSpaceElement& rhs) const {
		return m_area == rhs.get_area() && m_offset == rhs.get_offset();
	}

	/**
	 * Compare two fault space elements for their total ordering
	 */
	bool operator<(const FaultSpaceElement& other) const {
		return this->get_address() < other.get_address();
	}

	// each fault space element is at most 8 bit in length, thus its
	// value and/orinjected may also only be at most 8 bit long.
	typedef uint8_t injector_value;
	struct injector_result {
		uint8_t original;
		uint8_t injected;
	};
	typedef std::function<injector_value(injector_value)> injector_fn;


	/** Inject the given element using the injector function.
	 * The injector function takes the original value and returns
	 * the injected value.
	 * Implementations of this function should then wrap both values inside the
	 * injector_result_t struct and return them.
	 * NOTE: this should be pure virtual, but if it is, element is not copy constructible.
	 *       instead we provide a default implementation, here which throws an
	 *       exception.
	 */
	virtual injector_result inject(injector_fn injector);

	/**
	 * Describe this element for output in an ostream.
	 * This function is called by the element ostream<<
	 * operator to describe this element instance.
	 * May be override in subclasses to provide a more
	 * in-depth description.
	 * */
	virtual std::ostream&  dump(std::ostream & os) const {
		return (os << "{ generic element "
				<< " @ "
				<< std::hex << std::showbase
				<< get_offset()
				<< " in area '"
				<< get_area()->get_name()
				<< "' }");
	}
};


/**
 * Output a faultspace element to an ostream It uses the virtual
 * get_description() function so that element subclasses may provde
 * their own description
 */
inline std::ostream& operator<<(std::ostream& os, const FaultSpaceElement& e) {
	return e.dump(os);
}

/**
 * This class is a fault space abstraction. It contains one or more
 * memory areas, who themselves can encode or decode elements into or
 * from this fault space. 
 *
 * To generate an element, i.e. when importing a trace, subclass
 * FaultSpaceArea for your use case and provide a generation method and
 * a size for your fault space area. Finally pass the generated element
 * to this classes encode function to get an absolute fault space address.
 *
 * To decode a fault space address an element, i.e. when executing an
 * experiment, use the provided decode method, which automatically returns
 * an element which can be injected.
 *
 * It is possible to have multiple fault space instances, but within
 * one instance of FAIL*, they are all consistent with each other.
 *
 * NOTE: make sure all your areas mapped with this abstraction fit in
 * sizeof(address_t) (currently 64-Bit). Otherwise the generated map might
 * not work correctly and this abstraction will yield weird results.
 */
class BaseFaultSpace {
private:
	std::map<fsp_address_t, std::unique_ptr<FaultSpaceArea>> m_areas;
	fsp_address_t bumping_ptr = 0;

protected:
	void set_point(fsp_address_t point);

	void add_area(std::unique_ptr<FaultSpaceArea> area);
public:
	BaseFaultSpace() {};
	virtual ~BaseFaultSpace() { }



	// return an area matching the provided name
	// used to create elements in this area
	FaultSpaceArea& get_area(const std::string& name);

	// recreate an element from a previously generated fault space address
	std::unique_ptr<FaultSpaceElement> decode(fsp_address_t addr);
};

} /* namespace fail */
