/*
copyright (C) 2009-2010 Electronic Arts, Inc.  All rights reserved.

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
// PathString.cpp
// 
// copyright (c) 2006, Electronic Arts. All Rights Reserved.
// Written and maintained by Talin and Paul Pedriana.
///////////////////////////////////////////////////////////////////////////////


#include <eaio/internal/Config.h>
#include <eaio/PathString.h>
#include <eaio/EAFileBase.h>
#include <eaio/FnEncode.h>
#include <eacoreallocator/icoreallocator_interface.h>
#include EA_ASSERT_HEADER


namespace EA
{
namespace IO
{
namespace Path
{


namespace
{

    inline bool IsDirectorySeparator(char16_t c)
    {
        return (c == kFilePathSeparator16) || 
               (c == kFilePathSeparatorAlt16);
    }

    inline bool IsDirectorySeparator(char8_t c)
    {
        return (c == kFilePathSeparator8) || 
               (c == kFilePathSeparatorAlt8);
    }


    bool HasDrivePrefix(PathString16::const_iterator first, PathString16::const_iterator last)
    {
        for(; first < last; ++first)
        {
            if(*first == kFilePathDriveSeparator16) 
                return true;
            if(IsDirectorySeparator(*first)) // Check for this second, given that ':' is a directory separator.
                return false;
        }
        return false;
    }

    bool HasDrivePrefix(PathString8::const_iterator first, PathString8::const_iterator last)
    {
        for(; first < last; ++first)
        {
            if(*first == kFilePathDriveSeparator8) 
                return true;
            if(IsDirectorySeparator(*first)) // Check for this second, given that ':' is a directory separator.
                return false;
        }
        return false;
    }


    inline bool HasUNCPrefix(PathString16::const_iterator first, PathString16::const_iterator last)
    {
        // UNC paths specifically use '\' and not '/'.
        return (((first + 2) <= last) && (first[0] == '\\') && (first[1] == '\\'));
    }

    inline bool HasUNCPrefix(PathString8::const_iterator first, PathString8::const_iterator last)
    {
        // UNC paths specifically use '\' and not '/'.
        return (((first + 2) <= last) && (first[0] == '\\') && (first[1] == '\\'));
    }

} // anonymous



//////////////////////////////////////////////////////////////////////////
EAIO_API int compare(PathString16::const_iterator a, PathString16::const_iterator a_end,
                     PathString16::const_iterator b, PathString16::const_iterator b_end)
{
    EA_ASSERT(a && b);

    if(a_end == kEndAuto16)
        a_end = StrEnd(a);

    if(b_end == kEndAuto16)
        b_end = StrEnd(b);

    for(;;)
    {
        if(a >= a_end)
        {
            if(b >= b_end) 
                return 0;
            return 1;
        } 
        else if(b >= b_end)
            return -1;
        else if(*a != *b)
        {
            if(!IsDirectorySeparator(*a))
                return 1;
            if(!IsDirectorySeparator(*b))
                return -1;
        }

        ++a;
        ++b;
    }
}

EAIO_API int compare(PathString8::const_iterator a, PathString8::const_iterator a_end,
                     PathString8::const_iterator b, PathString8::const_iterator b_end)
{
    EA_ASSERT(a && b);

    if(a_end == kEndAuto8)
        a_end = StrEnd(a);

    if(b_end == kEndAuto8)
        b_end = StrEnd(b);

    for(;;)
    {
        if(a >= a_end)
        {
            if(b >= b_end) 
                return 0;
            return 1;
        } 
        else if(b >= b_end)
            return -1;
        else if(*a != *b)
        {
            if(!IsDirectorySeparator(*a))
                return 1;
            if(!IsDirectorySeparator(*b))
                return -1;
        }

        ++a;
        ++b;
    }
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator findComponentFwd(
    PathString16::iterator first,
    PathString16::iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto16) 
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    // Skip over initial UNC separators
    if(HasUNCPrefix(first, last))
        first += 2;

    // Then, skip over any non-separator chars (if drive separator encountered,
    // then break)
    while(first < last && !IsDirectorySeparator(*first))
    {
        if(*first == kFilePathDriveSeparator16)
        { 
            first++; 
            break; 
        }
        ++first;
    }

    // Skip over trailing separator
    if((first < last) && IsDirectorySeparator(*first)) 
        ++first;

    return first;
}


EAIO_API PathString8::iterator findComponentFwd(
    PathString8::iterator first,
    PathString8::iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto8) 
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    // Skip over initial UNC separators
    if(HasUNCPrefix(first, last))
        first += 2;

    // Then, skip over any non-separator chars (if drive separator encountered,
    // then break)
    while(first < last && !IsDirectorySeparator(*first))
    {
        if(*first == kFilePathDriveSeparator8)
        { 
            first++; 
            break; 
        }
        ++first;
    }

    // Skip over trailing separator
    if((first < last) && IsDirectorySeparator(*first)) 
        ++first;

    return first;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator findComponentFwd(
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    return findComponentFwd(const_cast<PathString16::iterator>(first), 
                            const_cast<PathString16::iterator>(last));
}

EAIO_API PathString8::const_iterator findComponentFwd(
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    return findComponentFwd(const_cast<PathString8::iterator>(first), 
                            const_cast<PathString8::iterator>(last));
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator findComponentRvs(
    PathString16::iterator first,
    PathString16::iterator last)
{
    EA_ASSERT(first);

    if(last == kEndAuto16) 
        last = StrEnd(first);
    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    // Skip over any file path separators
    if((last > first) && IsDirectorySeparator(last[-1])) 
        --last;

    // Skip over drive separator
    if((last > first) && (last[-1] == kFilePathDriveSeparator16)) 
        --last;

    // Skip over any non-file-path separators
    while((last > first) && !IsDirectorySeparator(last[-1]) && (last[-1] != kFilePathDriveSeparator16)) 
        --last;

    // Skip over UNC prefix.
    if((last == first + 2) && IsDirectorySeparator(first[0]) && IsDirectorySeparator(first[1]))
        last = first;

    return last;
}

EAIO_API PathString8::iterator findComponentRvs(
    PathString8::iterator first,
    PathString8::iterator last)
{
    EA_ASSERT(first);

    if(last == kEndAuto8) 
        last = StrEnd(first);
    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    // Skip over any file path separators
    if((last > first) && IsDirectorySeparator(last[-1])) 
        --last;

    // Skip over drive separator
    if((last > first) && (last[-1] == kFilePathDriveSeparator8)) 
        --last;

    // Skip over any non-file-path separators
    while((last > first) && !IsDirectorySeparator(last[-1]) && (last[-1] != kFilePathDriveSeparator8)) 
        --last;

    // Skip over UNC prefix.
    if((last == first + 2) && IsDirectorySeparator(first[0]) && IsDirectorySeparator(first[1]))
        last = first;

    return last;
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator findComponentRvs(
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    return findComponentRvs(const_cast<PathString16::iterator>(first), 
                            const_cast<PathString16::iterator>(last));
}

EAIO_API PathString8::const_iterator findComponentRvs(
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    return findComponentRvs(const_cast<PathString8::iterator>(first), 
                            const_cast<PathString8::iterator>(last));
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator getPathComponentStart(
    PathString16::iterator first,
    PathString16::iterator last,
    int32_t nIndex)
{
    EA_ASSERT(first);
    if(last == kEndAuto16) 
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    if(nIndex >= 0){
        // Positive index - search forward
        while(first < last && nIndex > 0){
            first = findComponentFwd(first, last);
            nIndex--;
        }
        return first;
    } 
    else{
        while(last > first && nIndex < 0){
            last = findComponentRvs(first, last);
            nIndex++;
        }
        return last;
    }
}

EAIO_API PathString8::iterator getPathComponentStart(
    PathString8::iterator first,
    PathString8::iterator last,
    int32_t nIndex)
{
    EA_ASSERT(first);
    if(last == kEndAuto8) 
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    if(nIndex >= 0)
    {
        // Positive index - search forward
        while(first < last && nIndex > 0)
        {
            first = findComponentFwd(first, last);
            nIndex--;
        }
        return first;
    } 
    else
    {
        while(last > first && nIndex < 0)
        {
            last = findComponentRvs(first, last);
            nIndex++;
        }
        return last;
    }
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator getPathComponentEnd(
    PathString16::iterator first,
    PathString16::iterator last,
    int32_t nIndex)
{
    //if(last == kEndAuto16) 
        last = StrEnd(first);

    if(nIndex >= 0){
        // Positive index - search forward
        nIndex++;
        PathString16::iterator result = first;
        while((result < last) && (nIndex > 0)){
            result = findComponentFwd(result, last);
            nIndex--;
        }
        // back up over the trailing separator
        if((nIndex == 0) && (result > first) && IsDirectorySeparator(result[-1])) 
            --result;
        return result;
    }
    else{
        nIndex++;
        PathString16::iterator result = last;
        while(result > first && nIndex < 0){
            result = findComponentRvs(first, result);
            nIndex++;
        }
        if((result > first) && IsDirectorySeparator(result[-1])) 
            --result;
        return result;
    }
}

EAIO_API PathString8::iterator getPathComponentEnd(
    PathString8::iterator first,
    PathString8::iterator last,
    int32_t nIndex)
{
    if(last == kEndAuto8) 
        last = StrEnd(first);

    if(nIndex >= 0)
    {
        // Positive index - search forward
        nIndex++;
        PathString8::iterator result = first;
        while((result < last) && (nIndex > 0))
        {
            result = findComponentFwd(result, last);
            nIndex--;
        }
        // back up over the trailing separator
        if((nIndex == 0) && (result > first) && IsDirectorySeparator(result[-1])) 
            --result;
        return result;
    }
    else
    {
        nIndex++;
        PathString8::iterator result = last;
        while(result > first && nIndex < 0)
        {
            result = findComponentRvs(first, result);
            nIndex++;
        }
        if((result > first) && IsDirectorySeparator(result[-1])) 
            --result;
        return result;
    }
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator getFileName(
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto16) 
        last = StrEnd(first);

    // If it's a trailing separator, then there's no file name
    if((first < last) && IsDirectorySeparator(last[-1])) 
        return (PathString16::iterator)last;

    PathString16::iterator fname = (PathString16::iterator)last;

    // Skip over any non-file-path separators
    while((fname > first)
        && !IsDirectorySeparator(fname[-1])
        && (fname[-1] != kFilePathDriveSeparator16))
            --fname;

    // A UNC machine name is not, I am afraid, a file name
    if((fname == first + 2) && HasUNCPrefix(first, last))
        return (PathString16::iterator)last;

    // Neither is a drive name
    //if(fname == first && HasDrivePrefix(first, last))
    //    return last;

    return fname;
}

EAIO_API PathString8::iterator getFileName(
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto8) 
        last = StrEnd(first);

    // If it's a trailing separator, then there's no file name
    if((first < last) && IsDirectorySeparator(last[-1])) 
        return (PathString8::iterator)last;

    PathString8::iterator fname = (PathString8::iterator)last;

    // Skip over any non-file-path separators
    while((fname > first)
        && !IsDirectorySeparator(fname[-1])
        && (fname[-1] != kFilePathDriveSeparator8))
    {
        --fname;
    }

    // A UNC machine name is not, I am afraid, a file name
    if((fname == first + 2) && HasUNCPrefix(first, last))
        return (PathString8::iterator)last;

    // Neither is a drive name
    //if(fname == first && HasDrivePrefix(first, last))
    //    return last;

    return fname;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator getFileName(const PathString16& path)
{
    return getFileName(
        const_cast<PathString16&>(path).begin(),
        const_cast<PathString16&>(path).end());
}

EAIO_API PathString8::const_iterator getFileName(const PathString8& path)
{
    return getFileName(
        const_cast<PathString8&>(path).begin(),
        const_cast<PathString8&>(path).end());
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator getFileExtension(
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto16) 
        last = StrEnd(first);

    // If it's a trailing separator, then there's no file name
    if(first < last && IsDirectorySeparator(last[-1]))
        return (PathString16::iterator)last;

    // If it has a UNC prefix, it needs to be treated specially
    if(HasUNCPrefix(first, last)){
        first = findComponentFwd(first, last);
    }

    for(PathString16::iterator it = (PathString16::iterator)last-1; it >= first; --it){
        if(IsDirectorySeparator(*it) || *it == kFilePathDriveSeparator16)
            break;
        if(*it == '.')
            return it;
    }

    return (PathString16::iterator)last;
}

EAIO_API PathString8::iterator getFileExtension(
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto8) 
        last = StrEnd(first);

    // If it's a trailing separator, then there's no file name
    if(first < last && IsDirectorySeparator(last[-1]))
        return (PathString8::iterator)last;

    // If it has a UNC prefix, it needs to be treated specially
    if(HasUNCPrefix(first, last))
        first = findComponentFwd(first, last);

    for(PathString8::iterator it = (PathString8::iterator)last-1; it >= first; --it)
    {
        if(IsDirectorySeparator(*it) || *it == kFilePathDriveSeparator8)
            break;
        if(*it == '.')
            return it;
    }

    return (PathString8::iterator)last;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator getFileExtension(const PathString16& path)
{
    return getFileExtension(
        const_cast<PathString16&>(path).begin(),
        const_cast<PathString16&>(path).end());
}

EAIO_API PathString8::const_iterator getFileExtension(const PathString8& path)
{
    return getFileExtension(
        const_cast<PathString8&>(path).begin(),
        const_cast<PathString8&>(path).end());
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::iterator getLocalRoot(
    PathString16::iterator first,
    PathString16::iterator last)
{
    if(last == kEndAuto16) 
        last = StrEnd(first);

    if(HasDrivePrefix(first, last))
        return first + 2;

    if(HasUNCPrefix(first, last))
        return getPathComponentStart(first, last, 1);

    return first;
}

EAIO_API PathString8::iterator getLocalRoot(
    PathString8::iterator first,
    PathString8::iterator last)
{
    if(last == kEndAuto8) 
        last = StrEnd(first);

    if(HasDrivePrefix(first, last))
        return first + 2;

    if(HasUNCPrefix(first, last))
        return getPathComponentStart(first, last, 1);

    return first;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator getLocalRoot(const PathString16& path)
{
    return getLocalRoot(
        const_cast<PathString16&>(path).begin(),
        const_cast<PathString16&>(path).end());
}

EAIO_API PathString8::const_iterator getLocalRoot(const PathString8& path)
{
    return getLocalRoot(
        const_cast<PathString8&>(path).begin(),
        const_cast<PathString8&>(path).end());
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& append(
    PathString16& dst,
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto16)
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    if(first == last)
        return dst;

    if(isRelative(first, last))
    {
        if(!dst.empty()) // It makes little sense to append a '/' char to an empty dst, as an empty dst means there is no dst and the appended directory is all there is.
            EnsureTrailingSeparator(dst);
    }
    else
        dst.clear();

    dst.append(first, last);
    return dst;
}

EAIO_API PathString8& append(
    PathString8& dst,
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    EA_ASSERT(first);
    if(last == kEndAuto8)
        last = StrEnd(first);

    EA_ASSERT(last);
    EA_ASSERT(first <= last);

    if(first == last)
        return dst;

    if(isRelative(first, last))
    {
        if(!dst.empty()) // It makes little sense to append a '/' char to an empty dst, as an empty dst means there is no dst and the appended directory is all there is.
            EnsureTrailingSeparator(dst);
    }
    else
        dst.clear();

    dst.append(first, last);
    return dst;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& append(PathString16& dst, const PathString16& suffix)
{
    return append(dst, suffix.begin(), suffix.end());
}

EAIO_API PathString8& append(PathString8& dst, const PathString8& suffix)
{
    return append(dst, suffix.begin(), suffix.end());
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& join(
    PathString16& dst,
    PathString16::const_iterator first,
    PathString16::const_iterator last)
{
    append(dst, first, last);
    return normalize(dst);
}

EAIO_API PathString8& join(
    PathString8& dst,
    PathString8::const_iterator first,
    PathString8::const_iterator last)
{
    append(dst, first, last);
    return normalize(dst);
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& join(PathString16& dst, const PathString16& suffix)
{
    return join(dst, suffix.begin(), suffix.end());
}

EAIO_API PathString8& join(PathString8& dst, const PathString8& suffix)
{
    return join(dst, suffix.begin(), suffix.end());
}

//////////////////////////////////////////////////////////////////////////
EAIO_API void split(const PathString16& path, PathString16* pDrive, PathString16* pDirectory, PathString16* pFileName, PathString16* pFileExtension)
{
    PathString16::const_iterator psDirectory     = getLocalRoot(path);
    PathString16::const_iterator psFileName      = getFileName(path);
    PathString16::const_iterator psFileExtension = getFileExtension(path);

    if(pDrive)
        pDrive->assign(path.c_str(), psDirectory);
    if(pDirectory)
        pDirectory->assign(psDirectory, psFileName);
    if(pFileName)
        pFileName->assign(psFileName, psFileExtension);
    if(pFileExtension)
        pFileExtension->assign(psFileExtension, path.c_str() + path.length());
}

EAIO_API void split(const PathString8& path, PathString8* pDrive, PathString8* pDirectory, PathString8* pFileName, PathString8* pFileExtension)
{
    PathString8::const_iterator psDirectory     = getLocalRoot(path);
    PathString8::const_iterator psFileName      = getFileName(path);
    PathString8::const_iterator psFileExtension = getFileExtension(path);

    if(pDrive)
        pDrive->assign(path.c_str(), psDirectory);
    if(pDirectory)
        pDirectory->assign(psDirectory, psFileName);
    if(pFileName)
        pFileName->assign(psFileName, psFileExtension);
    if(pFileExtension)
        pFileExtension->assign(psFileExtension, path.c_str() + path.length());
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16::const_iterator truncateComponent(PathString16& path, int nIndex){
    PathString16::iterator it = getPathComponentStart(path.begin(), path.end(), nIndex);
    return path.erase(it, path.end());
}

EAIO_API PathString8::const_iterator truncateComponent(PathString8& path, int nIndex){
    PathString8::iterator it = getPathComponentStart(path.begin(), path.end(), nIndex);
    return path.erase(it, path.end());
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& canonicalize(PathString16& path, char16_t separator){
    for(PathString16::iterator it = path.begin(); it != path.end(); ++it){
        if((*it == '/') || (*it =='\\')) // This may do the wrong thing with Unix-like paths.
            *it = separator;
    }
    return path;
}

EAIO_API PathString8& canonicalize(PathString8& path, char8_t separator){
    for(PathString8::iterator it = path.begin(); it != path.end(); ++it){
        if((*it == '/') || (*it =='\\')) // This may do the wrong thing with Unix-like paths.
            *it = separator;
    }
    return path;
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& PathStringnormalize(PathString16& path, bool bcanonicalize)
{
    PathString16::iterator first = path.begin();
    PathString16::iterator last  = path.end();
    PathString16::iterator out   = first;

    bool bHasNonLocalPart = HasUNCPrefix(first, last) || HasDrivePrefix(first, last);

    // copy initial UNC path
    while((first < last) && (*first == '\\'))
    {
        if(bcanonicalize)
            *out++ = kFilePathSeparator16;
        else
            *out++ = *first;
        ++first;
    }

    // Because simplification never increases the size of the path, we can simply
    // write the buffer in-place and adjust the size at the end.
    while(first < last)
    {
        if((first + 1 < last) && (first[0] == '.') && IsDirectorySeparator(first[1]))
        {
            first += 2;

            // Skip over doubled separator
            while(first < last && IsDirectorySeparator(*first)) 
                ++first;
            continue;
        }
        else if(first + 2 < last
            && first[0] == '.'
            && first[1] == '.'
            && IsDirectorySeparator(first[2])
            && out > path.begin())
        {
            // strip and that directory isn't a ".."; otherwise, copy the ".." literally 
            // (i.e. leave it in the string.)
            PathString16::iterator prev = findComponentRvs(path.begin(), out);

            if((prev > path.begin() || !bHasNonLocalPart) && 
                !((prev + 2 < last) && (prev[0] == '.') && (prev[1] == '.') && IsDirectorySeparator(prev[2])))
            {
                out = prev;
                first += 3;

                // Skip over doubled separator
                while(first < last && IsDirectorySeparator(*first)) 
                    ++first;
                continue;
            }
        }

        // copy next component
        while(first < last)
        {
            char16_t c = *first++;

            if(IsDirectorySeparator(c))
            {
                // Use the canonical separator, not the one in the string
                if(bcanonicalize)
                    *out++ = kFilePathSeparator16;
                else
                    *out++ = c;

                // Skip over doubled separator
                while(first < last && IsDirectorySeparator(*first))
                    ++first;
                break;
            } 
            else if(c == kFilePathDriveSeparator16){
                // Drive separator. Break here if the next char is not a dir sep.
                *out++ = c;
                if(first >= last || !IsDirectorySeparator(*out))
                    break;
            } 
            else{
                *out++ = c;
            }
        }
    }

    path.erase(out, last);
    return path;
}

EAIO_API PathString8& PathStringnormalize(PathString8& path, bool bcanonicalize)
{
    PathString8::iterator first = path.begin();
    PathString8::iterator last  = path.end();
    PathString8::iterator out   = first;

    bool bHasNonLocalPart = HasUNCPrefix(first, last) || HasDrivePrefix(first, last);

    // copy initial UNC path
    while((first < last) && (*first == '\\'))
    {
        if(bcanonicalize)
            *out++ = kFilePathSeparator8;
        else
            *out++ = *first;
        ++first;
    }

    // Because simplification never increases the size of the path, we can simply
    // write the buffer in-place and adjust the size at the end.
    while(first < last)
    {
        if((first + 1 < last) && (first[0] == '.') && IsDirectorySeparator(first[1])){
            first += 2;

            // Skip over doubled separator
            while(first < last && IsDirectorySeparator(*first)) 
                ++first;
            continue;
        }
        else if(first + 2 < last
            && first[0] == '.'
            && first[1] == '.'
            && IsDirectorySeparator(first[2])
            && out > path.begin())
        {
            // strip and that directory isn't a ".."; otherwise, copy the ".." literally 
            // (i.e. leave it in the string.)
            PathString8::iterator prev = findComponentRvs(path.begin(), out);

            if((prev > path.begin() || !bHasNonLocalPart) && 
                !((prev + 2 < last) && (prev[0] == '.') && (prev[1] == '.') && IsDirectorySeparator(prev[2])))
            {
                out = prev;
                first += 3;

                // Skip over doubled separator
                while(first < last && IsDirectorySeparator(*first)) 
                    ++first;
                continue;
            }
        }

        // copy next component
        while(first < last)
        {
            char8_t c = *first++;

            if(IsDirectorySeparator(c))
            {
                // Use the canonical separator, not the one in the string
                if(bcanonicalize)
                    *out++ = kFilePathSeparator8;
                else
                    *out++ = c;

                // Skip over doubled separator
                while(first < last && IsDirectorySeparator(*first))
                    ++first;
                break;
            } 
            else if(c == kFilePathDriveSeparator8)
            {
                // Drive separator. Break here if the next char is not a dir sep.
                *out++ = c;
                if(first >= last || !IsDirectorySeparator(*out))
                    break;
            } 
            else{
                *out++ = c;
            }
        }
    }

    path.erase(out, last);
    return path;
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& simplify(PathString16& path){
    return PathStringnormalize(path, false);
}

EAIO_API PathString8& simplify(PathString8& path){
    return PathStringnormalize(path, false);
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& normalize(PathString16& path){
    return PathStringnormalize(path, true);
}

EAIO_API PathString8& normalize(PathString8& path){
    return PathStringnormalize(path, true);
}


//////////////////////////////////////////////////////////////////////////
EAIO_API bool isRelative(PathString16::const_iterator first, PathString16::const_iterator last)
{
    if(last == kEndAuto16) 
        last = StrEnd(first);

    if(first >= last) // If it's empty
        return true;

    if(HasDrivePrefix(first, last)) // If begins with "C:\"
        return false;

    if((last > first) && IsDirectorySeparator(*first)) // If begins with \ or / (then it is absolute, either as a Unix rooted path or as a Microsoft UNC path)
        return false;

    return true;
}

EAIO_API bool isRelative(PathString8::const_iterator first, PathString8::const_iterator last)
{
    if(last == kEndAuto8) 
        last = StrEnd(first);

    if(first >= last) // If it's empty
        return true;

    if(HasDrivePrefix(first, last)) // If begins with "C:\"
        return false;

    if((last > first) && IsDirectorySeparator(*first)) // If begins with \ or / (then it is absolute, either as a Unix rooted path or as a Microsoft UNC path)
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////
EAIO_API bool isRelative(const PathString16& path)
{
    return isRelative(path.begin(), path.end());
}

EAIO_API bool isRelative(const PathString8& path)
{
    return isRelative(path.begin(), path.end());
}


//////////////////////////////////////////////////////////////////////////
EAIO_API int compare(const PathString16& a, const PathString16& b)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString16::const_iterator  a_first = a.begin(), a_last;
    PathString16::const_iterator  b_first = b.begin(), b_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while((a_first < a.end()) && (b_first < b.end()))
    {
        a_last = findComponentFwd(a_first, a.end());
        b_last = findComponentRvs(b_first, b.end());

        int result = compare(a_first, a_last, b_first, b_last);
        if(result)
            return result;

        a_first = a_last;
        b_first = b_last;
    }

    return 0;
}

EAIO_API int compare(const PathString8& a, const PathString8& b)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString8::const_iterator  a_first = a.begin(), a_last;
    PathString8::const_iterator  b_first = b.begin(), b_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while((a_first < a.end()) && (b_first < b.end()))
    {
        a_last = findComponentFwd(a_first, a.end());
        b_last = findComponentRvs(b_first, b.end());

        int result = compare(a_first, a_last, b_first, b_last);
        if(result)
            return result;

        a_first = a_last;
        b_first = b_last;
    }

    return 0;
}



//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& ComputeRelative(
    PathString16& result,
    const PathString16& source,
    const PathString16& target)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString16::const_iterator  source_first = source.begin(), source_last;
    PathString16::const_iterator  target_first = target.begin(), target_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while(source_first < source.end() && target_first < target.end())
    {
        source_last = findComponentFwd(source_first, source.end());
        target_last = findComponentRvs(target_first, target.end());

        int cResult = compare(source_first, source_last,
                              target_first, target_last);
        if(cResult)
            break;

        source_first = source_last;
        target_first = target_last;
    }

    // Clear the result buffer
    result.clear();

    // If both were equal, then we're done
    if(source_first == source.end() && target_first == target.end())
        return result;

    // If dest is not relative, then just use that
    if(!isRelative(target)){
        result = target;
        return result;
    }

    // Otherwise, add a ".." for each dir remaining in source
    while(source_first < source.end()){
        source_last = findComponentFwd(source_first, source.end());
        result.pushBack('.');
        result.pushBack('.');
        result.pushBack(kFilePathSeparator16);
        source_first = source_last;
    }

    return join(result, target_first, target.end());
}

EAIO_API PathString8& ComputeRelative(
    PathString8& result,
    const PathString8& source,
    const PathString8& target)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString8::const_iterator  source_first = source.begin(), source_last;
    PathString8::const_iterator  target_first = target.begin(), target_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while(source_first < source.end() && target_first < target.end()){
        source_last = findComponentFwd(source_first, source.end());
        target_last = findComponentRvs(target_first, target.end());

        int cResult = compare(source_first, source_last,
                              target_first, target_last);

        if(cResult)
            break;

        source_first = source_last;
        target_first = target_last;
    }

    // Clear the result buffer
    result.clear();

    // If both were equal, then we're done
    if(source_first == source.end() && target_first == target.end())
        return result;

    // If dest is not relative, then just use that
    if(!isRelative(target)){
        result = target;
        return result;
    }

    // Otherwise, add a ".." for each dir remaining in source
    while(source_first < source.end()){
        source_last = findComponentFwd(source_first, source.end());
        result.pushBack('.');
        result.pushBack('.');
        result.pushBack(kFilePathSeparator8);
        source_first = source_last;
    }

    return join(result, target_first, target.end());
}


//////////////////////////////////////////////////////////////////////////
EAIO_API bool IsSubdirectory(
    const PathString16& dir,
    const PathString16& sub)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString16::const_iterator  dir_first = dir.begin(), dir_last;
    PathString16::const_iterator  sub_first = sub.begin(), sub_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while(dir_first < dir.end() && sub_first < sub.end())
    {
        dir_last = findComponentFwd(dir_first, dir.end());
        sub_last = findComponentRvs(sub_first, sub.end());

        int result = compare(
            dir_first, dir_last,
            sub_first, sub_last);

        if(result)
            break;

        dir_first = dir_last;
        sub_first = sub_last;
    }

    return dir_first >= dir.end();
}

EAIO_API bool IsSubdirectory(
    const PathString8& dir,
    const PathString8& sub)
{
    // first and last refer to the start and end of an individual name
    // in the path.
    PathString8::const_iterator  dir_first = dir.begin(), dir_last;
    PathString8::const_iterator  sub_first = sub.begin(), sub_last;

    // compare each component one at a time, until we find a name that
    // doesn't match.
    while(dir_first < dir.end() && sub_first < sub.end())
    {
        dir_last = findComponentFwd(dir_first, dir.end());
        sub_last = findComponentRvs(sub_first, sub.end());

        int result = compare(
            dir_first, dir_last,
            sub_first, sub_last);

        if(result)
            break;

        dir_first = dir_last;
        sub_first = sub_last;
    }

    return dir_first >= dir.end();
}



//////////////////////////////////////////////////////////////////////////
EAIO_API bool GetHasTrailingSeparator(const PathString16& path)
{
    return (!path.empty() && IsDirectorySeparator(path.back()));
}

EAIO_API bool GetHasTrailingSeparator(const PathString8& path)
{
    return (!path.empty() && IsDirectorySeparator(path.back()));
}


//////////////////////////////////////////////////////////////////////////
EAIO_API bool GetHasTrailingSeparator(PathString16::const_iterator path, size_t nLength)
{
    if (nLength == kLengthNull)
        nLength = EAIOStrlen16(path);

    return ((nLength != 0) && IsDirectorySeparator(*(path + nLength - 1)));
}

EAIO_API bool GetHasTrailingSeparator(PathString8::const_iterator path, size_t nLength)
{
    if (nLength == kLengthNull)
        nLength = strlen(path);

    return ((nLength  != 0) && IsDirectorySeparator(*(path + nLength - 1)));
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& EnsureTrailingSeparator(PathString16& path)
{
    // To consider: should we add to an empty path?
    if(/*!path.empty() &&*/ !GetHasTrailingSeparator(path))
        path.pushBack(kFilePathSeparator16);
    return path;
}

