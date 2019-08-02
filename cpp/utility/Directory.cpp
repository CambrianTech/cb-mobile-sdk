#include "Directory.h"

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/errno.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#define stat _stat
#endif

#include <memory.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <vector>

#include <iostream>
#include <fstream>

#if __APPLE__
#   include <bzlib.h>
#   include <lzma.h>
#endif

#include "Diagnostics.h"
#include "CommonUtility.h"


Directory::Directory()
{
#ifdef _WIN32
	m_handle = INVALID_HANDLE_VALUE;
	m_open = false;
	m_onFirstFile = false;
#else
	dir = NULL;
#endif
}

Directory::~Directory()
{
	close();
}

bool Directory::zipDirectoryContents(std::string directoryPath, std::string zipFilePath)
{
    return true;
}

bool Directory::unzipToPath(std::string zipFilePath, std::string toPath)
{
    printf("Unzipping file %s to %s\n", zipFilePath.c_str(), toPath.c_str());
    
    if (Directory::exists(toPath.c_str())) {
        Directory::remove_directory(toPath.c_str(), true);
    }
    Directory::mkpath(toPath.c_str());
    
    bool success = false;
    
#if 0
    
    bool destIsFile = toPath.size();
    
    std::string folderPrefix = get_filename(zipFilePath);
    folderPrefix = folderPrefix.substr(0, folderPrefix.length() - 4) + "/";
    
    unzFile _unzFile = unzOpen(zipFilePath.c_str());
    if( _unzFile )
    {
        unz_global_info  globalInfo = {0};
        if( unzGetGlobalInfo(_unzFile, &globalInfo )==UNZ_OK )
        {
            printf("%lu entries in the zip file", globalInfo.number_entry);
        }
    }
    else {
        return false;
    }
    
    int ret = unzGoToFirstFile( _unzFile );
    unsigned char		buffer[4096] = {0};

    do{
        ret = unzOpenCurrentFile( _unzFile );
        
        if( ret!=UNZ_OK )
        {
            success = false;
            break;
        }
        // reading data and write to file
        int read ;
        unz_file_info	fileInfo ={0};
        ret = unzGetCurrentFileInfo(_unzFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
        if( ret!=UNZ_OK )
        {
            success = false;
            unzCloseCurrentFile( _unzFile );
            break;
        }
        char* filename = (char*) malloc( fileInfo.size_filename +1 );
        unzGetCurrentFileInfo(_unzFile, &fileInfo, filename, fileInfo.size_filename + 1, NULL, 0, NULL, 0);
        filename[fileInfo.size_filename] = '\0';
        
        // check if it contains directory
        bool isDirectory = false;
        if( filename[fileInfo.size_filename-1]=='/' || filename[fileInfo.size_filename-1]=='\\')
            isDirectory = true;
        
        std::string filteredFileName = filename;
        
        free( filename );
        
        //filter out zips that have the zip file name as a directory inside
        if (filteredFileName.find(folderPrefix) == 0) {
            filteredFileName = filteredFileName.substr(folderPrefix.length());
        }
        std::string fullPath = toPath + "/" + filteredFileName;
        
        if (isDirectory) {
            mkpath(fullPath.c_str());
        }
        else if (destIsFile) {
            fullPath = toPath;
        } else {
            mkpath(parent_dir(fullPath.c_str()).c_str());
        }
        
        FILE* fp = 0;
        fp = fopen(fullPath.c_str(), "wb");
                
        while( fp )
        {
            read=unzReadCurrentFile(_unzFile, buffer, 4096);
            if( read > 0 )
            {
                fwrite(buffer, read, 1, fp );
            }
            else if( read<0 )
            {
                success = false;
                break;
            }
            else
                break;
        }
        
        if( fp )
        {
            fclose( fp );
        }
        unzCloseCurrentFile( _unzFile );
        ret = unzGoToNextFile( _unzFile );
    } while( ret==UNZ_OK && UNZ_OK!=UNZ_END_OF_LIST_OF_FILE );
    
#endif
    
    return success;
    
}

    std::string Directory::parent_dir(const std::string &file_path)
    {
        size_t pos = file_path.find_last_of("\\/");
        return (std::string::npos == pos) ? "" : file_path.substr(0, pos);
    }
    
    std::string Directory::get_filename(const std::string &file_path)
    {
        size_t pos = file_path.find_last_of("\\/");
        return (std::string::npos == pos) ? file_path : file_path.substr(pos+1);
    }

    std::string Directory::get_file_extension(const std::string& file_path)
    {
        if(file_path.find_last_of(".") != std::string::npos)
            return file_path.substr(file_path.find_last_of(".")+1);
        return "";
    }
    
    void Directory::ensure_parent_dir_exists(const std::string &file_path)
    {
        //make sure the path exists
        bool containsDir = file_path.find("/") != std::string::npos;
        if (containsDir) {
            //ensure directory exists
            Directory::mkpath(Directory::parent_dir(file_path).c_str());
        }
    }
    
    void Directory::copy_file(const char *src_path, const char *dest_path, bool mkDir)
    {
        if (mkDir) {
            ensure_parent_dir_exists(dest_path);
        }

        std::ifstream source(src_path, std::ios::binary);
        std::ofstream dest(dest_path, std::ios::binary);
            
        std::istreambuf_iterator<char> begin_source(source);
        std::istreambuf_iterator<char> end_source;
        std::ostreambuf_iterator<char> begin_dest(dest);
        std::copy(begin_source, end_source, begin_dest);
        
        source.close();
        dest.close();
    }

    typedef struct stat Stat;
    
    static int do_mkdir(const char *path, mode_t mode)
    {
        Stat            st;
        int             status = 0;
        
        if (stat(path, &st) != 0)
        {
            /* Directory does not exist. EEXIST for race condition */
            if (mkdir(path, mode) != 0 && errno != EEXIST) {
                //printf("Couldn't make directory %s\n", path);
                status = -1;
            }
        }
        else if (!S_ISDIR(st.st_mode))
        {
            errno = ENOTDIR;
            status = -1;
        }
        
        return(status);
    }

    time_t Directory::creationTime(const std::string &filename) {
        struct stat result;
        if(stat(filename.c_str(), &result)==0)
        {
#if __APPLE__
            auto mod_time = result.st_ctimespec.tv_sec;
#else
            auto mod_time = result.st_ctime;
#endif
            return mod_time;
        }
        return 0;
    }

    time_t Directory::lastWriteTime(const std::string &filename) {
        struct stat result;
        if(stat(filename.c_str(), &result)==0)
        {
#if __APPLE__
            auto mod_time = result.st_mtimespec.tv_sec;
#else
            auto mod_time = result.st_mtime;
#endif
            return mod_time;
        }
        return 0;
    }

    time_t Directory::lastAccessTime(const std::string &filename) {
        struct stat result;
        if(stat(filename.c_str(), &result)==0)
        {
#if __APPLE__
            auto mod_time = result.st_atimespec.tv_sec;
#else
            auto mod_time = result.st_atime;
#endif
            
            return mod_time;
        }
        return 0;
    }

    /**
     ** mkpath - ensure all directories in path exist
     ** Algorithm takes the pessimistic view and works top-down to ensure
     ** each directory in path exists, rather than optimistically creating
     ** the last element and working backwards.
     */
    int Directory::mkpath(const char *path, mode_t mode)
    {
        char           *pp;
        char           *sp;
        int             status;
        char           *copypath = strdup(path);
        
        status = 0;
        pp = copypath;
        while (status == 0 && (sp = strchr(pp, '/')) != 0)
        {
            if (sp != pp)
            {
                /* Neither root nor double slash in path */
                *sp = '\0';
                status = do_mkdir(copypath, mode);
                *sp = '/';
            }
            pp = sp + 1;
        }
        if (status == 0)
            status = do_mkdir(path, mode);
        free(copypath);
        return (status);
    }


bool Directory::mkdir(const char* dir_name)
{
#ifdef _WIN32
	return CreateDirectory(dir_name, NULL) != 0;
#else
	return ::mkdir(dir_name, 0777) == 0;
#endif
}

static int path_is_directory (const char* path) {
    struct stat s_buf;
    
    if (stat(path, &s_buf))
        return 0;
    
    return S_ISDIR(s_buf.st_mode);
}

//bool Directory::remove_file(const char* path)
//{
//    remove(path);
//}

bool Directory::rename_directory(const char* oldName, const char* newName)
{
    return 0 == rename(oldName, newName);
}

bool Directory::remove_directory(const char* path, bool removeFiles)
{
    DIR*            dp;
    struct dirent*  ep;
    char            p_buf[512] = {0};
    
    if (removeFiles) {
        dp = opendir(path);
        
        while ((ep = readdir(dp)) != NULL) {
            if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
                continue;
            sprintf(p_buf, "%s/%s", path, ep->d_name);
            
            if (path_is_directory(p_buf))
                remove_directory(p_buf, true);
            else
                unlink(p_buf);
        }
        
        closedir(dp);
    }
    rmdir(path);
    
    return true;
}

std::vector<std::string> Directory::list(const char* path, bool includeDirectories, bool includeFiles)
{
    std::vector<std::string> files;
    
    if (!exists(path)) return files;
    
    DIR*            dp;
    struct dirent*  ep;
    char            p_buf[512] = {0};
    
    dp = opendir(path);
    
    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        sprintf(p_buf, "%s/%s", path, ep->d_name);
        
        bool isDir = path_is_directory(p_buf);
        
        if ((isDir && includeDirectories)
            || (!isDir && includeFiles)) {
            files.push_back(ep->d_name);
        }
    }
    
    closedir(dp);
    
    return files;
}

