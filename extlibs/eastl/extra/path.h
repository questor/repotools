// A simple class for manipulating paths on Linux/Windows/Mac OS

// Copyright 2015 Wenzel Jakob. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#if !defined(WIN32)
    path path1("/dir 1/dir 2/");
#else
    path path1("C:\\dir 1\\dir 2\\");
#endif
    path path2("dir 3");

    cout << path1.exists() << endl;
    cout << path1 << endl;
    cout << (path1/path2) << endl;
    cout << (path1/path2).parent_path() << endl;
    cout << (path1/path2).parent_path().parent_path() << endl;
    cout << (path1/path2).parent_path().parent_path().parent_path() << endl;
    cout << (path1/path2).parent_path().parent_path().parent_path().parent_path() << endl;
    cout << path().parent_path() << endl;

    cout << "nonexistant:exists = " << path("nonexistant").exists() << endl;
    cout << "nonexistant:is_file = " << path("nonexistant").is_file() << endl;
    cout << "nonexistant:is_directory = " << path("nonexistant").is_directory() << endl;
    cout << "nonexistant:extension = " << path("nonexistant").extension() << endl;
    cout << "filesystem/path.h:exists = " << path("filesystem/path.h").exists() << endl;
    cout << "filesystem/path.h:is_file = " << path("filesystem/path.h").is_file() << endl;
    cout << "filesystem/path.h:is_directory = " << path("filesystem/path.h").is_directory() << endl;
    cout << "filesystem/path.h:extension = " << path("filesystem/path.h").extension() << endl;
    cout << "filesystem/path.h:make_absolute = " << path("filesystem/path.h").make_absolute() << endl;
    cout << "../filesystem:exists = " << path("../filesystem").exists() << endl;
    cout << "../filesystem:is_file = " << path("../filesystem").is_file() << endl;
    cout << "../filesystem:is_directory = " << path("../filesystem").is_directory() << endl;
    cout << "../filesystem:extension = " << path("../filesystem").extension() << endl;
    cout << "../filesystem:make_absolute = " << path("../filesystem").make_absolute() << endl;

    cout << "resolve(filesystem/path.h) = " << resolver().resolve("filesystem/path.h") << endl;
    cout << "resolve(nonexistant) = " << resolver().resolve("nonexistant") << endl;
*/

#if !defined(__FILESYSTEM_PATH_H)
#define __FILESYSTEM_PATH_H

#include "eastl/string.h"
#include "eastl/vector.h"
#include <stdexcept>
#include <memory>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#if defined(_MSC_VER)
# include <windows.h>
# include <ShlObj.h>
#else
# include <sys/stat.h>
# include <unistd.h>
# include <dirent.h>
# include <sys/types.h>
#endif

#if defined(__linux)
# include <linux/limits.h>
#endif

namespace filesystem {

/**
 * \brief Simple class for manipulating paths on Linux/Windows/Mac OS
 *
 * This class is just a temporary workaround to avoid the heavy boost
 * dependency until boost::filesystem is integrated into the standard template
 * library at some point in the future.
 */
class path {
public:
    enum path_type {
        windows_path = 0,
        posix_path = 1,
#if defined(WIN32)
        native_path = windows_path
#else
        native_path = posix_path
#endif
    };

    path() : m_type(native_path), m_absolute(false), m_smb(false) { }

    path(const path &path)
        : m_type(path.m_type), m_path(path.m_path), m_absolute(path.m_absolute), m_smb(path.m_smb) {}

    path(path &&path)
        : m_type(path.m_type), m_path(std::move(path.m_path)),
          m_absolute(path.m_absolute), m_smb(path.m_smb) {}
    
    path(const char *string) { set(string); }

    path(const eastl::string &string) { set(string); }

#if defined(WIN32)
    path(const std::wstring &wstring) { set(wstring); }
    path(const wchar_t *wstring) { set(wstring); }
#endif

    size_t size() const { return m_path.size(); }

    bool empty() const { return m_path.empty(); }

    bool is_absolute() const { return m_absolute; }

