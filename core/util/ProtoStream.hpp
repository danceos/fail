/** brief One way to manage large protobuf-messages
 * 
 *	Google protobuf is not designed for large messages.
 * 	That leads to much memory which is consumed by processing large 
 *  messages. To solve this problem the ProtoStream class stores the 
 *  protobuf-messages in an other way.
 *	Instead of using the repeat-function the data is written 
 *  sequentially in a file.
 *	Written in the format: 
 * |4 bytes length-information of the first message | first message 
 * |4 bytes length-information of the second message| second message 
 * | ...
 */

#ifndef __PROTOSTREAM_HPP__
#define __PROTOSTREAM_HPP__

#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <google/protobuf/message.h>

#include "Logger.hpp"

/**
 * \class ProtoOStream
 * 
 * This class can be used to write messages in a file.
 *
 */
class ProtoOStream
{
	private:
		uint32_t m_size;
		
		Logger m_log;
		std::ostream* m_outfile;
		
	public:
		ProtoOStream(std::ostream *outfile);
		virtual ~ProtoOStream() {};
		/**
		 *	Writes a message to a file. 
		 *  @param m The protobuf-message to be written.
		 *  @return Returns true if data was written.
		 */
		bool writeMessage(google::protobuf::Message* m);
};


/**
 * \class ProtoIStream
 * 
 * This class can be used to read messages sequentially from a file.
 *
 */
class ProtoIStream
{
	private:
		uint32_t m_size;
		long m_sizeOfInfile;
		
		Logger m_log;
		std::istream *m_infile;
		
		
	public:
		ProtoIStream(std::istream *infile);
		virtual ~ProtoIStream() {};
		/**
		 *	Resets the position of the get pointer. After that getNext 
		 *  delivers the first message again.
		 */
		void reset();
		/**
		 *	Delivers the protobuf-messages sequentially from file.
		 *  @param m The protobuf-message in which, the data should be 
		 * 	written.
		 *  @return Returns true if data was written to protobuf-message.
		 */
		bool getNext(google::protobuf::Message* m);
};

#endif
