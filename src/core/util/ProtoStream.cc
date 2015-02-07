#include "ProtoStream.hpp"

namespace fail {

ProtoOStream::ProtoOStream(std::ostream *outfile) : m_outfile(outfile)
{
	m_log.setDescription("ProtoStream");
	// TODO: log-Level?
	m_log.showTime(false);
}

bool ProtoOStream::writeMessage(google::protobuf::Message *m)
{
	uint32_t m_size = htonl(m->ByteSize());
	m_outfile->write(reinterpret_cast<char*>(&m_size), sizeof(m_size));

	if (m_outfile->bad()) {
		m_log << "Could not write to file!" << std::endl;
		// TODO: log-Level?
		return false;
	}

	m->SerializeToOstream(m_outfile);

	return true;
}

ProtoIStream::ProtoIStream(std::istream *infile) : m_infile(infile)
{
	m_log.setDescription("ProtoStream");
	// TODO: log-Level?
	m_log.showTime(false);
}

void ProtoIStream::reset()
{
	m_infile->clear();
	m_infile->seekg(0, std::ios::beg);
}

bool ProtoIStream::getNext(google::protobuf::Message *m)
{
	uint32_t m_size;
	m_infile->read(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	if (!m_infile->good())
		return false;
	m_size = ntohl(m_size);

	// FIXME reuse buffer (efficiency)
	// FIXME new[] may fail (i.e., return 0)
	char *buf = new char[m_size];
	m_infile->read(buf, m_size);
	if (!m_infile->good())
		// FIXME we're leaking buf[]
		return false;
	std::string st(buf, m_size);
	m->ParseFromString(st);

	delete [] buf;
	return true;
}

} // end-of-namespace: fail
