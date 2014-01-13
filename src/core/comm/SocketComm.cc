#include <string>
#include <errno.h>
#include <signal.h>

#include "SocketComm.hpp"

namespace fail {

void SocketComm::init()
{
	// It's usually much easier to handle the error on write(), than to do
	// anything intelligent in a SIGPIPE handler.
	signal(SIGPIPE, SIG_IGN);
}

bool SocketComm::sendMsg(int sockfd, google::protobuf::Message& msg)
{
    int size = htonl(msg.ByteSize());
    std::string buf;
    if (safe_write(sockfd, &size, sizeof(size)) == -1
	 || !msg.SerializeToString(&buf)
	 || safe_write(sockfd, buf.c_str(), buf.size()) == -1) {
        return false;
    }
    return true;
}
  
bool SocketComm::rcvMsg(int sockfd, google::protobuf::Message& msg)
{
    int size;
    if (safe_read(sockfd, &size, sizeof(size)) == -1) {
        return false;
    }
    size = ntohl(size);
    char *buf = new char[size];
    if (safe_read(sockfd, buf, size) == -1) {
        delete [] buf;
        return false;
    }
    std::string st(buf, size);
    delete [] buf;
    return msg.ParseFromString(st);
}

ssize_t SocketComm::safe_write(int fd, const void *buf, size_t count)
{
	ssize_t ret;
	const char *cbuf = (const char *) buf;
	do {
		ret = write(fd, cbuf, count);
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		count -= ret;
		cbuf += ret;
	} while (count);
	return cbuf - (const char *)buf;
}

ssize_t SocketComm::safe_read(int fd, void *buf, size_t count)
{
	ssize_t ret;
	char *cbuf = (char *) buf;
	do {
		ret = read(fd, cbuf, count);
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		} else if (ret == 0) {
			// this deliberately deviates from read(2)
			return -1;
		}
		count -= ret;
		cbuf += ret;
	} while (count);
	return cbuf - (const char *) buf;
}

} // end-of-namespace: fail