    path make_absolute() const {
#if !defined(WIN32)
        char temp[PATH_MAX];
        if (realpath(str().c_str(), temp) == NULL)
            throw std::runtime_error("Internal error in realpath(): " + eastl::string(strerror(errno)));
        return path(temp);
#else
        std::wstring value = wstr(), out(MAX_PATH_WINDOWS, '\0');
        DWORD length = GetFullPathNameW(value.c_str(), MAX_PATH_WINDOWS, &out[0], NULL);
        if (length == 0)
            throw std::runtime_error("Internal error in realpath(): " + std::to_string(GetLastError()));
        return path(out.substr(0, length));
#endif
    }

    bool exists() const {
#if defined(WIN32)
        return GetFileAttributesW(wstr().c_str()) != INVALID_FILE_ATTRIBUTES;
#else
        struct stat sb;   
        return stat(str().c_str(), &sb) == 0;
#endif
    }

    size_t file_size() const {
#if defined(_WIN32)
        struct _stati64 sb;
        if (_wstati64(wstr().c_str(), &sb) != 0) {
            return (size_t)-1;
//            throw std::runtime_error("path::file_size(): cannot stat file \"" + str() + "\"!");
        }
#else
        struct stat sb;
        if (stat(str().c_str(), &sb) != 0) {
            return (size_t)-1;
//            throw std::runtime_error("path::file_size(): cannot stat file \"" + str() + "\"!");
        }
#endif
        return (size_t) sb.st_size;
    }
    bool is_directory() const {
#if defined(WIN32)
        DWORD result = GetFileAttributesW(wstr().c_str());
        if (result == INVALID_FILE_ATTRIBUTES)
            return false;
        return (result & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
        struct stat sb;   
        if (stat(str().c_str(), &sb))
            return false;
        return S_ISDIR(sb.st_mode);
#endif
    }

    bool is_file() const {
#if defined(_WIN32)
        DWORD attr = GetFileAttributesW(wstr().c_str());
        return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
        struct stat sb;   
        if (stat(str().c_str(), &sb))
            return false;
        return S_ISREG(sb.st_mode);
#endif
    }

    eastl::string extension() const {
        const eastl::string &name = filename();
        size_t pos = name.findLastOf(".");
        if (pos == eastl::string::npos)
            return "";
        return name.substr(pos+1);
    }
    eastl::string filename() const {
        if (empty())
            return "";
        const eastl::string &last = m_path[m_path.size()-1];
        return last;
    }

    path parent_path() const {
        path result;
        result.m_absolute = m_absolute;
        result.m_smb = m_smb;

        if (m_path.empty()) {
            if (!m_absolute)
                result.m_path.pushBack("..");
        } else {
            size_t until = m_path.size() - 1;
            for (size_t i = 0; i < until; ++i)
                result.m_path.pushBack(m_path[i]);
        }
        return result;
    }

    path operator/(const path &other) const {
        if (other.m_absolute)
            throw std::runtime_error("path::operator/(): expected a relative path!");
        if (m_type != other.m_type)
            throw std::runtime_error("path::operator/(): expected a path of the same type!");

        path result(*this);

        for (size_t i=0; i<other.m_path.size(); ++i)
            result.m_path.pushBack(other.m_path[i]);

        return result;
    }

    eastl::string str(path_type type = native_path) const {
        std::ostringstream oss;

       if (m_absolute) {
            if (m_type == posix_path)
                oss << "/";
            else {
                size_t length = 0;
                for (size_t i = 0; i < m_path.size(); ++i)
                    // No special case for the last segment to count the NULL character
                    length += m_path[i].length() + 1;
                if (m_smb)
                    length += 2;

                // Windows requires a \\?\ prefix to handle paths longer than MAX_PATH
                // (including their null character). NOTE: relative paths >MAX_PATH are
                // not supported at all in Windows.
                if (length > MAX_PATH_WINDOWS_LEGACY) {
                    if (m_smb)
                        oss << "\\\\?\\UNC\\";
                    else
                        oss << "\\\\?\\";
                } else if (m_smb)
                    oss << "\\\\";
            }
        }

        for (size_t i=0; i<m_path.size(); ++i) {
            oss << m_path[i].c_str();
            if (i+1 < m_path.size()) {
                if (type == posix_path)
                    oss << '/';
                else
                    oss << '\\';
            }
        }
        return eastl::string(oss.str().c_str());
    }


    eastl::string string() const {
        return str();
    }

    void set(const eastl::string &str, path_type type = native_path) {
        m_type = type;
        if (type == windows_path) {
            eastl::string tmp = str;

            // Long windows paths (sometimes) begin with the prefix \\?\. It should only
            // be used when the path is >MAX_PATH characters long, so we remove it
            // for convenience and add it back (if necessary) in str()/wstr().
            static const eastl::string LONG_PATH_PREFIX = "\\\\?\\";
            if (tmp.length() >= LONG_PATH_PREFIX.length()
             && eastl::mismatch(eastl::begin(LONG_PATH_PREFIX), eastl::end(LONG_PATH_PREFIX), eastl::begin(tmp)).first == eastl::end(LONG_PATH_PREFIX)) {
                tmp.erase(0, LONG_PATH_PREFIX.length());
            }

            // Special-case handling of absolute SMB paths, which start with the prefix "\\".
            if (tmp.length() >= 2 && tmp[0] == '\\' && tmp[1] == '\\') {
                m_path = {};
                tmp.erase(0, 2);

                // Interestingly, there is a special-special case where relative paths may be specified as beginning with a "\\"
                // when a non-SMB file with a more-than-260-characters-long absolute _local_ path is double-clicked. This seems to
                // only happen with single-segment relative paths, so we can check for this condition by making sure no further
                // path separators are present.
                if (tmp.findFirstOf("/\\") != eastl::string::npos)
                    m_absolute = m_smb = true;
                else
                    m_absolute = m_smb = false;

            // Special-case handling of absolute SMB paths, which start with the prefix "UNC\"
            } else if (tmp.length() >= 4 && tmp[0] == 'U' && tmp[1] == 'N' && tmp[2] == 'C' && tmp[3] == '\\') {
                m_path = {};
                tmp.erase(0, 4);
                m_absolute = true;
                m_smb = true;
            // Special-case handling of absolute local paths, which start with the drive letter and a colon "X:\"
            } else if (tmp.length() >= 3 && std::isalpha(tmp[0]) && tmp[1] == ':' && (tmp[2] == '\\' || tmp[2] == '/')) {
                m_path = {tmp.substr(0, 2)};
                tmp.erase(0, 3);
                m_absolute = true;
                m_smb = false;
            // Relative path
            } else {
                m_path = {};
                m_absolute = false;
                m_smb = false;
            }

            eastl::vector<eastl::string> tokenized = tokenize(tmp, "/\\");
            m_path.insert(eastl::end(m_path), eastl::begin(tokenized), eastl::end(tokenized));
        } else {
            m_path = tokenize(str, "/");
            m_absolute = !str.empty() && str[0] == '/';
        }
    }
    
    path &operator=(const path &path) {
        m_type = path.m_type;
        m_path = path.m_path;
        m_absolute = path.m_absolute;
        m_smb = path.m_smb;
        return *this;
    }

    path &operator=(path &&path) {
        if (this != &path) {
            m_type = path.m_type;
            m_path = std::move(path.m_path);
            m_absolute = path.m_absolute;
            m_smb = path.m_smb;
        }
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const path &path) {
        os << path.str().c_str();
        return os;
    }

    static path getcwd() {
#if !defined(_WIN32)
        char temp[PATH_MAX];
        if (::getcwd(temp, PATH_MAX) == NULL) {
            return path("INTERNAL ERROR!");
//            throw eastl::runtime_error("Internal error in getcwd(): " + eastl::string(strerror(errno)));
        }
        return path(temp);
#else
        std::wstring temp(MAX_PATH_WINDOWS, '\0');
        if (!_wgetcwd(&temp[0], MAX_PATH_WINDOWS)) {
            return path("INTERNAL ERROR!");
//            throw eastl::runtime_error("Internal error in getcwd(): " + eastl::to_string(GetLastError()));
        }
        return path(temp.c_str());
#endif
    }
#if defined(WIN32)
    std::wstring wstr() const {
        eastl::string temp = str();
        int size = MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), NULL, 0);
        std::wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), &result[0], size);
        return result;
    }


    void set(const std::wstring &wstring, path_type type = native_path) {
        eastl::string string;
        if (!wstring.empty()) {
            int size = WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(),
                            NULL, 0, NULL, NULL);
            string.resize(size, 0);
            WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(),
                                &string[0], size, NULL, NULL);
        }
        set(string, type);
    }

    path &operator=(const std::wstring &str) { set(str); return *this; }
