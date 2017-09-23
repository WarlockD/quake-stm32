#include "common.h"
#include <sys\stat.h>
#include <assert.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <mutex>


File::FileAttributes File::ReadFileAttribute(const std::string& filename) {
	WIN32_FILE_ATTRIBUTE_DATA file_info;
	FileAttributes ret = { 0, Type::NotValid };
	// Get file info
	if (GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &file_info))
	{
		if (file_info.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY)
			ret.type = Type::Directory;
		else {
			ret.type = Type::File;
			ret.size = static_cast<std::streamsize>(file_info.nFileSizeLow);
			assert(file_info.nFileSizeHigh == 0);
			if (file_info.nFileSizeHigh > 0) { // not sure if a text file should be this big?:P
				ret.size += static_cast<std::streamsize>(file_info.nFileSizeHigh) << 32;
			}
		}
	}
	return ret;
}
 std::shared_ptr<File::vector_type> File::ReadFile(Symbol filename) {
	 static std::unordered_map<Symbol, std::weak_ptr<vector_type>> data_cache;
	 static std::mutex data_mutex;
	 
	 auto it = data_cache.find(filename);
	 if (it != data_cache.end()) return it->second.lock();
	 else {
		 std::lock_guard<std::mutex> lock(data_mutex);
		 std::shared_ptr<File::vector_type> ret;
		 auto attr = ReadFileAttribute(filename);

		 if (attr.type != Type::File || attr.size == 0) return ret;
		 ret.reset(new vector_type(attr.size), [filename](vector_type* ptr) { 
			 std::lock_guard<std::mutex> lock(data_mutex);  
			 data_cache.erase(filename); 
		 });
		 std::ifstream f(filename);
		 f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		 assert(f.is_open());
		 if (f.is_open()) {
			 f.read((char*)ret->data(), ret->size());
		 }
		 f.close();
		 data_cache[filename] = ret;
		 return ret;
	 }
}

void File::FixPath(std::string& filename) {
	for (auto it = filename.begin(); it != filename.end(); it++) {
		if (isPathSep(*it) &&   != *it) *it = path_sep;
	}
}
 std::string File::GetExtension(const std::string& filename) {
	for (auto it = filename.rbegin(); it != filename.rend(); it++) {
		if(isPathSep(*it)) break;
		else if (*it == '.') return std::string(it.base(), filename.end());
	}
	return std::string(); // returns an empty string, no extension
}

 std::string File::GetFileName(const std::string& filename) {
	 for (auto it = filename.rbegin(); it != filename.rend(); it++) {
		 if (isPathSep(*it)) return std::string(it.base(), filename.end());
	 }
	 return std::string(); // returns an empty string, no extension
}
 File::File(const std::string& path) : _path(GetDirectory(path)) {

	 struct stat stat;
	 assert(_stat(_path.c_str(), &stat) == 0);
	 assert(stat.st_mode & _S_IFDIR);
	 FixPath(_path);
 }
 

File::vector_type File::ReadFile(const std::string& filename) {

 }

File::File(const std::string& filename) : _data(new vector_type(ReadFile(filename))), : _filename(filename) {}
