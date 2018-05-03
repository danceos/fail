#include <iostream>
#include <sstream>
#include <set>
#include <stdlib.h>
#include "util/MemoryMap.hpp"

using namespace fail;
using std::cerr;
using std::endl;

#define LEN(arr) (sizeof(arr)/sizeof(*arr))

// start, size, start, size, ...
uint32_t inside[] = { 10, 5, 11, 1, 12, 1, 15, 1, 17, 1, 19, 1, 18, 2, 23, 2, 22, 1, 21, 1 };
uint32_t outside[] = { 0, 10, 16, 1, 20, 1, 25, 10 };

void test_failed(std::string msg)
{
	cerr << "MemoryMap test failed (" << msg << ")!" << endl;
	abort();
}

// pass by value intentional
void test(MemoryMap mm)
{
	std::stringstream ss;
	for (unsigned i = 0; i < LEN(inside); i += 2) {
		uint32_t start = inside[i], size = inside[i+1];
		uint32_t end = start + size;

		if (!mm.isMatching(start, size)) {
			ss << "1 " << start << " " << size;
			test_failed(ss.str());
			ss.str("");
		}

		for (; start < end; ++start) {
			if (!mm.isMatching(start)) {
				ss << "2 " << start;
				test_failed(ss.str());
				ss.str("");
			}
		}
	}
	for (unsigned i = 0; i < LEN(outside); i += 2) {
		uint32_t start = outside[i], size = outside[i+1];
		uint32_t end = start + size;

		if (mm.isMatching(start, size)) {
			ss << "3 " << start << " " << size;
			test_failed(ss.str());
			ss.str("");
		}

		for (; start < end; ++start) {
			if (mm.isMatching(start)) {
				ss << "4 " << start;
				test_failed(ss.str());
				ss.str("");
			}
		}
	}
	for (MemoryMap::iterator it = mm.begin(); it != mm.end(); ++it) {
		cerr << *it << " ";
		bool found = false;
		for (unsigned i = 0; i < LEN(inside); i += 2) {
			uint32_t start = inside[i], size = inside[i+1];
			uint32_t end = start + size;
			if (*it >= start && *it < end) {
				found = true;
				break;
			}
		}
		if (!found) {
			ss << "5 " << *it;
			test_failed(ss.str());
			ss.str("");
		}
		found = false;
		for (unsigned i = 0; i < LEN(outside); i += 2) {
			uint32_t start = outside[i], size = outside[i+1];
			uint32_t end = start + size;
			if (*it >= start && *it < end) {
				found = true;
				break;
			}
		}
		if (found) {
			ss << "6 " << *it;
			test_failed(ss.str());
			ss.str("");
		}
	}
	cerr << "Test finished" << endl;
}

int main()
{
	MemoryMap mm;
	char const *filename_tmp = "tmp.memorymap";
	char const *filename_test1 = "test1.memorymap";
	char const *filename_test2 = "test2.memorymap";
	char const *filename_test3 = "test3.memorymap";

	for (unsigned i = 0; i < LEN(inside); i += 2) {
		mm.add(inside[i], inside[i+1]);
	}
	mm.dump(cerr);
	
	test(mm);

	if (!mm.writeToFile(filename_tmp)) {
		test_failed("writing tmp file");
	}
	mm.clear();
	if (!mm.readFromFile(filename_tmp)) {
		test_failed("reading tmp file");
	}

	mm.dump(cerr);

	test(mm);

	// intentionally omitting mm.clear() here
	if (!mm.readFromFile(filename_test1)) {
		test_failed(filename_test1);
	}
	test(mm);

	mm.clear();
	if (!mm.readFromFile(filename_test1)) {
		test_failed(filename_test1);
	}
	test(mm);

	mm.clear();
	if (!mm.readFromFile(filename_test2)) {
		test_failed(filename_test2);
	}
	test(mm);

	mm.clear();
	if (!mm.readFromFile(filename_test3)) {
		test_failed(filename_test3);
	}
	test(mm);
}
