
// Author: Adrian Böckenkamp
// Date:   07.09.2011

#include "Register.hpp"

namespace sal
{
	Register* UniformRegisterSet::getRegister(size_t i)
	{
		assert(i < m_Regs.size() && "FATAL ERROR: Invalid index provided!");
		return (m_Regs[i]);
	}

	Register* RegisterManager::getRegister(size_t i)
	{
		assert(i < m_Registers.size() && "FATAL ERROR: Invalid index provided!");
		return (m_Registers[i]);
	}
	
	void RegisterManager::add(Register* reg)
	{
		assert(!reg->isAssigned() && "FATAL ERROR: The register is already assigned!");
		m_Registers.push_back(reg);

		UniformRegisterSet* urs = getSetOfType(reg->getType());
		if(urs == NULL) {
			urs = new UniformRegisterSet(reg->getType());
			m_Subsets.push_back(urs);
		}
		urs->m_add(reg);
	}
	
	void UniformRegisterSet::m_add(Register* preg)
	{
		assert(!preg->m_Assigned &&
			"FATAL ERROR: The register has already been assigned.");
		m_Regs.push_back(preg);
		preg->m_Assigned = true;
		preg->m_Index = m_Regs.size()-1; // the index within the vector (set)
	}
	
	size_t RegisterManager::count() const
	{
		return (m_Registers.size());
	}
	
	UniformRegisterSet& RegisterManager::getSet(size_t i)
	{
		assert(i < m_Subsets.size() && "FATAL ERROR: Invalid index provided!");
		return (*m_Subsets[i]);
	}
	
	UniformRegisterSet* RegisterManager::getSetOfType(RegisterType t)
	{
		for(std::vector< UniformRegisterSet* >::iterator it = m_Subsets.begin();
		    it != m_Subsets.end(); it++)
		{
			if((*it)->getType() == t)
				return (*it);
		}
		return (NULL);
	}
	
	void RegisterManager::clear()
	{
		for(std::vector< UniformRegisterSet* >::iterator it = m_Subsets.begin();
		    it != m_Subsets.end(); it++)
			delete (*it);
		m_Subsets.clear();
	}
}
