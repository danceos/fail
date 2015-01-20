#include "util/SumTree.hpp"
#include "util/Logger.hpp"

#include <iostream>

using std::endl;

fail::Logger LOG("SumTreeTest");

struct Pilot {
	uint32_t id;
	uint32_t instr2;
	uint32_t data_address;
	uint64_t duration;

	typedef uint64_t size_type;
	size_type size() const { return duration; }
};

int main()
{
	typedef fail::SumTree<Pilot, 2> sumtree_type;
	sumtree_type tree;
	for (int i = 0; i <= 20; ++i) {
		Pilot p;
		p.duration = i;
		tree.add(p);
	}

	LOG << "tree contents:" << endl;
	for (sumtree_type::iterator it = tree.begin(); it != tree.end(); ++it) {
		LOG << it->size() << endl;
	}

	while (tree.get_size() > 0) {
		uint64_t pos = tree.get_size() / 2;
		LOG << "MAIN tree.get_size() = " << tree.get_size()
			<< ", trying to retrieve pos = " << pos << endl;
		Pilot p = tree.remove(pos);
		LOG << "MAIN retrieved pilot with duration " << p.duration << endl;
	}
}
