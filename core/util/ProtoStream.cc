#include "ProtoStream.hpp"

using namespace std;

ProtoOStream::ProtoOStream(ostream* outfile)
{
	m_outfile = outfile;
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
}

bool ProtoOStream::writeMessage(google::protobuf::Message* m)
{
	m_size = htonl(m->ByteSize());
	m_outfile->write(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	
	if(m_outfile->bad()) {
		m_log << "Could not write to file!" << endl;
		return false;
	}
	
	m->SerializeToOstream(m_outfile);
	
    return true;
}


void ProtoIStream:: reset()
{
	m_infile->seekg(0,ios::beg);
}


//PROTOISTREAM

ProtoIStream::ProtoIStream(istream* infile)
{
	m_infile = infile;
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
	
	m_infile->seekg(0, ios::end);
	m_sizeOfInfile = m_infile->tellg();
	m_infile->seekg(0, ios::beg);
}

bool ProtoIStream::getNext(google::protobuf::Message* m)
{
	if(m_infile->tellg() >= m_sizeOfInfile) {
		return false;
	}
	
	m_infile->read(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	m_size = ntohl(m_size);
	char *buf = new char[m_size];
	m_infile->read(buf, m_size);
	std::string st(buf, m_size);
	m->ParseFromString(st);
	
	delete[] buf;
	return true;
}
