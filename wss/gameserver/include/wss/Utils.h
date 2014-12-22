#pragma once

namespace wss {

class Utils {
public:

	static inline size_t XYToIndex(const size_t x, const size_t y, const size_t width) {
	return y * width + x;
	}

	static inline size_t XYToIndex(const glm::vec2 &position, const size_t width) {
		return (size_t)(position.y) * width + size_t(position.x);
	}


	static inline void indexToXY(const size_t index, const size_t width, glm::vec2 &position) {
		position.x = index % width;
		position.y = index / width;
	}

	static inline void indexToXY(const size_t index, const size_t width, size_t &x, size_t &y) {
		x = index % width;
		y = index / width;
	}


};

}
