/*
copyright (C) 2007,2009-2010 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

///////////////////////////////////////////////////////////////////////////////
// EAFileUtil.h
// 
// created by Paul Pedriana - 2007
//
// Implements various file and directory manipulation functions.
///////////////////////////////////////////////////////////////////////////////

#if !defined(EAIO_EAFILEUTIL_H) && !defined(FOUNDATION_EAFILEUTIL_H)
#define EAIO_EAFILEUTIL_H
#define FOUNDATION_EAFILEUTIL_H // For backward compatability, eventually we'll want to remove all refernces to FOUNDATION_EAFILEUTIL_H.


#ifndef INCLUDED_eabase_H
    #include <eastl/EABase/eabase.h>
#endif
#include <eaio/internal/Config.h>
#ifndef EAIO_EAFILEBASE_H
    #include <eaio/EAFileBase.h>
#endif
#ifndef EAIO_EASTREAM_H
    #include <eaio/EAStream.h>
#endif
#ifndef EASTL_STRING_H
   #include <eastl/string.h>
#endif
#ifndef EASTL_FIXEDSTRING_H
   #include <eastl/fixed_string.h>
#endif
#ifndef EAIO_EASTLCOREALLOCATOR_H
    #include <eaio/internal/EAIOEASTLCoreAllocator.h>
#endif

#include <stddef.h>
#include <time.h>


#ifdef _MSC_VER
    #pragma warning(push)           // We have to be careful about disabling this warning. Sometimes the warning is meaningful; sometimes it isn't.
    #pragma warning(disable: 4251)  // class (some template) needs to have dll-interface to be used by clients.
#endif


namespace EA
{
    namespace IO
    {
        // Note that file functions are generally in namespace File, while directory
        // functions are in namespace Directory. The reason for this is to avoid
        // a bunch of annoying defines in <windows.h>, most notably createFile,
        // DeleteFile, copyFile, moveFile, SetFileAttributes, GetFileAttributes,
        // createDirectory, and removeDirectory.
        //
        // It is suggested that you not 'using namespace' the File and Directory
        // inner namespaces to avoid conflicts.

        namespace File
        {
            /// File::exists
            /// Returns true if the given file exists. If the path exists but is a path
            /// to a directory, the return value will be false.
            EAIO_API bool exists(const char16_t* pPath);
            EAIO_API bool exists(const char8_t* pPath);

            /// File::patternExists
            /// Returns true if the given file exists. Returns false if the file 
            /// doesn't exist or if there is a directory of the same name
            /// as the input file. The input path may use simple wildcard
            /// matching with '?' and '*' characters.
            EAIO_API bool patternExists(const char16_t* pPathPattern); 
            EAIO_API bool patternExists(const char8_t* pPathPattern); 

            /// File::create
            /// creates a file at the given path. If the file already exists at the 
            /// given path, then if bTruncate is true the file is truncated to zero bytes.
            /// The return value is true if upon completion of this function there 
            /// is a file at the given path and the bTruncate value could be honored. 
            /// Otherwise the return value is false and the error can be retrived via errno.
            /// The created file is not open upon return of this function.
            /// If you want finer control over how the file is created, use the FileStream class.
            EAIO_API bool create(const char16_t* pPath, bool bTruncate = false);
            EAIO_API bool create(const char8_t* pPath, bool bTruncate = false);

            /// File::remove
            /// Deletes the given file. Returns true if it could be deleted.
            /// Returns false if the file doesn't exist.
            EAIO_API bool remove(const char16_t* pPath);
            EAIO_API bool remove(const char8_t* pPath);

            /// File::move
            /// moves the given file. If the old and new path refer to the same
            /// directories, the move acts as a file rename. The return value is
            /// true if the file could be moved to the new location and the old
            /// file could be removed. 
            /// On some platforms, file moves are not natively supported and 
            /// the move must be emulated by a copy + delete. It is possible in 
            /// these cases the copy may succeed but the delete may fail and 
            /// thus leave the source file as it was.
            EAIO_API bool move(const char16_t* pPathSource, const char16_t* pPathDestination, bool bOverwriteIfPresent = true);
            EAIO_API bool move(const char8_t* pPathSource, const char8_t* pPathDestination, bool bOverwriteIfPresent = true);

            /// File::rename
            /// renames a file. Returns true if the file could be renamed. If the
            /// return value if false, the original file will remain as it was before.
            inline
            bool rename(const char16_t* pPathSource, const char16_t* pPathDestination, bool bOverwriteIfPresent = true)
                { return move(pPathSource, pPathDestination, bOverwriteIfPresent); }

            inline
            bool rename(const char8_t* pPathSource, const char8_t* pPathDestination, bool bOverwriteIfPresent = true)
                { return move(pPathSource, pPathDestination, bOverwriteIfPresent); }

            /// File::copy
            /// Copies a file from one location to another. Returns true if the 
            /// copy could be successfully completed as specified. If bOverwriteIfPresent 
            /// is true, then this function will overwrite an existing destination 
            /// file if it already exists. If false, then the copy will do nothing
            /// and return false if the destination file already exists.
            EAIO_API bool copy(const char16_t* pPathSource, const char16_t* pPathDestination, bool bOverwriteIfPresent = true);
            EAIO_API bool copy(const char8_t* pPathSource, const char8_t* pPathDestination, bool bOverwriteIfPresent = true);

            /// File::getSize
            /// Returns the length of the file at the given path. Returns (size_type)-1
            /// (a.k.a. kSizeTypeError) if the file doesn't exist or the length could 
            /// not be determined.
            EAIO_API size_type getSize(const char16_t* pPath);
            EAIO_API size_type getSize(const char8_t* pPath);

            /// File::isWritable
            /// Returns true if the file can be written to. This is a true writability test
            /// and not simply a test for a 'writable' file attribute.
            EAIO_API bool isWritable(const char16_t* pPath);
            EAIO_API bool isWritable(const char8_t* pPath);

            /// File::getAttributes
            /// Gets a bitmask of enum Attribute for the file at the given path.
            /// Returns zero (kAttributeNone) if the file could not be found.
            EAIO_API int getAttributes(const char16_t* pPath);
            EAIO_API int getAttributes(const char8_t* pPath);

            /// File::setAttributes
            /// Sets or clears the specified attributes of the given file. Has no 
            /// effect on other attributes. Returns true if the attributes could
            /// all be set as desired.
            EAIO_API bool setAttributes(const char16_t* pPath, int nAttributeMask, bool bEnable);
            EAIO_API bool setAttributes(const char8_t* pPath, int nAttributeMask, bool bEnable);

            /// File::getTime
            /// Allows you to get creation time, last modification time, last access time
            /// or any other timeType as identified by enum TimeType. Return value is in
            /// the format of the C time_t data type. A return value of zero means the  
            /// operation was not possible.
            EAIO_API time_t getTime(const char16_t* pPath, FileTimeType timeType);
            EAIO_API time_t getTime(const char8_t* pPath, FileTimeType timeType);

            /// File::setTime
            /// Sets the time of the given FileTimeType attribute(s) via the specified 
            /// nTime input. nTime is of the C time_t format, which can be read via 
            /// the C time() function (from time.h). Returns true if the request could
            /// be successfully honored.
            EAIO_API bool setTime(const char16_t* pPath, int nFileTimeTypeFlags, time_t nTime); 
            EAIO_API bool setTime(const char8_t* pPath, int nFileTimeTypeFlags, time_t nTime); 


            /// resolveAliasResult
            /// Defines valid return values for the resolveAlias function.
            enum resolveAliasResult {
                kRARInvalid,    /// The path was invalid
                kRARNone,       /// The path is not an alias
                kRARAlias       /// The path is an alias.
            };

            /// File::resolveAlias
            /// Some platforms (e.g. Windows) have the concept of file aliases or shortcuts. 
            /// This function converts such a path to its final destination path.
            /// If the destination path is non-NULL, the result path is copied to it. In this
            /// case even if the source is not an alias, it is copied to the destination. This is
            /// so even if the source file could not be found. The destination and source paths
            /// may be the same block of memory. The destination path is filled in regardless of
            /// the return value. If the path is not an alias, the source is copied to the destination.
            /// 
            /// Windows supports explicit aliases (.lnk files) and implicit aliases (junctions).
            /// Unix supports implicit aliases (symbolic links and hard links).
            /// Mac supports implicit aliases (symbolic links and hard links) and smart aliases (called simply "aliases" by Apple).
            /// Console platforms support no kind of file system aliases.
            ///
            /// This function does not work on all platforms. As of this writing, we have support 
            /// only for creating and resolving Windows explicit aliases.
            /// 
            EAIO_API resolveAliasResult resolveAlias(const char16_t* pPathSource, char16_t* pPathDestination, uint32_t nDestLength = kMaxPathLength);
            EAIO_API resolveAliasResult resolveAlias(const char8_t* pPathSource, char8_t* pPathDestination, uint32_t nDestLength = kMaxPathLength);


            /// File::createAlias
            /// creates a shortcut file with a given set of parameters.
            /// Shortcuts here are not the same thing as WinNT junctions and Unix symbolic links.
            /// The pDestinationPath and pShortcutPath parameters must be valid.
            /// The pShortcutDescription and pShortcutArguments parameters may be NULL, in which
            /// case they will be ignored.
            /// This function does not work on all platforms. As of this writing, we have support 
            /// only for creating and resolving Windows explicit aliases.
            ///
            EAIO_API bool createAlias(const char16_t* pDestinationPath, const char16_t* pShortcutPath, 
                                      const char16_t* pShortcutDescription, const char16_t* pShortcutArguments);
            EAIO_API bool createAlias(const char8_t* pDestinationPath, const char8_t* pShortcutPath, 
                                      const char8_t* pShortcutDescription, const char8_t* pShortcutArguments);

        } // namespace File


        namespace Directory
        {
            /// Directory::exists
            /// Returns true if the given input pDirectoryPath exists. If the path exists
            /// but the path refers to a file and not a directory, the return value is false.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool exists(const char16_t* pDirectory);
            EAIO_API bool exists(const char8_t* pDirectory);

            /// Directory::ensureExists
            /// Makes sure the directory exists, creates it if needed. 
            /// Returns true if the directory existed or could be created.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool ensureExists(const char16_t* pDirectory);
            EAIO_API bool ensureExists(const char8_t* pDirectory);

            /// Directory::create
            /// Makes sure the directory exists; creates it if it doesn't exist. 
            /// The specified directory must end with a trailing path separator.
            /// Returns true if it exists after the function return. This function is 
            /// smart enough to create the parent directories of the directory if they 
            /// happen to also not exist. This is as opposed to the standard C library 
            /// directory creation function which only succeeds if the directory's parent
            /// already exists. If unable to build the directory path, this function 
            /// restores the directory system state to what it was before the attempt
            /// to create the directory; thus any directories created are removed.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            /// It seems on linux if you create relative paths it doesn't work like it should, for example
            /// creating "test/" will not work, but "./test/" WILL work!
            EAIO_API bool create(const char16_t* pDirectory);
            EAIO_API bool create(const char8_t* pDirectory);

            /// Directory::remove
            /// removes the given directory. If the input 'bAllowRecursiveRemoval' is 
            /// true, then this function will also remove all files and directories below
            /// the input directory. If 'bAllowRecursiveRemoval' is false and there are 
            /// child files or directories, the removal attempt will fail.
            /// The specified directory must end with a trailing path separator.
            /// Returns true if the directory could be removed.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool remove(const char16_t* pDirectory, bool bAllowRecursiveRemoval = true);
            EAIO_API bool remove(const char8_t* pDirectory, bool bAllowRecursiveRemoval = true);

            /// Directory::rename
            /// renames the directory. This function cannot be used to move a directory; the old
            /// and new directory names must refer to a directory with the same parent. The input
            /// 'pDirectoryOld' must be a full directory path. The input 'pDirectoryNew' can be 
            /// either a directory path or just an individual directory name but if it is a path 
            /// then only the last name within the path will be used.
            /// The specified directory must end with a trailing path separator.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool rename(const char16_t* pDirectoryOld, const char16_t* pDirectoryNew);
            EAIO_API bool rename(const char8_t* pDirectoryOld, const char8_t* pDirectoryNew);

            /// Directory::copy
            /// This function performs a deep copy of a directory structure from the source to the 
            /// destination, creating any entries it needs along the way. If bOverwriteIfAlreadyPresent
            /// is true, then any entries that exist at the destination before the copy are overwritten
            /// by the source. Any entries that exist at the destination that aren't present in the 
            /// source are left in place; you must use removeDirectory if you want to ensure that these
            /// are removed as well.
            /// Returns true if the entire operation could be completed without copy error. If an error
            /// copying a file or subdirectory is encountered, the operation continues to proceed.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool copy(const char16_t* pDirectorySource, const char16_t* pDirectoryDestination, bool bRecursive, bool bOverwriteIfAlreadyPresent = true);
            EAIO_API bool copy(const char8_t* pDirectorySource, const char8_t* pDirectoryDestination, bool bRecursive, bool bOverwriteIfAlreadyPresent = true);

            /// Directory::getAttributes
            /// Directory::setAttributes
            /// Currently these are the same as for files.
            using File::getAttributes;
            using File::setAttributes;

            /// Directory::setAttributes
            /// Sets or clears the given attributes (see enum Attribute) for all directory
            /// entries recursively. This is useful for preparing for a mass deletion of 
            /// a directory tree, for example. The attribute application applies to both 
            /// file and directory entries. If bIncludeBaseDirectory is true, the attributes
            /// are applied to the base directory entry itself as wel as its children.
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool setAttributes(const char16_t* pBaseDirectory, int nAttributeMask, bool bEnable, bool bRecursive, bool bIncludeBaseDirectory = true);
            EAIO_API bool setAttributes(const char8_t* pBaseDirectory, int nAttributeMask, bool bEnable, bool bRecursive, bool bIncludeBaseDirectory = true);

            /// Directory::getTime
            /// Allows you to get creation time, last modification time, last access time
            /// or any other timeType as identified by enum TimeType. Return value is in
            /// the format of the C time_t data type. A return value of zero means the  
            /// operation was not possible.
            EAIO_API time_t getTime(const char16_t* pPath, FileTimeType timeType);
            EAIO_API time_t getTime(const char8_t* pPath, FileTimeType timeType);

            /// Directory::setTime
            /// Sets the time of the given FileTimeType attribute(s) via the specified 
            /// nTime input. nTime is of the C time_t format, which can be read via 
            /// the C time() function (from time.h). Returns true if the request could
            /// be successfully honored.
            EAIO_API bool setTime(const char16_t* pPath, int nFileTimeTypeFlags, time_t nTime); 
            EAIO_API bool setTime(const char8_t* pPath, int nFileTimeTypeFlags, time_t nTime); 

            /// Directory::getCurrentWorkingDirectory
            /// Returns a full path to the current working directory.
            /// The output pDirectory must be of at least length kMaxDirectoryLength.
            /// Returns the strlen of the returned path, which may be zero if the 
            /// working directory is unspecified. Note that an empty string is not 
            /// the same as a string of "/", as "/" refers to the file system root.
            /// The returned directory will end with a trailing path separator.
            EAIO_API int getCurrentWorkingDirectory(char16_t* pDirectory, uint32_t nPathLength = kMaxPathLength);
            EAIO_API int getCurrentWorkingDirectory(char8_t* pDirectory, uint32_t nPathLength = kMaxPathLength);

            /// Directory::setCurrentWorkingDirectory
            /// Sets the current working directory to that specified. 
            /// Note that this function and all other functions in the EAFile/EADirectory system requires
            /// a directory path name that ends in a path separator. This is by design as it simplifies
            /// the specification of and manipulation of paths.
            EAIO_API bool setCurrentWorkingDirectory(const char16_t* pDirectory);
            EAIO_API bool setCurrentWorkingDirectory(const char8_t* pDirectory);


            /// Directory::isDirectoryEmpty
            /// Returns true if the directory is empty. The nDirectoryEntryFlags parameter specifies if 
            /// you want to see if the directory is empty of files or empty of directories. The bRecursive
            /// parameter specifies if the emptiness check applies to files within subdirectories in addition
            /// to the current directory. 
            /// For example, you want to see if a directory has no files in it nor any of its subdirectories,
            /// use isDirectoryEmpty(<dirpath>, kDirectoryEntryFile, true). If you want to see if a directory 
            /// has any subdirectories, use isDirectoryEmpty(<dirpath>, kDirectoryEntryDirectory, false).
            EAIO_API bool isDirectoryEmpty(const char16_t* pDirectory, int nDirectoryEntryFlags = kDirectoryEntryFile, bool bRecursive = true);
            EAIO_API bool isDirectoryEmpty(const char8_t*  pDirectory, int nDirectoryEntryFlags = kDirectoryEntryFile, bool bRecursive = true);

        } // namespace Directory


        /// initializeFileSystem
        /// This is a manual file system initialization/shutdown function.
        /// You usually won't need to call this, as it is called automatically on startup.
        /// However, there may be cases where this is not so.
        EAIO_API bool initializeFileSystem(bool bInitialize);

        /// makeTempPathName
        /// Makes a temporary file name path for the user.
        /// Upon return, a file at the given path will be created with read/write 
        /// attributes and a size of zero. This is done so in order to reserve the 
        /// path's usability for the caller.
        /// The return value is true if the given path could be created.
        /// If the input pDirectory is non-NULL, then the temp file is created in 
        /// that directory. Otherwise, a default temp directory will be used.
        /// An input directory must end with a trailing path separator.
        /// If the input pFileName is non-NULL, it is used as a prefix for the temp
        /// file name. Otherwise an internal default will be used.
        /// If the pExtension parameter is non-NULL, then the returned path will have a file name with the given
        /// extension. The input pExtension should begin with a '.' character and must not be any longer than 32 characters.
        /// The output pPath must be able to hold at least kMaxPathLength characters.
        /// Note that this function and all other functions in the EAFile/EADirectory system requires
        /// a directory path name that ends in a path separator. This is by design as it simplifies
        /// the specification of and manipulation of paths.
        EAIO_API bool makeTempPathName(char16_t* pDestPath, const char16_t* pDirectory = NULL, const char16_t* pFileName = NULL, const char16_t* pExtension = NULL, uint32_t nDestPathLength = kMaxPathLength);
        EAIO_API bool makeTempPathName(char8_t* pDestPath, const char8_t* pDirectory = NULL, const char8_t* pFileName = NULL, const char8_t* pExtension = NULL, uint32_t nDestPathLength = kMaxPathLength);

        /// getTempDirectory
        /// Returns a standard temporary directory for the current user.
        /// The output pDirectory must be able to hold at least kMaxDirectoryLength characters.
        /// The output directory will end with a trailing path separator.
        /// Returns the strlen of the given name or -1 upon error.
        /// If an error occurs, the output directory will be unmodified.
        /// If the user has specified a non-empty temp directory via setTempDirectory,
        /// that directory will be returned. Otherwise a system-default temp directory
        /// will be returned.
        /// The returned directory path will end in a directory path separator.
        EAIO_API int getTempDirectory(char16_t* pDirectory, uint32_t nPathLength = kMaxPathLength);
        EAIO_API int getTempDirectory(char8_t* pDirectory, uint32_t nPathLength = kMaxPathLength);

        /// setTempDirectory
        /// Sets the temp directory that the user wishes to use. Future getTempDirectory
        /// calls will return this directory. If setTempDirectory is not called, a default
        /// directory path will be returned. The input pDirectory strlen must be no longer
        /// than kMaxDirectoryLength characters, or else the function will fail. 
        /// If the input directory doesn't end in a trailing path separator, one will be 
        /// appended. This trailing path charcater counts towards the input directory length.
        /// If the input directory doesn't exist, this function attempts to create it.
        /// Returns true if the specified directory exists upon the return of this function.
        /// The user can clear the temp directory by passing in an empty string.
        /// Note that this function and all other functions in the EAFile/EADirectory system requires
        /// a directory path name that ends in a path separator. This is by design as it simplifies
        /// the specification of and manipulation of paths.
        EAIO_API bool setTempDirectory(const char16_t* pDirectory);
        EAIO_API bool setTempDirectory(const char8_t* pDirectory);

        /// getDriveFreeSpace
        /// Returns an estimate of the amount of free space available on the 
        /// drive/volume that contains the given input path. The path can be 
        /// a file or directory path. Returns UINT64_C(0xffffffffffffffff) 
        /// (i.e. (uint64_t)-1) if the value could not be determined.
        EAIO_API uint64_t getDriveFreeSpace(const char16_t* pPath);
        EAIO_API uint64_t getDriveFreeSpace(const char8_t* pPath);

        /// getDriveName
        /// Gets the name for the drive/volume specified by the given input path.
        /// The output name should be able to hold at least kMaxVolumeNameLength characters.
        /// Note that the name of the drive isn't usually the same as the volume name, 
        /// though it may be. The name is the user-applied name to the drive, regardless
        /// of its drive letter or directory tree location.
        /// Returns the strlen of the given name or -1 upon error.
        /// If an error occurs, the output name will be unmodified.
        EAIO_API int getDriveName(const char16_t* pPath, char16_t* pName);
        EAIO_API int getDriveName(const char8_t* pPath, char8_t* pName);

        /// getDriveSerialNumber
        /// Gets the serial number for drive/volume specified by the given input path. 
        /// The path can consist of as little as a volume name (e.g. "d:").
        /// The output name should be able to hold at least kMaxVolumeSerialNumberLength characters.
        /// Returns the strlen of the given name or -1 upon error.
        /// If an error occurs, the output name will be unmodified.
        EAIO_API int getDriveSerialNumber(const char16_t* pPath, char16_t* pSerialNumber);
        EAIO_API int getDriveSerialNumber(const char8_t* pPath, char8_t* pSerialNumber);

        /// getDriveTypeValue
        /// Returns the DriveType associated with the given file path.
        /// The path can consist of as little as a volume name (e.g. "d:")
        /// or can contain a file path. A full path is useful or even necessary
        /// on platforms such as Unix which use mounted volumes.
        EAIO_API DriveType getDriveTypeValue(const char16_t* pPath);
        EAIO_API DriveType getDriveTypeValue(const char8_t* pPath);

        /// DriveInfo
        /// Used to store drive/volume infomation.
        /// Where the file system is case-independent, the return value will always be lower case.
        /// Where the file system has drives that end with ':', the ':' is the last character.
        struct EAIO_API DriveInfo
        {
            DriveInfo(const char16_t* pName = NULL, DriveType driveType = kDriveTypeUnknown);

            typedef eastl::fixedString<char16_t, 16, true, Allocator::EAIOEASTLCoreAllocator> DriveInfoName;

            DriveType     mType;  /// e.g. kDriveTypeFixed, kDriveTypeDVD, etc.
            DriveInfoName mName;  /// The format is dependent on the platform. For Win32-like platforms, the drive name is of the form "c:" (lower-case, no trailing slash).
        };
        #define DriveInfo16 DriveInfo // Backwards compatibility.

        struct EAIO_API DriveInfo8
        {
            DriveInfo8(const char8_t* pName = NULL, DriveType driveType = kDriveTypeUnknown);

            typedef eastl::fixedString<char8_t, 16, true, Allocator::EAIOEASTLCoreAllocator> DriveInfoName;

            DriveType     mType;  /// e.g. kDriveTypeFixed, kDriveTypeDVD, etc.
            DriveInfoName mName;  /// The format is dependent on the platform. For Win32-like platforms, the drive name is of the form "c:" (lower-case, no trailing slash).
        };

        /// isVolumeAvailable
        /// Determines if the mounted volume associated with the given path is available.
        /// Example usage:
        ///     bool available = isVolumeAvailable(L"/app_home/", 1000);
        EAIO_API bool isVolumeAvailable(const char16_t* pPath, int timeoutMS = 2000);
        EAIO_API bool isVolumeAvailable(const char8_t* pPath, int timeoutMS = 2000);

        /// getSpecialDirectory
        /// Gets the 'special directory' as defined by enum SpecialDirectory.
        /// This function is generally specific to desktop and server systems and not to 
        /// embedded or console systems.
        /// The output pDirectory must be of at least length kMaxDirectoryLength.
        /// If 'bEnsureDirectoryExistence' is true, then this function creates the given
        /// directory if not found.
        /// The return directory will end with a trailing path separator.
        /// Returns the strlen of the output directory or -1 upon error.
        /// If an error occurs, the output will be unmodified.
        /// If 'bEnsureDirectoryExistence' is true, then the directory must also 
        /// exist after the return of this function for the return value to be success.
        /// Note that this function and all other functions in the EAFile/EADirectory system requires
        /// a directory path name that ends in a path separator. This is by design as it simplifies
        /// the specification of and manipulation of paths.
        EAIO_API int getSpecialDirectory(SpecialDirectory specialDirectory, char16_t* pDirectory, bool bEnsureDirectoryExistence = false, uint32_t nPathLength = kMaxDirectoryLength);
        EAIO_API int getSpecialDirectory(SpecialDirectory specialDirectory, char8_t* pDirectory, bool bEnsureDirectoryExistence = false, uint32_t nPathLength = kMaxDirectoryLength);


        /// Returns true if the input character can always be used in a path file name.
        /// You cannot use this function one-by-one on a file name to determine if the file
        /// name is valid, as there are special combination cases that make this so.
        bool isFileNameCharValid(char16_t c, FileSystem fileSystemType);

        /// Returns true if the input file name is valid for the file system.
        bool isFileNameStringValid(const char16_t* pName, FileSystem fileSystemType);

        /// Returns true if the input directory name is valid for the file system.
        bool isDirectoryNameStringValid(const char16_t* pName, FileSystem fileSystemType);

        /// Returns true if the given path is valid. The path can consist of a full
        /// directory and file path, just a full directory path, or just a file name.
        /// fileSystemType refers to the file system, which defaults to the system's
        /// standard file system. 
        bool isFilePathStringValid(const char16_t* pPath, FileSystem fileSystemType);


    } // namespace IO


} // namespace EA


#ifdef _MSC_VER
    #pragma warning(pop)
#endif


#endif // Header include guard










