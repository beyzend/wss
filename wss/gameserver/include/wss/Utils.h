#pragma once

namespace wss {

class Utils {
public:

	static inline size_t XYToIndex(const int x, const int y, const size_t width) {
	return y * width + x;
	}

	static inline size_t XYToIndex(const glm::vec2 &position, const size_t width) {
		return size_t((position.y) * width + size_t(position.x));
	}


	static inline void indexToXY(const size_t index, const size_t width, glm::vec2 &position) {
		position.x = int(index % width);
		position.y = int(index / width);
	}

	static inline void indexToXY(const size_t index, const size_t width, int &x, int &y) {
		x = index % width;
		y = index / width;
	}


};

}
