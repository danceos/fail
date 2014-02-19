#ifndef __EXPERIMENT_DATA_HPP__
#define __EXPERIMENT_DATA_HPP__

#include <string>
#include <google/protobuf/message.h>

namespace fail {

/**
 * \class ExperimentData
 * Container for experiment data with wrapper methods for serialization and deserialization.
 */
class ExperimentData {
protected:
	google::protobuf::Message* msg;
	uint32_t m_workloadID;
public:
	ExperimentData() : msg(0), m_workloadID(0) {};
	ExperimentData(google::protobuf::Message* m) : msg(m) , m_workloadID(0) { }
	virtual ~ExperimentData() {}

	google::protobuf::Message& getMessage()  { return *msg; }
	void setMessage(google::protobuf::Message *msg)  { this->msg = msg; }
	uint32_t getWorkloadID() const { return m_workloadID; };
	void setWorkloadID(uint32_t id) { m_workloadID = id; }
	/**
	* Serializes the ExperimentData.
	* @param ped output the target-stream.
	* @return \c true if the serialization was successful, \c false otherwise
	*/
	bool serialize(std::ostream* output) const { return msg->SerializeToOstream(output); }
	/**
	* Unserializes the ExperimentData.
	* @param ped input the stream which is read from
	* @return \c true if the unserialization was successful, \c false otherwise
	*/
	bool unserialize(std::istream* input) { return msg->ParseFromIstream(input); }
	/**
	 * Returns a debug string.
	 * @return the debug string
	 */
	std::string debugString() const { return msg->DebugString(); };
};

} // end-of-namespace: fail

#endif //__EXPERIMENT_DATA_HPP__
