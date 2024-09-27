#ifndef __AGENT_FILE_SEARCHER_H__
#define __AGENT_FILE_SEARCHER_H__

#include <vector>
#include "common/Defines.h"

class AGENT_API CFileSearcher {
private:
    typedef std::vector<mp_string> PATHS;
    mp_string m_strPath;
    PATHS m_paths;


public:
    CFileSearcher() {};
    ~CFileSearcher() {}

    mp_void SetPath(const mp_string& pszPath);
    const mp_string& GetPath();
    mp_void AddPath(const mp_string& pszPath);
    mp_void Clear();
    mp_bool Search(const mp_string& pszFile, mp_string& strPath);
    mp_bool Search(const mp_string& pszFile, std::vector<mp_string>& flist);

private:
    mp_void RebuildPathString();
    mp_void AddPath(const mp_string& pszDir, mp_size sz);
    mp_bool IsExists(const mp_string& pszFile, const mp_string& strDir, mp_string& strPath);
};

#endif  // __AGENT_FILE_SEARCHER_H__
