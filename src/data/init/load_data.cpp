/*
 * load_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */
#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>

using namespace std;

static char* tocstring(const string& s){
	char* ret = new char[s.size()+1];
	memcpy(ret, s.c_str(), s.size()+1);
	return ret;
}

void load_tile_data(const char* filename){

    fstream file(filename, fstream::in | fstream::binary);


	YAML::Parser parser(file);
	YAML::Node root;
	
	
	string name, filen;
	int issolid = 0;
	
	parser.GetNextDocument(root);
	game_tile_data.clear();
	
	const YAML::Node& node = root["Tiles"];

	for(int i = 0; i < node.size(); i++){
		issolid = 0;
		const YAML::Node& entry = node[i];
		entry["tile"] >> name;
		entry["spritefile"] >> filen;
		
		if(const YAML::Node* sptr = entry.FindValue("solid")){
			entry["solid"] >> issolid;
		}
		
		game_tile_data.push_back(TileEntry(tocstring(name) , tocstring(filen), issolid > 0));
	}
	for (int i = 0; i < game_tile_data.size();i++){
		game_tile_data[i].init();
	}
}

void load_sprite_data(const char* filename){
	
    fstream file(filename, fstream::in | fstream::binary);


	YAML::Parser parser(file);
	YAML::Node root;
	
	
	string name, filen;
	int issolid = 0;
	
	parser.GetNextDocument(root);
	game_sprite_data.clear();
	
	const YAML::Node& node = root["Sprites"];

	for(int i = 0; i < node.size(); i++){
		issolid = 0;
		const YAML::Node& entry = node[i];
		entry["sprite"] >> name;
		entry["spritefile"] >> filen;
		
		game_sprite_data.push_back(SpriteEntry(tocstring(name) , tocstring(filen)));
	}
	for (int i = 0; i < game_sprite_data.size();i++){
		game_sprite_data[i].init();
	}
}
