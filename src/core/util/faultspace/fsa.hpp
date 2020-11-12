#pragma once

#include "util.hpp"
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <sstream>

namespace fail {
    namespace util {
        namespace fsp {
            using std::unique_ptr;
            using std::vector;
            /**
             * Abstraction for a specific area inside the fault space.
             * The given implementation can translate existing fault space elements
             * which belong to this area into absolute fault space addresses.
             * When subclassed, the specific subclass needs to provide a way to generate
             * fault space elements, which then can be mapped through the base class.
             * Additionally the subclass must provide function for decoding an absolute
             * fault space address to a injectable fault space element and a method for
             * getting its size.
             */
            class element;
            class area {
                private:
                protected:
                    const std::string m_name;
                    address_t m_mapped_at;
                public:
                    const std::string& get_name() const { return m_name; }

                    // Returns the size of this area.
                    virtual const address_t get_size() const = 0;
                    // Convert an existing fault space element into an *absolute* address inside
                    // the fault space.
                    virtual address_t encode(unique_ptr<element> e) const;

                    // recreate a previsouly encoded element from the fault space address
                    // the address passed to this function will be relative to the fault areas
                    // mapping.
                    virtual unique_ptr<element> decode(address_t addr) = 0;

                    void set_offset(address_t addr) { m_mapped_at = addr; }
                    address_t get_offset() const { return m_mapped_at; }

                    area(const std::string name): m_name(std::move(name)) { }
                    area(area&) = delete;
                    area(const area&) = delete;
                    virtual ~area() { }
            };
            /**
             * Abstraction for a singular element inside the fault space.
             * It belongs to an area, and saves additional information to the
             * point in the fault space such as: injection mask, access type and if this
             * a known_outcome event.
             * NOTE: This is copy constructible, but this functionality should only
             *       be used, if the element is not used for injecting since the inject
             *       method is virtual and overriden in specific subclasses.
             * every fault space element is 1 byte or 8 bit in length, since fault
             * space memory is byte-addressed.
             * It may however only inject a subset of these 8 bits, more
             * specifically, all bits that are set in m_mask
             */
            class element {
                protected:
                    area* m_area;
                    const address_t m_mapped_at;

                public:

                    area* get_area() const { return m_area; };
                    void set_area(area* a) { m_area = a; }
                    const address_t get_offset() const { return m_mapped_at; }

                    inline bool operator==(const element& rhs) const {
                        // no need to compare names (area, offset, len) specifies fsp::element completely
                        return m_area == rhs.get_area() &&
                               m_mapped_at == rhs.get_offset();
                    }

                    bool operator<(const element& rhs) const;

                    /* Inject the given element using the injector function.
                     * The injector function takes the original value and returns
                     * the injected value.
                     * Implementations of this function should then wrap both values inside the
                     * injector_result_t struct and return them.
                     * NOTE: this should be pure virtual, but if it is, element is not copy constructible.
                     *       instead we provide a default implementation, here which throws an
                     *       exception.
                     */
                    virtual injector_result inject(injector_fn injector) {
                        throw std::runtime_error("can't inject a non-subclassed fault space element");
                    };

                    /**
                     * Describe this element for output in an ostream.
                     * This function is called by the element ostream<<
                     * operator to describe this element instance.
                     * May be override in subclasses to provide a more
                     * in-depth description.
                     * */
                    virtual std::string get_description() const {
                        std::stringstream out;
                        out << "{ generic element "
                            << " @ "
                            << std::hex << std::showbase
                            << get_offset()
                            << " in area '"
                            << get_area()->get_name()
                            << "' }";
                        return out.str();
                    }

                    // copy + move constructor
                    element(const element& e) = default;
                    element(element&& e) = default;
                    element() = delete;

                    // the default element constructor
                    element(area* area, address_t mapped_at):
                        m_area(area),
                        m_mapped_at(mapped_at)
                        { }

                    virtual ~element() { }
            };


            /**
             * Encode an element of *this* fault space area to an absolute address
             * inside the fault space.
             * NOTE: this takes ownership of the element, since it acts as a sink.
             */
            inline address_t area::encode(unique_ptr<element> e) const {
                assert(e->get_area() == this);
                return get_offset() + e->get_offset();
            }
            /**
             * Compare two fault space elements for their total ordering
             */
            inline bool element::operator<(const element& other) const {
                address_t lhs = get_area()->get_offset() + get_offset();
                address_t rhs = other.get_area()->get_offset() + other.get_offset();
                return lhs < rhs;
            }
            /**
             * Output a faultspace element to an ostream
             * It uses the virtual get_description() function
             * so that element subclasses may provde their own description
             */
            inline std::ostream& operator<<(std::ostream& os, const element& e) {
                return os << e.get_description();
            }
            /**
             * Output a wrapped faultspace element (such as these returned by encode/decode
             * to an ostream.
             */
            inline std::ostream& operator<<(std::ostream& os, std::unique_ptr<element>& e) {
                return os << e->get_description();
            }
        }
    }
}