bool Directory::exists(const char* dir_name)
{
    Stat            st;
    
    if (stat(dir_name, &st) != 0)
    {
        return false;
    }
    
    return S_ISDIR(st.st_mode);
}

bool Directory::file_exists (const char* file_name) {
    Stat            st;
    
    if (stat(file_name, &st) != 0)
    {
        return false;
    }
    
    return !S_ISDIR(st.st_mode);
}

size_t Directory::file_size (const char* file_name) {
    
    std::streampos begin,end;
    std::ifstream myfile (file_name, std::ios::binary);
    begin = myfile.tellg();
    myfile.seekg (0, std::ios::end);
    end = myfile.tellg();
    myfile.close();
    
    size_t fileSize = size_t(end-begin);
    
    return fileSize;
}

bool Directory::open(const char* dir_name)
{
	close();
#ifdef _WIN32
	m_handle = FindFirstFile(dir_name, &m_fileData);

	if (m_handle != INVALID_HANDLE_VALUE)
	{
		m_open = true;
		m_onFirstFile = true;
	}
	else
	{
		m_open = false;
	}

	return m_open;
#else
	dir = opendir(dir_name);

	return dir != NULL;
#endif
}

void Directory::close()
{
#ifdef _WIN32
	m_open = false;
#else
	if (dir)
	{
		closedir(dir);
		dir = NULL;
	}
#endif
}

