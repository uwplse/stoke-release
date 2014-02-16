#ifndef STOKE_SRC_ARGS_TESTCASE_H
#define STOKE_SRC_ARGS_TESTCASE_H

#include <vector>

#include "src/ext/cpputil/include/serialize/text_reader.h"
#include "src/ext/cpputil/include/serialize/text_writer.h"
#include "src/state/cpu_state.h"

namespace cpputil {

template <typename Style>
struct TextReader<std::vector<stoke::CpuState>, Style> {
	void operator()(std::istream& is, std::vector<stoke::CpuState>& ts) {
		ts.clear();
		for ( std::string s; !((is >> s).eof()); ) {
			stoke::CpuState t;
			is >> s >> t;
			ts.emplace_back(t);
		}
		if ( is.eof() ) {
			is.clear(std::ios::eofbit);
		}	
	}
};

template <typename Style>
struct TextWriter<std::vector<stoke::CpuState>, Style> {
	void operator()(std::ostream& os, const std::vector<stoke::CpuState>& ts) {
		for ( size_t i = 0, ie = ts.size(); i < ie; ++i ) {
			os << "Testcase " << i << ":" << std::endl << std::endl;
			os << ts[i] << std::endl << std::endl;
		}
	}
};

} // namespace cpputil

#endif
