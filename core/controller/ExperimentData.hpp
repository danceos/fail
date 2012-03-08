/**
 * \brief ExperimentData interface
 * 
 * This is the base class for all user-defined data types for
 * expirement parameter and results.
 *
 * \author Martin Hoffmann, Richard Hellwig
 *
 */

#ifndef __EXPERIMENT_DATA_H__
#define __EXPERIMENT_DATA_H__

#include <string>
#include <google/protobuf/message.h>
using namespace std;

namespace fi{

/**
 * \class ExperimentData
 * Container for experiment data with wrapper methods for serialization and deserialization.
 */

	class ExperimentData	
	{
	  protected:
		  google::protobuf::Message* msg;
		  uint32_t m_workloadID;
	  public:
	  	  ExperimentData() : msg(0), m_workloadID(0) {};
		  ExperimentData(google::protobuf::Message* m) : msg(m) , m_workloadID(0) {};
		
		 google::protobuf::Message& getMessage()  { return *msg; };
		 uint32_t getWorkloadID() const { return m_workloadID;}; 
		 void setWorkloadID(uint32_t id) { m_workloadID = id; };
		/**
		 * Serializes the ExperimentData.
		 * @param ped output the target-stream.
		 * @return \c true if the serialization was successful, \c false otherwise
		 */
		  bool serialize(ostream * output) const { return msg->SerializeToOstream(output); }
		/**
		 * Unserializes the ExperimentData.
		 * @param ped input the stream which is read from
		 * @return \c true if the unserialization was successful, \c false otherwise
		 */
		  bool unserialize(istream * input) { return msg->ParseFromIstream(input); }
		  string DebugString() const { return msg->DebugString(); };
	};

};

#endif //__EXPERIMENT_DATA_H__