std::string Directory::get_file_contents(const char *filename)
{
    std::ifstream ifs(filename);
    std::string content( (std::istreambuf_iterator<char>(ifs) ),
                        (std::istreambuf_iterator<char>()    ) );
    
    return content;
}

const char*
Directory::next()
{
#ifdef _WIN32
	if (m_open)
	{
		// If we already have the first file from calling open() just use it
		if (m_onFirstFile)
		{
			m_onFirstFile = false;
			return m_fileData.cFileName;
		}
		else
		{
			if (FindNextFile(m_handle, &m_fileData))
			{
				return m_fileData.cFileName;
			}
			else
			{
				close();
				return NULL;
			}
		}
	}
	else
	{
		return NULL;
	}
#else
	if (!dir)
	{
		return NULL;
	}
	dirent* res = readdir(dir);
	if (res)
	{
		memcpy(&entry, res, sizeof(dirent));
		return entry.d_name;
	}

	return NULL;
#endif

}

bool Directory::bzunzipFile(std::string zipFilePath, std::string outFilePath) {
    
    bool success = false;
    
#ifdef _BZLIB_H
    int bzError;
    BZFILE *bzf = nullptr;
    char buf[4096];
    
    success = true;
    
    FILE *f = 0;
    f = fopen(zipFilePath.c_str(), "r");
    
    FILE * fOut = 0;
    fOut = fopen (outFilePath.c_str(), "wb");
    
    if (!fOut) return false;
    
    bzf = BZ2_bzReadOpen(&bzError, f, 0, 0, NULL, 0);
    if (success && bzError != BZ_OK) {
        fprintf(stderr, "bzunzipFile Error: BZ2_bzReadOpen: %d\n", bzError);
        success = false;
    }
    
    while (success && bzError == BZ_OK) {
        int nread = BZ2_bzRead(&bzError, bzf, buf, sizeof buf);
        if (bzError == BZ_OK || bzError == BZ_STREAM_END) {
            size_t nwritten = fwrite(buf, 1, nread, fOut);
            if (nwritten != (size_t) nread) {
                fprintf(stderr, "bzunzipFile Error: short write\n");
                success = false;
            }
        }
    }
    
    
    if (success && bzError != BZ_STREAM_END) {
        fprintf(stderr, "bzunzipFile Error: bzip error after read: %d\n", bzError);
        success = false;
    }
    
    if (fOut) {
        fclose(fOut);
    }
    
    if (bzf) {
        BZ2_bzReadClose(&bzError, bzf);
    }
    
    if (f) {
        fclose(f);
    }
#endif
    
    return success;
}

