#ifndef __PROTOSTREAM_HPP__
#define __PROTOSTREAM_HPP__

#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <google/protobuf/message.h>

#include "Logger.hpp"

using namespace std;

/**
 * \class ProtoStream
 * 
 * 
 * \brief One way to manage large protobuf-messages
 * 
 *	Google protobuf is not designed for large messages.
 * 	That leads to much memory which is consumed by processing large messages.
 *	To solve this problem the ProtoStream class stores the protobuf-messages in an other way.
 *	Instead of using the repeat-function the data is written sequentially in a file.
 *	Written in the format: 
 * |4 bytes length-information of the first message| first message | 4 bytes length-information of second message | second message | ...
 *
 */
class ProtoStream
{
	private:
		bool m_write_mode;
		uint32_t m_size;
		
		Logger m_log;
		ostream* m_outfile;
		istream* m_infile;
		
		
	public:
		ProtoStream(ostream * outfile);
		ProtoStream(istream * infile);
		virtual ~ProtoStream() {};
		/**
		 *	Writes a message to a file. 
		 * 	This works only if the constructor was called wit a pointer 
		 *  of an ostream.
		 *  @param m The protobuf-message to be written.
		 *  @return Returns true if data was written.
		 */
		bool writeMessage(google::protobuf::Message* m);
		/**
		 *	Resets the position of the get pointer. After that getNext 
		 *  delivers the first message again.
		 * 	This works only if the constructor was called wit a pointer 
		 *  of an istream.
		 *  @return Returns true if the pointer is reset.
		 */
		bool reset();
		/**
		 *	Delivers the protobuf-messages sequentially from file.
		 * 	This works only if the constructor was called wit a pointer 
		 *  of an istream.
		 *  @param m The protobuf-message in which, the data should be 
		 * 	written.
		 *  @return Returns true if data was written to protobuf-message.
		 */
		bool getNext(google::protobuf::Message* m);
};

#endif
