#pragma once

#include "wss.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include <string>
#include <vector>
#include <iostream>

#include "wss/Utils.h"

namespace wss
{

using std::vector;
using std::int32_t;
using std::string;
using std::ostream;
using std::cout;
using std::endl;

struct LayerObject
{
	int32_t gid = 0;
	string name = "";
	int32_t x = 0;
	int32_t y = 0;

	explicit LayerObject(){}

	LayerObject(const rapidjson::Value& objectJSON) {
		gid = objectJSON["gid"].GetInt();
		name = objectJSON["name"].GetString();
		x = objectJSON["x"].GetInt();
		y = objectJSON["y"].GetInt();
	}
};

struct Layer
{
	vector<int32_t> data;
	vector<LayerObject> objects;
	int32_t height;
	string name = "";
	int32_t opacity = 1;
	string type = "";
	bool visible = false;
	int32_t width = 0;
	int32_t x = 0;
	int32_t y = 0;

	explicit Layer(){}

	Layer(const rapidjson::Value& layerJSON) {
		if (layerJSON.HasMember("data")) {
			const rapidjson::Value& dataJSON = layerJSON["data"];
			for (rapidjson::SizeType i = 0; i < dataJSON.Size(); ++i) {
				data.push_back(dataJSON[i].GetInt());
			}
		}
		if (layerJSON.HasMember("objects")) {
			const rapidjson::Value& dataJSON = layerJSON["objects"];
			for (rapidjson::SizeType i = 0; i < dataJSON.Size(); ++i) {
				LayerObject object(dataJSON[i]);
				objects.push_back(object);
			}
		}

		height = layerJSON["height"].GetInt();
		name = layerJSON["name"].GetString();
		opacity = layerJSON["opacity"].GetInt();
		type = layerJSON["type"].GetString();
		visible = layerJSON["visible"].GetBool();
		width = layerJSON["width"].GetInt();
		x = layerJSON["x"].GetInt();
		y = layerJSON["y"].GetInt();
	}

	friend ostream& operator<<(ostream& os, const Layer& layer) {
		os << "height: " << layer.height << " ";
		os << "name: " << layer.name << " ";
		os << "opacity: " << layer.opacity << " ";
		os << "type: " << layer.type << " ";
		os << "visible: " << layer.visible << " ";
		os << "width: " << layer.width << " ";
		os << "x: " << layer.x << " ";
		os << "y: " << layer.y;
		os << "data size: " << layer.data.size();
		os << "objects size: " << layer.objects.size();
		return os;
	}

};

struct Tilesets
{
	std::string image = "";
	std::int32_t imageheight = 0;
	std::int32_t imagewidth = 0;
	std::int32_t tileheight = 0;
	std::int32_t tilewidth = 0;
};

struct TiledMap
{
	std::int32_t height = 0;
	std::int32_t width = 0;
	std::int32_t tileheight = 0;
	std::int32_t tilewidth = 0;
	std::int32_t version = 0;

	std::vector<wss::Layer> layers;
	std::vector<wss::Tilesets> tilesets;
};



class Map {
public:
	Map(const std::string &fileName);
	virtual ~Map();

	const Layer& getCollisionLayer() const;
	const Layer& getAdvertLayer() const;
private:
	Layer _floorLayer;
	Layer _advertisementLayer;
	//TiledMap tiledMap;
};

}
