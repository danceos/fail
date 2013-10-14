/**
 * \file Minion.hpp
 * \brief The representation of a minion.
 */

#ifndef __MINION_HPP__
#define __MINION_HPP__

#include <string>

#include "comm/ExperimentData.hpp"

namespace fail {

/**
 * \class Minion
 *
 * Contains all informations about a minion.
 */
class Minion {
private:
	std::string hostname;
	bool isWorking;
	ExperimentData* currentExperimentData;
	int sockfd;
public:
	Minion() : isWorking(false), currentExperimentData(0), sockfd(-1) { }
	/**
	 * Sets the socket descriptor.
	 * @param sock the new socket descriptor (used internal)
	 */
	void setSocketDescriptor(int sock) { sockfd = sock; }
	/**
	 * Retrives the socket descriptor.
	 * @return the socket descriptor
	 */
	int getSocketDescriptor() const { return (sockfd); }
	/**
	 * Returns the hostname of the minion.
	 * @return the hostname
	 */
	const std::string& getHostname() { return (hostname); }
	/**
	 * Sets the hostname of the minion.
	 * @param host the hostname
	 */
	void setHostname(const std::string& host) { hostname = host; }
	/**
	 * Returns the current ExperimentData which the minion is working with.
	 * @return a pointer of the current ExperimentData
	 */
	ExperimentData* getCurrentExperimentData() { return currentExperimentData; }
	/**
	 * Sets the current ExperimentData which the minion is working with.
	 * @param exp the current ExperimentData
	 */
	void setCurrentExperimentData(ExperimentData* exp) { currentExperimentData = exp; }
	/**
	 * Returns the current state of the minion.
	 * @return the current state
	 */
	bool isBusy() { return (isWorking); }
	/**
	 * Sets the current state of the minion
	 * @param state the current state
	 */
	void setBusy(bool state) { isWorking = state; }
};

} // end-of-namespace: fail

#endif // __MINION_HPP__
