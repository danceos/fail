#include <string>

#include "SocketComm.hpp"

namespace fail {

bool SocketComm::send_msg(int sockfd, google::protobuf::Message& msg)
{
#ifdef USE_SIZE_PREFIX
    int size = htonl(msg.ByteSize());
    if (write(sockfd, &size, sizeof(size)) != sizeof(size)) {
        return false;
    }
    std::string buf;
    msg.SerializeToString(&buf);
    if (write(sockfd, buf.c_str(), buf.size()) != (ssize_t) buf.size()) {
        return false;
    }
#else
    char c = 0;
    if (!msg.SerializeToFileDescriptor(sockfd) || write(sockfd, &c, 1) != 1) {
        return false;
    }
#endif
    return true;
}
  
bool SocketComm::rcv_msg(int sockfd, google::protobuf::Message& msg)
{
#ifdef USE_SIZE_PREFIX
    int size;
    // FIXME: read() may need to be called multiple times until all data was read
    if (read(sockfd, &size, sizeof(size)) != sizeof(size)) {
        return false;
    }
    size = ntohl(size);
    char *buf = new char[size];
    // FIXME: read() may need to be called multiple times until all data was read
    if (read(sockfd, buf, size) != size) {
        delete[] buf;
        return false;
    }
    std::string st(buf, size);
    delete [] buf;
    msg.ParseFromString(st);
    return true;
#else
    return msg.ParseFromFileDescriptor(sockfd);
#endif
}

} // end-of-namespace: fail
