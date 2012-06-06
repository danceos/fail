#include "ProtoStream.hpp"

using namespace std;

ProtoOStream::ProtoOStream(std::ostream *outfile) : m_outfile(outfile)
{
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
}

bool ProtoOStream::writeMessage(google::protobuf::Message* m)
{
	m_size = htonl(m->ByteSize());
	m_outfile->write(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	
	if (m_outfile->bad()) {
		m_log << "Could not write to file!" << endl;
		return false;
	}
	
	m->SerializeToOstream(m_outfile);
	
    return true;
}


//PROTOISTREAM

ProtoIStream::ProtoIStream(std::istream *infile) : m_infile(infile)
{
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
}

void ProtoIStream:: reset()
{
	m_infile->clear();
	m_infile->seekg(0,ios::beg);
}

bool ProtoIStream::getNext(google::protobuf::Message* m)
{	
	m_infile->read(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	if (!m_infile->good())
		return false;
	m_size = ntohl(m_size);
	
	//FIXME: This could be inefficient because for each data buf is 
	//allocated each time new
	char *buf = new char[m_size];
	m_infile->read(buf, m_size);
	if (!m_infile->good())
		return false;	
	std::string st(buf, m_size);
	m->ParseFromString(st);
	
	delete[] buf;
	return true;
}
