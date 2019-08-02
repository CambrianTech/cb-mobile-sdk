#ifndef Directory_h
#define Directory_h

#include <sys/types.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#endif

#include <iostream>
#include <vector>
#include <cambrian.h>

/**
 * Portable directory handling for Windows and Unix-like runtimes.
 */
class Directory
{
public:
	/**
	 * Constructor.
	 */
	Directory();

	/**
	 * Destructor.
	 */
	~Directory();

	/**
	 * Opens a directory for subsequent calls to next().
	 * @return true if the directory exists and next() can be called
	 */
	bool open(const char* dir_name);
    
    static std::string get_file_contents(const char *filename);

    static std::string parent_dir(const std::string &file_path);
    static std::string get_filename(const std::string &file_path);
    static std::string get_file_extension(const std::string& file_path);
    
    static void ensure_parent_dir_exists(const std::string &file_path);
    
    static void copy_file(const char *src_path, const char *dest_path, bool mkDir=false);
    
    static int mkpath(const char *path, mode_t mode = 0777);
	/**
	 * Creates a directory with open permissions (only matters on non-Windows).
	 * @param dir_name the directory path to create
	 * @return true if the operation was successful
	 */
	static bool mkdir(const char* dir_name);
    static bool remove_directory(const char* dir_name, bool removeFiles);
    static bool rename_directory(const char* oldName, const char* newName);
    static std::vector<std::string> list(const char* path, bool includeDirectories, bool includeFiles);
    
    static bool exists(const char* dir_name);
    
    static bool file_exists(const char *file_name);
    
    static size_t file_size (const char* file_name);
    
    static bool zipDirectoryContents(std::string directoryPath, std::string zipFilePath);
    static bool unzipToPath(std::string zipFilePath, std::string pathOrDirectory);
    
    static bool bzunzipFile(std::string zipFilePath, std::string outFilePath);
    static bool lzmaUnzipFile(std::string zipFilePath, std::string outFilePath);
    
    static bool unzipFileIfNewer(const std::string &compressedPath, const std::string &destPath);
    
    static time_t creationTime(const std::string &filename);
    static time_t lastWriteTime(const std::string &filename);
    static time_t lastAccessTime(const std::string &filename);

	/**
	 * Closes a directory after open() and one or more next() calls.
	 */
	void close();

	/**
	 * Called after open() to get the next file in the directory list.
	 */
	const char* next();
    

private:
#ifdef _WIN32
	/** Structure with windows file information. */
	WIN32_FIND_DATA m_fileData;

	/** Windows handle for walking directory. */
	HANDLE m_handle;

	/** Flag for directory open in Windows. */
	bool m_open;

	/** Flag for first file information is available. */
	bool m_onFirstFile;
#else
	/** Structure for directory walking in Unix-like runtimes. */
	DIR* dir;

	/** File information structure for Unix-like runtimes. */
	dirent entry;
#endif
};

#endif