bool Directory::unzipFileIfNewer(const std::string &compressedPath, const std::string &destPath) {
    
    std::string destStatPath = string_sprintf("%s.stat", destPath.c_str());
    
    Directory::ensure_parent_dir_exists(destPath);
    
    auto srcSize = Directory::file_size(compressedPath.c_str());
    
    bool statExists = Directory::file_exists(destStatPath.c_str());
    bool destExists = Directory::file_exists(destPath.c_str());
    
    bool unzip = !destExists || !statExists;
    
    if (!unzip) {
        //make sure it isn't corrupt
        if (Directory::file_exists(destStatPath.c_str())) {
            
            auto prevSrcSize = file_size(destStatPath.c_str());
            if (srcSize != prevSrcSize) {
                unzip = true;
            }
        } else {
            unzip = true;
        }
    }
    
    if (unzip) {
        if (destExists) std::remove(destPath.c_str());
        if (statExists) std::remove(destStatPath.c_str());
        
        auto start = sys_usec_time();
        
        std::string destTempPath = string_sprintf("%s.tmp", destPath.c_str());
        
        CBLog("Decompressing file at %s", compressedPath.c_str());
        
        auto extension = get_file_extension(compressedPath);
        
        bool success = false;
        
        if (extension == "zip") {
            success = Directory::unzipToPath(compressedPath, destTempPath);
        } else if (extension == "bz2") {
            success = Directory::bzunzipFile(compressedPath, destTempPath);
        } else if (extension == "lzma" || extension == "xz") {
            success = Directory::lzmaUnzipFile(compressedPath, destTempPath);
        }
        
        if (success) {
            CBLog("Decompressing took %.2f seconds.", seconds_elapsed(start));
            std::ofstream infoFile;
            infoFile.open(destStatPath);
            
            infoFile << srcSize;//store for later
            infoFile.close();
            
            std::rename(destTempPath.c_str(), destPath.c_str());
        } else {
            CBLog("Decompressing from %s to %s failed.", compressedPath.c_str(), destPath.c_str());
            return false;
        }
    }
    
    
    return unzip;
}

