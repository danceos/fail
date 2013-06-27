#include <iostream>
#include <cassert>

#include "CoroutineManager.hpp"
#include "ExperimentFlow.hpp"

namespace fail {

void CoroutineManager::m_invoke(void* pData)
{
	ExperimentFlow *flow = reinterpret_cast<ExperimentFlow*>(pData);
	flow->coroutine_entry();
	simulator.removeFlow(flow);
	//m_togglerstack.pop();
	// FIXME: need to pop our caller
	co_exit(); // deletes the associated coroutine memory as well

	// We really shouldn't get here:
	assert(false && "FATAL ERROR: CoroutineManager::m_invoke() -- shitstorm unloading!");
	while (1); // freeze.
}

CoroutineManager::~CoroutineManager()
{
	// Note that we do not destroy the associated coroutines; this causes
	// problems when shutting down.
	m_Flows.clear();
}

void CoroutineManager::toggle(ExperimentFlow* flow)
{
	assert((co_current() != m_simCoro || flow != SIM_FLOW) &&
		"FATAL ERROR: We are already in the simulators coroutine flow! \
		(Maybe you forgot to overwrite the (default) onTrigger() method?)");
	m_togglerstack.push(co_current());
	//std::cerr << "CORO toggle from " << m_togglerstack.top() << " to ";
	if (flow == SIM_FLOW) {
		co_call(m_simCoro);
		return;
	}

	flowmap_t::iterator it = m_Flows.find(flow);
	assert(it != m_Flows.end() && "FATAL ERROR: Flow does not exist!");
	//std::cerr << it->second << std::endl;
	co_call(it->second);
}

void CoroutineManager::create(ExperimentFlow* flow)
{
	corohandle_t res = co_create(CoroutineManager::m_invoke, flow, NULL,
								 STACK_SIZE_DEFAULT);
	//std::cerr << "CORO create " << res << std::endl;
	m_Flows.insert(std::pair<ExperimentFlow*,corohandle_t>(flow, res));
}

void CoroutineManager::remove(ExperimentFlow* flow)
{
	// find coroutine handle for this flow
	flowmap_t::iterator it = m_Flows.find(flow);
	if (it == m_Flows.end()) {
		// Not finding the flow to remove is not an error; especially when
		// shutting down this is the common case, as ~CoroutineManager probably
		// clears the flow list before the ExperimentFlow destructors run.
		return;
	}
	corohandle_t coro = it->second;
	//std::cerr << "CORO remove " << coro << std::endl;

	// remove flow from active list
	m_Flows.erase(it);

	// FIXME make sure resume() keeps working

	// delete coroutine (and handle the special case we're removing
	// ourselves)
	if (coro == co_current()) {
		if (!m_Terminated) {
			co_exit();
		}
	} else {
		co_delete(coro);
	}
}

void CoroutineManager::resume()
{
	corohandle_t next = m_togglerstack.top();
	m_togglerstack.pop();
	//std::cerr << "CORO resume from " << co_current() << " to " << next << std::endl;
	co_call(next);
}

ExperimentFlow* CoroutineManager::getCurrent()
{
	coroutine_t cr = co_current();
	for (std::map<ExperimentFlow*,corohandle_t>::iterator it = m_Flows.begin();
		it != m_Flows.end(); it++)
		if (it->second == cr)
			return it->first;

	return NULL;
}

const ExperimentFlow* CoroutineManager::SIM_FLOW = NULL;

} // end-of-namespace: fail
