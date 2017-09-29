
#include "common.h"
#include <sys\stat.h>
#include <assert.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <mutex>


#ifdef _WIN32
#include <Windows.h>
#endif
struct MatchPathSeparator
{
	bool operator()(char ch) const
	{
		return ch == '\\' || ch == '/';
	}
};

std::string file_basename(std::string const& pathname)
{
	return std::string(
		std::find_if(pathname.rbegin(), pathname.rend(),
MatchPathSeparator()).base(),
pathname.end());
}
std::string file_removebasename(std::string const& pathname)
{
	return std::string(
		pathname.begin(), std::find_if(pathname.rbegin(), pathname.rend(),
			MatchPathSeparator()).base() + 1);
}
std::string file_removeExtension(std::string const& filename)
{
	std::string::const_reverse_iterator
		pivot
		= std::find(filename.rbegin(), filename.rend(), '.');
	return pivot == filename.rend()
		? filename
		: std::string(filename.begin(), pivot.base() - 1);
}
struct DirectoryInfo {
	bool current_directory;
	std::string_view filename;
	std::string_view device;
	std::vector<std::string_view> path;
};
DirectoryInfo splitpath(const std::string& str)
{
	MatchPathSeparator match;
	DirectoryInfo result;
	size_t offset = 0;
	if (str.length() > 2 && str[1] == ':') {
		result.device = std::string_view(str.data(), 2);
		offset += 2;
	}
	if (match(str[offset])) {
		result.current_directory = false;
		offset++;
	}
	else result.current_directory = true;
	size_t i;
	for (i = offset; i < str.length(); i++) {
		if (match(str[i])) {
			result.path.emplace_back(std::string_view(str.data() + offset, i - offset));
			offset = i;
		}
	}
	if (i < offset) {
		result.filename = std::string_view(str.data() + offset, i - offset);
	}
	return result;
}
std::string WindowsGetFullPathName(const std::string& filename) {
	constexpr size_t BUFSIZE = 4096;
	DWORD  retval = 0;
	BOOL   success;
	TCHAR  buffer[BUFSIZE] = TEXT("");
	TCHAR  buf[BUFSIZE] = TEXT("");
	TCHAR** lppPart = { NULL };
	// Retrieve the full path name for a file. 
	// The file does not need to exist.

	retval = GetFullPathName(filename.c_str(), BUFSIZE, buffer, lppPart);
	assert(retval != 0);
	return std::string(buffer, retval);
}
// NtCreateFile
static void throw_last_error() {
	LPWSTR pMessage = L"%1!*.*s! %4 %5!*s!";
	DWORD_PTR pArgs[] = { (DWORD_PTR)4, (DWORD_PTR)2, (DWORD_PTR)L"Bill",  // %1!*.*s! refers back to the first insertion string in pMessage
		(DWORD_PTR)L"Bob",                                                // %4 refers back to the second insertion string in pMessage
		(DWORD_PTR)6, (DWORD_PTR)L"Bill" };                               // %5!*s! refers back to the third insertion string in pMessage
	const DWORD size = 100 + 1;
	static CHAR buffer[size];
	if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		pMessage,
		0,
		0,
		buffer,
		size,
		(va_list*)pArgs))
	{
		wprintf(L"Format message failed with 0x%x\n", GetLastError());
		return;
	}
	throw std::exception(buffer);
}

void SystemFile::HandleDeleter::operator()(void* ptr) const {
	if(INVALID_HANDLE_VALUE != ptr || ptr != nullptr)
	CloseHandle((HANDLE*)ptr);
}
static void close_handle_deleter(void*ptr) { }
bool SystemFile::Open(const std::string& filename, SysFileMode mode) {
	std::lock_guard<std::mutex> lock(_mutex);
	_filename = WindowsGetFullPathName(filename);

	HANDLE file_handle = nullptr;
	DWORD wmode;
	WIN32_FILE_ATTRIBUTE_DATA attrib;
	if (GetFileAttributesEx(_filename.c_str(), GetFileExInfoStandard, &attrib) == 0) {
		throw_last_error();
		return false;
	}
	_file_size = attrib.nFileSizeLow + (attrib.nFileSizeHigh << 32);
	if (attrib.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		throw std::exception("file is directory");
		return false;
	}
	if (static_cast<uint32_t>(mode) & static_cast<uint32_t>(SysFileMode::Create)) wmode |= CREATE_NEW;
	_handle.open(_filename, std::ios::in | std::ios::out);
	return _handle.is_open();
}

bool SystemFile::OpenAt(SystemFile& from, const std::string& filename, SysFileMode mode) {
	if (from.isOpen()) return false;

	std::lock_guard<std::mutex> lock(from._mutex);
	std::string ffilename = file_removebasename(from._filename);
	assert(!MatchPathSeparator()(filename[0]));
	ffilename += filename;
	return Open(ffilename, mode);
}
void SystemFile::Close() {
	_handle.close();
}
size_t SystemFile::Read(void* data, size_t len)  {
	std::lock_guard<std::mutex> lock(_mutex);
	assert(isOpen());
	 _handle.read((char*)data, len);

	return len;
}

std::vector<uint8_t> SystemFile::ReadAll()  {
	std::lock_guard<std::mutex> lock(_mutex);
	std::vector<uint8_t> data(_file_size);
	assert(isOpen());
	 _handle.read((char*)data.data(), data.size());
	return std::move(data);
}

size_t SystemFile::Write(const void* data, size_t len) {
	std::lock_guard<std::mutex> lock(_mutex);
	assert(isOpen());
	 _handle.write((const char*)data, len);

	return len;
}



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
File::FileAttributes File::ReadFileAttribute(std::string& filename) {
	WIN32_FILE_ATTRIBUTE_DATA file_info;
	// Get file info
	MatchPathSeparator match;
	while (match(filename.back())) filename.resize(filename.size() - 1);
	if (GetFileAttributesExA(filename.data(), GetFileExInfoStandard, &file_info))
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
	 std::string ss = filename;
	 ReadFileIntoVector(filename, ReadFileAttribute(ss),ptr);
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
void File::GetDirectory(std::string& filename) {
	MatchPathSeparator match;
	while (match(filename.back()))
		filename.resize(filename.size() - 1);
		while (filename.size() > 0 && !match(filename.back()))
			filename.resize(filename.size() - 1);
 }
void File::UpDirectory(std::string& filename) {
	MatchPathSeparator match;
	while (match(filename.back()))  
		filename.resize(filename.size() - 1);
	while (filename.size() > 0 && !match(filename.back())) 
		filename.resize(filename.size() - 1);
 }

 File::File(Symbol path) : _attributes(ReadFileAttribute(std::string(path))), _path(path)  { }
 
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
		 file._path = Symbol(ffd.cFileName);

	 } while (FindNextFile(hFind, &ffd) != 0);

	 dwError = GetLastError();
	 if (dwError != ERROR_NO_MORE_FILES)
	 {
		 throw std::exception(TEXT("FindFirstFile"));
	 }
	 return ret;
 }