#ifdef LZMA_H
static bool
init_decoder(lzma_stream *strm)
{
    // Initialize a .xz decoder. The decoder supports a memory usage limit
    // and a set of flags.
    //
    // The memory usage of the decompressor depends on the settings used
    // to compress a .xz file. It can vary from less than a megabyte to
    // a few gigabytes, but in practice (at least for now) it rarely
    // exceeds 65 MiB because that's how much memory is required to
    // decompress files created with "xz -9". Settings requiring more
    // memory take extra effort to use and don't (at least for now)
    // provide significantly better compression in most cases.
    //
    // Memory usage limit is useful if it is important that the
    // decompressor won't consume gigabytes of memory. The need
    // for limiting depends on the application. In this example,
    // no memory usage limiting is used. This is done by setting
    // the limit to UINT64_MAX.
    //
    // The .xz format allows concatenating compressed files as is:
    //
    //     echo foo | xz > foobar.xz
    //     echo bar | xz >> foobar.xz
    //
    // When decompressing normal standalone .xz files, LZMA_CONCATENATED
    // should always be used to support decompression of concatenated
    // .xz files. If LZMA_CONCATENATED isn't used, the decoder will stop
    // after the first .xz stream. This can be useful when .xz data has
    // been embedded inside another file format.
    //
    // Flags other than LZMA_CONCATENATED are supported too, and can
    // be combined with bitwise-or. See lzma/container.h
    // (src/liblzma/api/lzma/container.h in the source package or e.g.
    // /usr/include/lzma/container.h depending on the install prefix)
    // for details.
    lzma_ret ret = lzma_stream_decoder(
                                       strm, UINT64_MAX, LZMA_CONCATENATED);
    
    // Return successfully if the initialization went fine.
    if (ret == LZMA_OK)
        return true;
    
    // Something went wrong. The possible errors are documented in
    // lzma/container.h (src/liblzma/api/lzma/container.h in the source
    // package or e.g. /usr/include/lzma/container.h depending on the
    // install prefix).
    //
    // Note that LZMA_MEMLIMIT_ERROR is never possible here. If you
    // specify a very tiny limit, the error will be delayed until
    // the first headers have been parsed by a call to lzma_code().
    const char *msg;
    switch (ret) {
        case LZMA_MEM_ERROR:
            msg = "Memory allocation failed";
            break;
            
        case LZMA_OPTIONS_ERROR:
            msg = "Unsupported decompressor flags";
            break;
            
        default:
            // This is most likely LZMA_PROG_ERROR indicating a bug in
            // this program or in liblzma. It is inconvenient to have a
            // separate error message for errors that should be impossible
            // to occur, but knowing the error code is important for
            // debugging. That's why it is good to print the error code
            // at least when there is no good error message to show.
            msg = "Unknown error, possibly a bug";
            break;
    }
    
    fprintf(stderr, "Error initializing the decoder: %s (error code %u)\n",
            msg, ret);
    return false;
}

