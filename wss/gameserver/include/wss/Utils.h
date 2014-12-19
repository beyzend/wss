#pragma once

namespace wss {

class Utils {
public:

	static inline size_t XYToIndex(const size_t x, const size_t y, const size_t width) {
	return y * width + x;
	}

	static inline void indexToXY(const size_t index, const size_t width, size_t &x, size_t &y) {
		x = index % width;
		y = index / width;
	}


};

}
