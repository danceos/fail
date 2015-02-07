/**
 * \brief One way to manage large protobuf-messages
 *
 * Google protobuf is not designed for large messages. That leads to
 * much memory which is consumed by processing large  messages. To
 * solve this problem the ProtoStream class stores the  protobuf-messages
 * in an other way. Instead of using the repeat-function the data is
 * written sequentially in a file. Written in the format:
 *
 * \code
 * | 4 bytes length-information of the first message  | first message
 * | 4 bytes length-information of the second message | second message
 * | ...
 * \endcode
 */

#ifndef __PROTOSTREAM_HPP__
#define __PROTOSTREAM_HPP__

#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <google/protobuf/message.h>

#include "Logger.hpp"

namespace fail {

/**
 * \class ProtoOStream
 *
 * This class can be used to sequentially write a large number of
 * protocol buffer messages to a \c std::ostream.
 */
class ProtoOStream {
private:
	// TODO: comments needed here
	Logger m_log;
	std::ostream* m_outfile;
public:
	ProtoOStream(std::ostream *outfile);
	virtual ~ProtoOStream() { }
	/**
	 *	Writes a message to a file.
	 *  @param m The protobuf-message to be written
	 *  @return Returns \c true on success, \c false otherwise
	 */
	bool writeMessage(google::protobuf::Message *m);
};

/**
 * \class ProtoIStream
 *
 * This class can be used to read protocol buffer messages sequentially
 * from a \c std::istream.
 */
class ProtoIStream {
private:
	// TODO: comments needed here
	Logger m_log;
	std::istream *m_infile;
public:
	ProtoIStream(std::istream *infile);
	virtual ~ProtoIStream() { }
	/**
	 *	Resets the position of the get pointer. After that \c getNext()
	 *  reads the first message again.
	 */
	void reset();
	/**
	 *	Reads the next protobuf message from the input stream.
	 *  @param m The output protobuf message
	 *  @return Returns \c true on success, \c false otherwise
	 */
	bool getNext(google::protobuf::Message * m);
};

} // end-of-namespace: fail

#endif // __PROTOSTREAM_HPP__
