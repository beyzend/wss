#include "wss/Map.h"

#include <fstream>
#include <exception>

using namespace wss;
using namespace std;

Map::Map(const std::string &fileName) {

	cout << "read map file: " << fileName << endl;
	ifstream inFile(fileName, ios::binary | ios::in);
	try {
		streampos begin, end;
		begin = inFile.tellg();
		inFile.seekg(0, ios::end);
		end = inFile.tellg();
		inFile.seekg(0, ios::beg);

		auto length = end - begin;
		char* buffer = new char[length];

		inFile.read(buffer, length);

		if (mapDocument.ParseInsitu(buffer).HasParseError())
		{
			throw "Failed parse in situ file.";
		}
	}
	catch (std::string error) {
		cout << "Error parsing map: " << error << endl;
	}
	catch (std::exception e) {
		cout << e.what() << endl;
		inFile.close();
	}
}

Map::~Map() {

}

void Map::printCollisionLayer() {
	TiledMap tiledMap;
	// Layers
	vector<Layer> &layers = tiledMap.layers;
	const rapidjson::Value& layersValue = mapDocument["layers"];
	cout << "layers is array: " << layersValue.IsArray() << endl;
	for (rapidjson::SizeType i = 0; i < layersValue.Size(); ++i) {
		const rapidjson::Value& value = layersValue[i];
		cout << "name: " << value["name"].GetString() << endl;
		Layer layer(layersValue[i]);
		cout << layer << endl;
		layers.push_back(layer);
	}

}