static bool
decompress(lzma_stream *strm, FILE *infile, FILE *outfile)
{
    // When LZMA_CONCATENATED flag was used when initializing the decoder,
    // we need to tell lzma_code() when there will be no more input.
    // This is done by setting action to LZMA_FINISH instead of LZMA_RUN
    // in the same way as it is done when encoding.
    //
    // When LZMA_CONCATENATED isn't used, there is no need to use
    // LZMA_FINISH to tell when all the input has been read, but it
    // is still OK to use it if you want. When LZMA_CONCATENATED isn't
    // used, the decoder will stop after the first .xz stream. In that
    // case some unused data may be left in strm->next_in.
    lzma_action action = LZMA_RUN;
    
    uint8_t inbuf[BUFSIZ];
    uint8_t outbuf[BUFSIZ];
    
    strm->next_in = NULL;
    strm->avail_in = 0;
    strm->next_out = outbuf;
    strm->avail_out = sizeof(outbuf);
    
    while (true) {
        if (strm->avail_in == 0 && !feof(infile)) {
            strm->next_in = inbuf;
            strm->avail_in = fread(inbuf, 1, sizeof(inbuf),
                                   infile);
            
            if (ferror(infile)) {
                fprintf(stderr, "Read error: %s\n", strerror(errno));
                return false;
            }
            
            // Once the end of the input file has been reached,
            // we need to tell lzma_code() that no more input
            // will be coming. As said before, this isn't required
            // if the LZMA_CONATENATED flag isn't used when
            // initializing the decoder.
            if (feof(infile))
                action = LZMA_FINISH;
        }
        
        lzma_ret ret = lzma_code(strm, action);
        
        if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
            size_t write_size = sizeof(outbuf) - strm->avail_out;
            
            if (fwrite(outbuf, 1, write_size, outfile)
                != write_size) {
                fprintf(stderr, "Write error: %s\n",
                        strerror(errno));
                return false;
            }
            
            strm->next_out = outbuf;
            strm->avail_out = sizeof(outbuf);
        }
        
        if (ret != LZMA_OK) {
            // Once everything has been decoded successfully, the
            // return value of lzma_code() will be LZMA_STREAM_END.
            //
            // It is important to check for LZMA_STREAM_END. Do not
            // assume that getting ret != LZMA_OK would mean that
            // everything has gone well or that when you aren't
            // getting more output it must have successfully
            // decoded everything.
            if (ret == LZMA_STREAM_END)
                return true;
            
            // It's not LZMA_OK nor LZMA_STREAM_END,
            // so it must be an error code. See lzma/base.h
            // (src/liblzma/api/lzma/base.h in the source package
            // or e.g. /usr/include/lzma/base.h depending on the
            // install prefix) for the list and documentation of
            // possible values. Many values listen in lzma_ret
            // enumeration aren't possible in this example, but
            // can be made possible by enabling memory usage limit
            // or adding flags to the decoder initialization.
            const char *msg;
            switch (ret) {
                case LZMA_MEM_ERROR:
                    msg = "Memory allocation failed";
                    break;
                    
                case LZMA_FORMAT_ERROR:
                    // .xz magic bytes weren't found.
                    msg = "The input is not in the .xz format";
                    break;
                    
                case LZMA_OPTIONS_ERROR:
                    // For example, the headers specify a filter
                    // that isn't supported by this liblzma
                    // version (or it hasn't been enabled when
                    // building liblzma, but no-one sane does
                    // that unless building liblzma for an
                    // embedded system). Upgrading to a newer
                    // liblzma might help.
                    //
                    // Note that it is unlikely that the file has
                    // accidentally became corrupt if you get this
                    // error. The integrity of the .xz headers is
                    // always verified with a CRC32, so
                    // unintentionally corrupt files can be
                    // distinguished from unsupported files.
                    msg = "Unsupported compression options";
                    break;
                    
                case LZMA_DATA_ERROR:
                    msg = "Compressed file is corrupt";
                    break;
                    
                case LZMA_BUF_ERROR:
                    // Typically this error means that a valid
                    // file has got truncated, but it might also
                    // be a damaged part in the file that makes
                    // the decoder think the file is truncated.
                    // If you prefer, you can use the same error
                    // message for this as for LZMA_DATA_ERROR.
                    msg = "Compressed file is truncated or "
                    "otherwise corrupt";
                    break;
                    
                default:
                    // This is most likely LZMA_PROG_ERROR.
                    msg = "Unknown error, possibly a bug";
                    break;
            }
            
            fprintf(stderr, "Decoder error: "
                    "%s (error code %u)\n", msg, ret);
            return false;
        }
    }
}
#endif

bool Directory::lzmaUnzipFile(std::string zipFilePath, std::string outFilePath) {
    
    bool success = true;
    
#ifdef LZMA_H
    lzma_stream strm = LZMA_STREAM_INIT;
    
    // Try to decompress all files.
    
    if (!init_decoder(&strm)) {
        // Decoder initialization failed. There's no point
        // to retry it so we need to exit.
        success = false;
        return false;
    }
    
    FILE *infile = fopen(zipFilePath.c_str(), "rb");
    FILE *outfile = nullptr;
    
    if (infile == NULL) {
        CBError("%s: Error opening the input file: %s\n",
                zipFilePath.c_str(), strerror(errno));
        success = false;
    } else {
        outfile = fopen(outFilePath.c_str(), "wb");
        success &= decompress(&strm, infile, outfile);
        fclose(infile);
    }
    
    // Free the memory allocated for the decoder. This only needs to be
    // done after the last file.
    lzma_end(&strm);
    
    if (outfile && fclose(outfile)) {
        fprintf(stderr, "Write error: %s\n", strerror(errno));
        success = false;
    }
#endif
    
    return success;
}