EAIO_API PathString8& EnsureTrailingSeparator(PathString8& path)
{
    // To consider: should we add to an empty path?
    if(/*!path.empty() &&*/ !GetHasTrailingSeparator(path))
        path.pushBack(kFilePathSeparator8);
    return path;
}

EAIO_API bool EnsureTrailingSeparator(char16_t* pDirName, size_t nMaxPermittedLength)
{
    // To consider: should we add to an empty path?
    const size_t n = EAIOStrlen16(pDirName);

    if (!EA::IO::Path::GetHasTrailingSeparator(pDirName, (uint32_t)n))
    {
        if (n + 2 <= nMaxPermittedLength)
        {
            pDirName[n] = EA::IO::kFilePathSeparator16;
            pDirName[n + 1] = 0;

            return true;
        }
    }

    return false;
}

EAIO_API bool EnsureTrailingSeparator(char8_t* pDirName, size_t nMaxPermittedLength)
{
    // To consider: should we add to an empty path?
    const size_t n = strlen(pDirName);

    if (!EA::IO::Path::GetHasTrailingSeparator(pDirName, (uint32_t)n))
    {
        if (n + 2 <= nMaxPermittedLength)
        {
            pDirName[n] = EA::IO::kFilePathSeparator8;
            pDirName[n + 1] = 0;

            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////
EAIO_API PathString16& StripTrailingSeparator(PathString16& path)
{
    if(GetHasTrailingSeparator(path))
        path.popBack();
    return path;
}

EAIO_API PathString8& StripTrailingSeparator(PathString8& path)
{
    if(GetHasTrailingSeparator(path))
        path.popBack();
    return path;
}

EAIO_API void StripTrailingSeparator(char8_t* pPath, size_t nLength)
{
    if (nLength == kLengthNull)
        nLength = (uint32_t)strlen(pPath);

    if (pPath[nLength-1] == EA_FILE_PATH_SEPARATOR_8 || pPath[nLength-1] == EA_FILE_PATH_SEPARATOR_ALT_8)
        pPath[nLength-1] = 0;
}

EAIO_API void StripTrailingSeparator(char16_t* pPath, size_t nLength)
{
    if (nLength == kLengthNull)
        nLength = (uint32_t)EAIOStrlen16(pPath);

    if (pPath[nLength-1] == EA_FILE_PATH_SEPARATOR_16 || pPath[nLength-1] == EA_FILE_PATH_SEPARATOR_ALT_16)
        pPath[nLength-1] = 0;
}


} // namespace Path

} // namespace IO

} // namespace EA