#endif

    bool operator==(const path &p) const { return p.m_path == m_path; }
    bool operator!=(const path &p) const { return p.m_path != m_path; }
    
protected:
    static eastl::vector<eastl::string> tokenize(const eastl::string &string, const eastl::string &delim) {
        eastl::string::size_type lastPos = 0, pos = string.findFirstOf(delim, lastPos);
        eastl::vector<eastl::string> tokens;

        while (lastPos != eastl::string::npos) {
            if (pos != lastPos)
                tokens.pushBack(string.substr(lastPos, pos - lastPos));
            lastPos = pos;
            if (lastPos == eastl::string::npos || lastPos + 1 == string.length())
                break;
            pos = string.findFirstOf(delim, ++lastPos);
        }

        return tokens;
    }

protected:
#if defined(_WIN32)
    static const size_t MAX_PATH_WINDOWS = 32767;
#endif
    static const size_t MAX_PATH_WINDOWS_LEGACY = 260;
    path_type m_type;
    eastl::vector<eastl::string> m_path;
    bool m_absolute;
    bool m_smb; // Unused, except for on Windows
};

class directory_entry {
public:
    directory_entry() = default;
    explicit directory_entry(const path& p) : p(p) {}
    filesystem::path path() const {
        return p;
    }

private:
    filesystem::path p;
};

