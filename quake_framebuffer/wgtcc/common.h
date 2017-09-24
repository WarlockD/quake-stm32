#ifndef _COMMON_H_
#define _COMMON_H_



#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string_view>


#include "symbol.h"

// path helper
// cause I am trying my best NOT to include boost librarys, and even if I go c++17, visual studio 2015 dosn't yet support it.
class File {
public:
	enum class Type {
		File,
		Directory,
		NotValid
	};
	struct FileAttributes {
		std::streamsize size;
		Type type;
	};
	using vector_type = std::vector<uint8_t>;
	static constexpr int path_sep = '\\'; 
	static constexpr inline bool isPathSep(int c) { return c == '/' || c == '\\'; }
	static FileAttributes ReadFileAttribute(const std::string& filename);
	static std::shared_ptr<vector_type> ReadFile(Symbol filename);
	static void FixPath(std::string& filename);
	static std::string GetExtension(const std::string& filename);
	static std::string GetFileName(const std::string& path);
	static std::string GetDirectory(const std::string& filename);
	Symbol path() const { return _path; }

	bool isDirectory() const { return _attributes.type == Type::Directory; }
	bool isFile() const { return _attributes.type == Type::File; }
	bool exists() const { return _attributes.type != Type::NotValid; }

	std::string_view getExtension() const;
	Symbol getPath() const; // path to directoy the file is in or the current directory
	std::vector<File> getPathComponents() const; // the path in seperate components
	std::string_view getFileName() const; // just get the filename part
	File getUpDirectory() const; // get the directory before this one
	// return file list of current directory or if this is a directory, the list in that
	std::vector<File> filelist() const;
	File(Symbol filename);
	File() =  default;
	bool empty() const { return _data && _data->size(); }
	size_t size() const { return _attributes.size; }
	// interface
	auto operator[](size_t i) { return _data->operator[](i); }
	auto operator[](size_t i) const { return _data->operator[](i); }
	auto at(size_t i) { return _data->at(i); }
	auto at(size_t i) const { return _data->at(i); }
	auto begin() { return _data->begin(); }
	auto end() { return _data->end(); }
	auto begin() const { return _data->begin(); }
	auto end() const { return _data->end(); }
	auto rbegin() { return _data->rbegin(); }
	auto rend() { return _data->rend(); }
	auto rbegin() const { return _data->rbegin(); }
	auto rend() const { return _data->rend(); }
	auto data() { return _data->data(); }
	auto data() const { return _data->data(); }
	void open(); // open and read the file
public:
	FileAttributes _attributes;
	Symbol _path;
	std::shared_ptr<vector_type> _data;
};




#endif
