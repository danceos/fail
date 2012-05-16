#include "ProtoStream.hpp"

using namespace std;

ProtoStream::ProtoStream(ostream* outfile){
	
	m_write_mode = true;
	m_outfile = outfile;
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
}

ProtoStream::ProtoStream(istream* infile){
	
	m_write_mode = false;
	m_infile = infile;
	m_log.setDescription("ProtoStream");
	m_log.showTime(false);
}

bool ProtoStream::writeMessage(google::protobuf::Message* m){
	
	if(!m_write_mode){
		m_log << "To attach a message, please call the constructor of the ProtoStream-Object with a Pointer of an ostream." << endl;
		return false;
	}
	
	m_size = htonl(m->ByteSize());
	m_outfile->write(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	
	if(m_outfile->bad()){
		m_log << "Could not write to file!" << endl;
	}
	
	m->SerializeToOstream(m_outfile);
	
    return true;
}


bool ProtoStream:: reset(){

	if(m_write_mode){
		m_log << "To reset, please call the constructor of the ProtoStream-Object with a Pointer of an istream." << endl;
		return false;
	}
	
	m_infile->seekg(0,ios::beg);
	return true;
}

bool ProtoStream::getNext(google::protobuf::Message* m){
	
	if(m_write_mode){
		m_log << "To get a message, please call the constructor of the ProtoStream-Object with a Pointer of an istream." << endl;
		return false;
	}
	
	m_infile->read(reinterpret_cast<char*>(&m_size), sizeof(m_size));
	
	if(m_infile->bad()){
		m_log << "Could not write to file!" << endl;
	}
	
	m_size = ntohl(m_size);
	
	char *buf = new char[m_size];
	
	m_infile->read(buf, m_size);
	
	std::string st(buf, m_size);
	
	m->ParseFromString(st);
	
	
	return true;
}