class directory_iterator {
public:
    directory_iterator() = default;
    directory_iterator(const path& p) : p(p) {}
    directory_iterator(const directory_iterator& rhs) = default;
    directory_iterator(directory_iterator&& rhs) = default;
    ~directory_iterator() = default;

    directory_iterator begin() {
#if defined(_WIN32)
        hFind = FindFirstFile((p / path("*")).str().c_str(), &findData);
#else
        dir = std::shared_ptr<DIR>(opendir(p.str().c_str()), closedir);
#endif
        return increment();
    }

    directory_iterator end() const {
        return directory_iterator();
    }

    bool operator==(const directory_iterator& rhs) const {
        return entry.path() == rhs.entry.path();
    }

    bool operator!=(const directory_iterator& rhs) const {
        return entry.path() != rhs.entry.path();
    }

    directory_entry operator*() const {
        return entry;
    }

    directory_entry* operator->() {
        return &entry;
    }

    directory_iterator& increment() {
#if defined(_WIN32)
        if (hFind != NULL && hFind != INVALID_HANDLE_VALUE) {
            do {
                if (findData.cFileName[0] != '.') break;
            } while (FindNextFile(hFind, &findData));
        }

        if (hFind != NULL && hFind != INVALID_HANDLE_VALUE) {
            entry = directory_entry(p / path(findData.cFileName));
            if (!FindNextFile(hFind, &findData)) {
                FindClose(hFind);
                hFind = NULL;
            }
        } else {
            entry = directory_entry();
        }
#else
        dirent *ent = NULL;
        while ((ent = readdir(dir.get())) != NULL) {
            if (ent->d_name[0] != '.') break;
        }

        if (ent)
            entry = directory_entry(p / path(ent->d_name));
        else
            entry = directory_entry();
#endif
        return *this;
    }

    directory_iterator& operator++() {
        return increment();
    }

    directory_iterator operator++(int) {
        directory_iterator prev = *this;
        increment();
        return prev;
    }

private:
    path p;
    directory_entry entry;
#if defined(_WIN32)
    HANDLE hFind = NULL;
    WIN32_FIND_DATA findData;
#else
    std::shared_ptr<DIR> dir = nullptr;
#endif
};

inline bool create_directory(const path& p) {
#if defined(_WIN32)
    return CreateDirectoryW(p.wstr().c_str(), NULL) != 0;
#else
    return mkdir(p.str().c_str(), S_IRWXU) == 0;
#endif
}

inline bool create_directories(const path& p) {
#if defined(_WIN32)
    return SHCreateDirectory(nullptr, p.make_absolute().wstr().c_str()) == ERROR_SUCCESS;
#else
    if (create_directory(p.str().c_str()))
        return true;

    if (p.empty())
        return false;

    if (errno == ENOENT) {
        if (create_directory(p.parent_path()))
            return mkdir(p.str().c_str(), S_IRWXU) == 0;
        else
            return false;
    }
    return false;
#endif
}

inline bool exists(const path& p) {
    return p.exists();
}

inline bool is_directory(const path& p) {
    return p.is_directory();
}

}; /* namespace filesystem */

#endif /* __FILESYSTEM_PATH_H */
