#include "pch.h"
#include "FileIO.h"
#include "Buffer.h"
#include "Variant.h"
#include "Strings.h"

namespace fs = std::filesystem;

//std::string ToPlatformNotation(const std::wstring& Path)
//{
//	std::string UPath;
//	WStrToUtf8(UPath, Path);
//	for(;;)
//	{
//#ifdef WIN32
//		std::wstring::size_type Pos = UPath.find("/");
//		if(Pos == std::wstring::npos)
//			break;
//		UPath.replace(Pos,1,"\\");
//#else
//		std::wstring::size_type Pos = UPath.find("\\");
//		if(Pos == std::wstring::npos)
//			break;
//		UPath.replace(Pos,1,"/");
//#endif
//	}
//	return UPath;
//}
//
//std::wstring ToApplicationNotation(const std::string& UPath)
//{
//	std::wstring Path;
//	Utf8ToWStr(Path, UPath);
//	for(;;)
//	{
//		std::wstring::size_type Pos = Path.find(L"\\");
//		if(Pos == std::wstring::npos)
//			break;
//		Path.replace(Pos,1,L"/");
//	}
//	return Path;
//}

bool FileExists(const std::wstring& Path)
{
    if (GetFileAttributesW(Path.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        DWORD err = GetLastError();
        if(err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
            return false;
    }
    return true;
}

bool RemoveFile(const std::wstring& Path) 
{
    try 
    {
        return fs::remove(Path);
    } 
    catch (const std::exception& e) 
    {
        // Optionally log the error message: e.what()
        return false;
    }
}

bool RenameFile(const std::wstring& OldPath, const std::wstring& NewPath) 
{
    try {
        fs::rename(OldPath, NewPath);
        return true;
    } catch (const fs::filesystem_error&) {
        return false;
    }
}

bool WriteFile(const std::wstring& Path, const std::wstring& Data) 
{
    std::ofstream f(Path, std::ios::binary);
    if (!f) return false;

    std::string UData;
    WStrToUtf8(UData, Data);
    f.write(UData.data(), UData.size());

    return true;
}

bool ReadFile(const std::wstring& Path, std::wstring& Data) 
{
    std::ifstream f(Path, std::ios::binary);
    if (!f) return false;

    std::string UData((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    Utf8ToWStr(Data, UData);

    return true;
}

bool WriteFile(const std::wstring& Path, const std::string& Data) 
{
    std::ofstream f(Path, std::ios::binary);
    if (!f) return false;

    f.write(Data.data(), Data.size());

    return true;
}

bool ReadFile(const std::wstring& Path, std::string& Data) 
{
    std::ifstream f(Path, std::ios::binary);
    if (!f) return false;

    Data.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    return true;
}

bool WriteFile(const std::wstring& path, const CVariant& data) 
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    CBuffer buffer;
    data.ToPacket(&buffer);

    file.write(reinterpret_cast<const char*>(buffer.GetBuffer()), buffer.GetSize());
    return file.good();
}

bool ReadFile(const std::wstring& path, CVariant& data) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;

    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    CBuffer buffer(size, true);
    file.read(reinterpret_cast<char*>(buffer.GetBuffer()), size);

    if (!file)
        return false;

    data.FromPacket(&buffer);
    return true;
}

bool WriteFile(const std::wstring& path, std::uint64_t offset, const CBuffer& data) 
{
    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    file.seekp(offset);

    file.write(reinterpret_cast<const char*>(data.GetBuffer()), data.GetSize());
    return file.good();
}

bool ReadFile(const std::wstring& path, std::uint64_t offset, CBuffer& data) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;

    // Determine the size of the file
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (offset > size)
        return false;

    // If the size is specified in data, use that. Otherwise, use the file size.
    std::size_t toRead = data.GetSize() == 0 ? (size - offset) : data.GetSize();

    data.SetSize(toRead, true);

    file.seekg(offset);
    if (!file)
        return false;

    file.read(reinterpret_cast<char*>(data.GetBuffer()), toRead);
    return file.good();
}

bool WriteFile(const std::wstring& Path, const std::vector<BYTE>& Data)
{
    fs::remove(Path);

	std::ofstream f(Path, std::ios::binary);
	if (!f) return false;

	f.write(reinterpret_cast<const char*>(Data.data()), Data.size());

	return true;
}

bool ReadFile(const std::wstring& Path, std::vector<BYTE>& Data)
{
	std::ifstream f(Path, std::ios::binary);
	if (!f) return false;

	f.seekg(0, std::ios::end);
	std::size_t Size = f.tellg();
	f.seekg(0, std::ios::beg);

	Data.resize(Size);
	f.read(reinterpret_cast<char*>(Data.data()), Size);

	return true;
}

bool ListDir(const std::wstring& Path, std::vector<std::wstring>& Entries, const wchar_t* Filter)
{
    ASSERT(Path.back() == L'\\');

    for (const auto& entry : fs::directory_iterator(Path))
    {
        std::wstring Name = entry.path().filename().wstring();
        if (Filter && !wildcmpex(Filter, Name.c_str()))
            continue;

        if (entry.is_directory())
        {
            if (Name.compare(L"..") == 0 || Name.compare(L".") == 0)
                continue;
            Entries.push_back(Path + Name + L"\\");
        }
        else
        {
            Entries.push_back(Path + Name);
        }
    }
    return true;
}

unsigned long long GetFileSize(const std::wstring& Path)
{
    return fs::file_size(Path);
}

bool CreateFullPath(std::wstring Path) 
{
    wchar_t* tempPath = (wchar_t*)Path.c_str();
    
    // Iterate through the path and create each folder
    for (wchar_t* p = tempPath + 1; *p; p++) {
        if (*p == L'\\' || *p == L'/') {
            *p = L'\0'; // Temporarily terminate the string
            if (!CreateDirectoryW(tempPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
                return false;
            *p = L'\\'; // Restore the original character
        }
    }

    // Create the final directory in the path
    if (!CreateDirectoryW(tempPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        return false;

    return true;
}