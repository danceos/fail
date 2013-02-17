#include "UDIS86.hpp"

Udis86::Udis86(fail::address_t ip)
: udis_instr(NULL), udis_instr_size(0)
{
	// initialise the internal data structure
	ud_init(&ud_obj);
	ud_set_mode(&ud_obj, 32);
	ud_set_syntax(&ud_obj, UD_SYN_ATT);
	ud_set_pc(&ud_obj, ip);
}

void Udis86::setInputBuffer(unsigned char const *instr, size_t size)
{
	// initialise the buffer
	if (size > udis_instr_size) {
		void *new_instr = realloc(udis_instr, size);
		if (new_instr == NULL) {
			// highly improbable
			return;
		}
		udis_instr = reinterpret_cast<unsigned char*>(new_instr);
	}

	udis_instr_size = size;
	memcpy(udis_instr, instr, udis_instr_size);

	// assign the buffer to the data structure
	ud_set_input_buffer(&ud_obj, udis_instr, udis_instr_size);
}
