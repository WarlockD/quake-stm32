#include "common.h"
#include <sys\stat.h>
#include <assert.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <mutex>

static std::unordered_map<Symbol, std::weak_ptr<File::vector_type>> data_cache;
static std::mutex data_mutex;

template<typename T>
File::FileAttributes ReadFileAttributeTaslate(const T& file_info) {
	File::FileAttributes ret = { 0, File::Type::NotValid };
	if (file_info.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY)
		ret.type = File::Type::Directory;
	else {
		ret.type = File::Type::File;
		ret.size = static_cast<std::streamsize>(file_info.nFileSizeLow);
		assert(file_info.nFileSizeHigh == 0);
		if (file_info.nFileSizeHigh > 0) { // not sure if a text file should be this big?:P
			ret.size += static_cast<std::streamsize>(file_info.nFileSizeHigh) << 32;
		}
	}
	return ret;
}
File::FileAttributes File::ReadFileAttribute(const std::string& filename) {
	WIN32_FILE_ATTRIBUTE_DATA file_info;
	// Get file info
	if (GetFileAttributesExA(filename.c_str(), GetFileExInfoStandard, &file_info))
		return ReadFileAttributeTaslate(file_info);
	return { 0, File::Type::NotValid };
}
static bool ReadFileIntoVector(Symbol filename,const File::FileAttributes& attr, std::shared_ptr<File::vector_type>& ptr) {
	if (attr.type != File::Type::File || attr.size == 0) return false;
	auto it = data_cache.find(filename);
	if (it != data_cache.end()) {
		ptr = it->second.lock();	
	}
	else {
		std::lock_guard<std::mutex> lock(data_mutex);
		ptr.reset(new File::vector_type(attr.size), [filename](File::vector_type* ptr) {
			std::lock_guard<std::mutex> lock(data_mutex);
			data_cache.erase(filename);
		});
		std::ifstream f(filename);
		f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		assert(f.is_open());
		if (f.is_open()) {
			f.read((char*)ptr->data(), ptr->size());
		}
		f.close();
		data_cache[filename] = ptr;
	}
	return true;
}
 std::shared_ptr<File::vector_type> File::ReadFile(Symbol filename) {
	 std::shared_ptr<File::vector_type> ptr;
	 ReadFileIntoVector(filename, ReadFileAttribute(filename),ptr);
	 return ptr;
}

void File::FixPath(std::string& filename) {
	for (auto it = filename.begin(); it != filename.end(); it++) {
		if (isPathSep(*it) && path_sep != *it) *it = path_sep;
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
 File::File(Symbol path) : _attributes(ReadFileAttribute(path)), _path(path)  { }
 
 std::string_view File::getExtension() const {
	 if (isFile()) {
		 const char* end = _path.c_str() + _path.size();
		 for (auto it = end -1; it >= _path.c_str(); it--) {
			 if (isPathSep(*it)) break;
			 else if (*it == '.') return std::string_view(it, std::distance(it, end));
		 }
	 }
	 return std::string_view(); // returns an empty string, no extension
 }
 void File::open() {
	 ReadFileIntoVector(_path, _attributes, _data);
 }
 Symbol File::getPath() const {
	 return Symbol();
 }
 std::vector<File> File::getPathComponents() const {
	 return  std::vector<File>();
 }
 std::string_view File::getFileName() const {
	 if (isFile()) {
		 const char* end = _path.c_str() + _path.size();
		 for (auto it = end - 1; it >= _path.c_str(); it--) {
			 if (isPathSep(*it)) return std::string_view(it, std::distance(it, end));
		 }
	 }
	 return std::string_view(); // returns an empty string, no extension
 }
 File File::getUpDirectory() const {
	 return File();

 }
 //https://msdn.microsoft.com/en-us/library/windows/desktop/aa365200(v=vs.85).aspx
							  // return file list of current directory or if this is a directory, the list in that
 std::vector<File> File::filelist() const {

	 TCHAR szDir[MAX_PATH];
	 std::vector<File> ret;


	 if (_path.size() > MAX_PATH - 3) throw std::exception("Directory path is too long.");
	 std::copy(_path.begin(), _path.end(), szDir);
	 if (isPathSep(_path.back())) { 
		 szDir[_path.size()] = path_sep; 
		 szDir[_path.size() + 1] = 0; 
	 } else {
		 szDir[_path.size()] = 0;
	 }
	 // Prepare string for use with FindFile functions.  First, copy the
	 // string to a buffer, then append '\*' to the directory name.
	 WIN32_FIND_DATA ffd;
	 LARGE_INTEGER filesize;
	 size_t length_of_arg;
	 DWORD dwError = 0;
	 HANDLE hFind = FindFirstFile(szDir, &ffd);

	 if (INVALID_HANDLE_VALUE == hFind)
	 {
		 throw std::exception("FindFirstFile");
	 }

	 // List all the files in the directory with some info about them.

	 do
	 {
		 File file;
		 file._attributes = ReadFileAttributeTaslate(ffd);
		 file._path = Symbol::Lookup(ffd.cFileName);

	 } while (FindNextFile(hFind, &ffd) != 0);

	 dwError = GetLastError();
	 if (dwError != ERROR_NO_MORE_FILES)
	 {
		 throw std::exception(TEXT("FindFirstFile"));
	 }
	 return ret;
 }
