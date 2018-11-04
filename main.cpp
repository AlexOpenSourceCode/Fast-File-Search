#include <stdio.h>
#include <string>
#include <iostream>

#include <vector>
#include <algorithm>
#include <string> 
#include <string>
#include <sstream>

#include <fstream>

using namespace std;

#include <windows.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

struct List {
	vector<string> files;
	vector<string> folders;
};




struct List lsfiles(string folder) {
	vector<string> files;
	vector<string> folders;
	char search_path[200];
	sprintf_s(search_path, "%s*.*", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder, delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				files.push_back(fd.cFileName);
			}
			else {
				folders.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	List me;
	me.files = files;
	me.folders = folders;

	return me;
}



//lowercase string in place
void lowercase_string_in_place(string& str){
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}


string lowercase_string(const string& str){
	string lower_str = str;
	lowercase_string_in_place(lower_str);
	return lower_str;
}


//checks if lowercased str2 is in lowercased str1
bool lower_in(const string& str1, const string& str2){
	size_t str2_indx = lowercase_string(str1).find(lowercase_string(str2));
	if (str2_indx != std::string::npos){
		return true;
	}
	else{
		return false;
	}
}

//Splits string at delimeter and returns 
//vector<string> of each part
//#include <sstream>
vector<string> split(const string &s, char delim) {
	vector<string> elems;
	stringstream ss(s);
	string number;
	while (getline(ss, number, delim)) {
		elems.push_back(number);
	}
	return elems;
}






void search_dir(string path, string search_string, bool recursive = false, int depth = 0, int max_depth = 2){
	List this_dir_list = lsfiles(path); //Get contents of directory


	for (size_t x = 0; x < this_dir_list.folders.size(); x++){
		string directory_name = this_dir_list.folders[x];

		if (lower_in(directory_name, search_string)){
			cout << "[D] " << path << directory_name << " [depth: " << depth << "]" << "\n";
		}
		else{
			if (recursive && depth < max_depth && directory_name != "." && directory_name != ".."){
				search_dir(path + directory_name + "/", search_string, true, depth + 1, max_depth);
			}
		}
	}


	for (size_t x = 0; x < this_dir_list.files.size(); x++){
		string file_name = this_dir_list.files[x];
		if (lower_in(file_name, search_string)){
			cout << "[F] " << path << file_name << " [depth: " << depth << "]" << "\n";
		}
	}
}



bool file_exists(const char *fileName) {
	std::ifstream infile(fileName);
	return infile.good();
}

bool file_exists(string fileName) {
	return file_exists(fileName.c_str());
}



class JSONDB {
	string file_path;
	Document json_obj;

public:
	JSONDB(const string& _file_path){
		file_path = _file_path;
		load();
	}


	void reset(){
		json_obj.Parse("{}");
		save();
	}


	void load(){
		ifstream file;
		file.open(file_path);

		if (file.is_open()){
			//std::stringstream buffer;
			//buffer << file.rdbuf();

			//string json_str = buffer.str();
			std::string json_str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			//cout << "FILE JSON STR: " << json_str << endl;

			if (json_str == ""){
				//cout << "SETTING JSON STR TO: {}" << endl;
				//json_str = "{}";
				reset();
			}
			else{
				try{
					json_obj.Parse(json_str.c_str());
					//cout << "PARSED JSON_OBJ PROPERLY" << endl;
				}
				catch (...){
					reset();
				}
			}
		}
		else{
			cout << "COULDNT OPEN FILE " << endl;
			reset();
		}

		file.close();
	}

	void save(){
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		json_obj.Accept(writer);

		std::ofstream ofs;
		ofs.open(file_path, std::ofstream::out);

		ofs << buffer.GetString();
		ofs.close();
	}


	Value* get(string key){
		if (json_obj.HasMember(key.c_str())){
			return &(json_obj[key.c_str()]);
		}
		return NULL;
	}
};



int main(int argc, char *argv[]) {
	//Add priority directories to optimize search speed. New directories can be added below or in data.json
	vector<string> priority_directories = { "C:/Users/Bob/Documents/" };

	int max_recurse_depth = 3;
	int normal_recurse_depth = 2;


	string data_file_name = "data.json";

	if (file_exists(data_file_name)){
		JSONDB jsondb(data_file_name);

		Value* max_recurse_depth_value = jsondb.get("max_recurse_depth");
		Value* normal_recurse_depth_value = jsondb.get("normal_recurse_depth");
		Value* priority_directories_value = jsondb.get("priority_directories");

		if (max_recurse_depth_value != NULL && normal_recurse_depth_value != NULL && priority_directories_value != NULL){
			max_recurse_depth = max_recurse_depth_value->GetInt();
			normal_recurse_depth = normal_recurse_depth_value->GetInt();

			for (Value::ConstValueIterator itr = priority_directories_value->Begin(); itr != priority_directories_value->End(); ++itr){
				priority_directories.push_back(itr->GetString());
			}

			cout << "max_recurse_depth: " << max_recurse_depth << endl;
			cout << "normal_recurse_depth: " << normal_recurse_depth << endl;
		}
	}
	


	while (true){
		string search_string = "";
		cout << "search: ";

		//Takes input before first space
		//cin >> search_string;
		char input[100];
		//Allows cin to take spaces
		cin.getline(input, sizeof(input));
		string input_string = input;
		
		bool recursive = false;
		int recurse_depth = normal_recurse_depth;

		

		char split_char = ' ';
		vector<string> input_parts = split(input_string, split_char);

		bool is_search_string_part = true;
		string search_specific_path_string = "";

		for (size_t x = 0; x < input_parts.size(); x++){
			const string& input_part = input_parts[x];

			if (input_part == "-r"){
				is_search_string_part = false;
				recursive = true;

				if (x + 1 < input_parts.size()){
					try{
						recurse_depth = stoi(input_parts[x + 1]);
					}
					catch (...){}
					x += 1;
				}

				if (recurse_depth == -1){
					recurse_depth = max_recurse_depth;
				}
			}
			else if (input_part == "-d"){
				is_search_string_part = false;

				if (x + 1 < input_parts.size()){
					search_specific_path_string = input_parts[x + 1];
				}		
				x += 1;	
			}

			if (is_search_string_part){
				if (x == 0){
					search_string += input_part;
				}
				else{
					search_string += split_char + input_part;
				}	
			}
		}

		cout << "=== Searching for '" << search_string << "'" << endl;
		if (recursive){
			cout << "recurse_depth: " << recurse_depth << endl;
		}
		
		if (search_specific_path_string != ""){
			cout << "search_specific_path_string: " << search_specific_path_string << endl;

			search_dir(search_specific_path_string, search_string, recursive, 0, recurse_depth);
		}
		else{
			for (size_t z = 0; z < priority_directories.size(); z++){
				string path = priority_directories[z];

				search_dir(path, search_string, recursive, 0, recurse_depth);
			}
		}
		cout << "===" << endl;
	}
	




	std::system("pause");
	return 0;
}