#include "tools/agentcli/MergeConfFiles.h"
#include <algorithm>

namespace {
mp_string MERGINT_RES_FILE_NAME = "merging_res.ini";
}

mp_void MergeConfFiles::SetFilePath(mp_string filePathOld, mp_string filePathNew)
{
    mOldFileIni.fileName = filePathOld;
    mNewFileIni.fileName = filePathNew;
}

mp_int32 MergeConfFiles::ReadFileIni(FileIni &fileIni)
{
    DBGLOG("The path is [%s]", fileIni.fileName.c_str());
    mp_string path = fileIni.fileName;
    std::ifstream fileIniStream(path);
    if (!fileIniStream.is_open()) {
        ERRLOG("File [%s] not exists!", fileIni.fileName.c_str());
        return MP_FAILED;
    }
    mp_string readLine;
    vector<mp_string> content;
    while (getline(fileIniStream, readLine)) {
        content.push_back(readLine);
    }
    fileIniStream.close();
    ParseFileIni(fileIni, content);

    return MP_SUCCESS;
}

mp_void MergeConfFiles::ParseFileIni(FileIni &fileInitent, const vector<mp_string> &content)
{
    mp_string key = "Head";
    fileInitent.confSection.push_back(key);
    for (auto str : content) {
        vector<pair<mp_string, mp_string>> temp;
        if (str[0] == '[') {
            if (!temp.empty()) {
                fileInitent.fileIniContent[key] = temp;
            }
            key = str;
            fileInitent.confSection.push_back(str);
        } else {
            fileInitent.fileIniContent[key].push_back(SetPairs(str));
        }
    }
    return;
}

pair<mp_string, mp_string> MergeConfFiles::SetPairs(mp_string &fileLine)
{
    pair<mp_string, mp_string> res;
    if (fileLine.empty()) {
        return res;
    }
    if (fileLine[0] == ';') {
        res.first = fileLine;
    } else {
        size_t pos = fileLine.find("=");
        if (pos != mp_string::npos) {
            res.first = fileLine.substr(0, pos);
            res.second = fileLine.substr(pos);
        } else {
            res.first = fileLine;
        }
    }

    return res;
}

mp_void MergeConfFiles::MergeIniFiles(FileIni &fileIniOld, FileIni &fileIniNew)
{
    map<mp_string, vector<pair<mp_string, mp_string>>> fileNew = fileIniNew.fileIniContent;
    vector<mp_string> confSection = fileIniOld.confSection;
    for (auto itSection = fileNew.begin(); itSection != fileNew.end(); ++itSection) {
        auto itPair = find(confSection.begin(), confSection.end(), itSection->first.c_str());
        if (itPair != confSection.end()) {
            CheckSection(*itPair);
        } else {
            fileIniOld.confSection.push_back(itSection->first);
            fileIniOld.fileIniContent[itSection->first] = fileNew[itSection->first];
        }
    }
    return;
}

mp_void MergeConfFiles::CheckSection(mp_string section)
{
    if (mNewFileIni.fileIniContent[section] == mOldFileIni.fileIniContent[section]) {
        return;
    } else {
        for (auto it : mNewFileIni.fileIniContent[section]) {
            if (it.first[0] == ';') {
                continue;
            }
            if (!CheckKeyValuePair(it, mOldFileIni.fileIniContent[section])) {
                mOldFileIni.fileIniContent[section].push_back(it);
            }
        }
    }
    return;
}

bool MergeConfFiles::CheckKeyValuePair(pair<mp_string, mp_string> &pair1, vector<pair<mp_string, mp_string>> vec1)
{
    auto it = std::find_if(vec1.begin(), vec1.end(), [&](const auto &p) { return p.first == pair1.first; });
    if (it != vec1.end()) {
        return true;
    }
    return false;
}

mp_void MergeConfFiles::SaveInNewFile(const mp_string &filePathToSave)
{
    const vector<mp_string> content;
    ofstream file(filePathToSave);
    for (auto it : mOldFileIni.confSection) {
        if (it != "Head") {
            file << it << endl;
        }
        for (auto it2 = mOldFileIni.fileIniContent[it].begin(); it2 != mOldFileIni.fileIniContent[it].end(); ++it2) {
            file << it2->first << it2->second << endl;
        }
    }
    file.close();
}

mp_int32 MergeConfFiles::MergeFileHandle(const mp_string oldFilePath, const mp_string newFilePath)
{
    SetFilePath(oldFilePath, newFilePath);
    int iRet1 = ReadFileIni(mOldFileIni);
    int iRet2 = ReadFileIni(mNewFileIni);
    if (iRet1 != MP_SUCCESS || iRet2 != MP_SUCCESS) {
        ERRLOG("Configuration files open failed!");
        return MP_FAILED;
    }
    MergeIniFiles(mOldFileIni, mNewFileIni);
    mp_string filePathToSave = CPath::GetInstance().GetTmpFilePath(MERGINT_RES_FILE_NAME);
    if (CMpFile::CreateFile(filePathToSave) != MP_SUCCESS) {
        ERRLOG("Create res file failed, file is: %s", filePathToSave.c_str());
        return MP_FAILED;
    }

    SaveInNewFile(filePathToSave);
    return MP_SUCCESS;
}
