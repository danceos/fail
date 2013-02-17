#ifndef __GEM5_CONNECTOR_HPP__
  #define __GEM5_CONNECTOR_HPP__

#include <string>

/**
 * \class Gem5Connector
 * This class will be compiled inside the gem5 context and provides the
 * Gem5Controller a way to call gem5 functions.
 */
class Gem5Connector {
public:
	void save(const std::string &path);
	void restore(const std::string &path);
};

extern Gem5Connector connector;

#endif // __GEM5_CONNECTOR_HPP__
