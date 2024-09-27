/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef WIN32_FILESYSTEM_HANDLER_H
#define WIN32_FILESYSTEM_HANDLER_H

#ifdef WIN32
#include <windows.h>
#include <repository_handlers/RepositoryHandler.h>
#include <functional>

namespace VirtPlugin {
class Win32FileSystemHandler : public RepositoryHandler {
public:
    Win32FileSystemHandler() = default;
    ~Win32FileSystemHandler() = default;

    /**
     *  @brief 打开文件
     *
     *  @param fileName [IN]文件名
     *  @param mode     [IN]打开模式
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t Open(const std::string &fileName, const std::string &mode) override;

    /**
     * @brief 设置文件大小
     *
     * @param size
     * @return int32_t
     */
    int32_t Truncate(const uint64_t &size) override;

    /**
     *  @brief 关闭文件
     *
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t Close() override;

    /**
     *  @brief 读取数据
     *
     *  @param buf   [IN]读缓冲区
     *  @param count [IN]请求读取的字节数
     *  @return 成功返回读取的字节数，出错返回-1，如果在调用read之前已经到达文件末尾，则返回0
     */
    size_t Read(std::shared_ptr<uint8_t[]> buf, size_t count) override;

    /**
     *  @brief 读取数据
     *
     *  @param buf   [OUT]读缓冲区
     *  @param count [IN]请求读取的字节数
     *  @return 成功返回读取的字节数，出错返回-1，如果在调用read之前已经到达文件末尾，则返回0
     */
    size_t Read(std::string &buf, size_t count) override;

    /**
     *  @brief 写入数据
     *
     *  @param buf   [IN]写缓冲区
     *  @param count [IN]请求写入的字节数
     *  @return 成功返回写入的字节数，出错返回-1
     */
    size_t Write(const std::shared_ptr<uint8_t[]> &buf, size_t count) override;

    /**
     *  @brief 写入数据
     *
     *  @param str   [IN]待写入字符串
     *  @param count [IN]请求写入的字节数
     *  @return 成功返回写入的字节数，出错返回-1
     */
    size_t Write(const std::string &str) override;
    /**
     *  @brief 追加数据
     *
     *  @param buf   [IN]要追加的数据内容
     *  @param count [IN]请求追加的字节数
     *  @return 成功返回追加的字节数，出错返回-1
     */
    size_t Append(std::shared_ptr<uint8_t[]> buf, size_t count) override;

    /**
     *  @brief 获取打开文件的指针位置
     *
     *  @return 成功 返回给定文件的文件指针的位置，失败 返回-1
     */
    int64_t Tell() override;

    /**
     *  @brief 设置文件的指针位置
     *
     *  @return 成功 返回0，失败 返回-1
     */
    int64_t Seek(size_t offset, int origin = SEEK_SET) override;

    /**
     *  @brief 获取文件大小
     *
     *  @param fileName [IN]文件名
     *  @return 成功 文件大小，失败 返回-1
     */
    size_t FileSize(const std::string &fileName) override;

    /**
     *  @brief 数据下盘
     *
     *  @param sync [IN]是否同步刷新，缺省为同步方式
     *  @return true 成功，false 失败
     */
    bool Flush(bool sync = true) override;

    /**
     *  @brief 判断文件是否存在
     *
     *  @param fileName [IN]文件名
     *  @return true 存在，false 不存在
     */
    bool Exists(const std::string &fileName) override;

    /**
     *  @brief 重命名文件或者目录
     *
     *  @param oldName [IN]旧名称
     *  @param newName [IN]新名称
     *  @return true 成功，false 失败
     */
    bool Rename(const std::string &oldName, const std::string &newName) override;

    /**
     *  @brief 复制文件到目标目录或目标文件
     *
     *  @param oldName [IN]源文件名称
     *  @param newName [IN]目标文件名称或目录名称
     *  @return true 成功，false 失败
     */
    virtual bool CopyFile(const std::string &srcName, const std::string &destName) override;

    /**
     *  @brief 判断是否为目录
     *
     *  @param path [IN]目录名
     *  @return true 是目录，false 不是目录
     */
    virtual bool IsDirectory(const std::string &path) override;

    /**
     *  @brief 判断是否为普通文件
     *
     *  @param fileName [IN]文件名
     *  @return true 是普通文件，false 不是普通文件
     */
    virtual bool IsRegularFile(const std::string &fileName) override;

    /**
     *  @brief 移除文件
     *
     *  @param fileName [IN]文件名
     *  @return true 存在，false 不存在
     */
    bool Remove(const std::string &fileName) override;

    /**
     *  @brief 清空目录
     *
     *  @param dirName [IN]目录名
     *  @return true 成功，false 失败
     */
    bool RemoveAll(const std::string &dirName) override;

    /**
     *  @brief 创建目录
     *
     *  @param dirName [IN]目录名
     *  @return true 成功，false 失败
     */
    bool CreateDirectory(const std::string &dirName) override;

    /**
    *  @brief 获取目录下的所有文件名称
    *
    *  @param path [IN]路径名称
    *  @param files 当前目录下的文件名称
    */
    void GetFiles(std::string pathName, std::vector<std::string> &files) override;

private:
    std::vector<std::string> Tokenize(const std::string &str, char delimiter);
    bool RemoveAllInner(const std::string &path, std::function<int(const char *)> callback);
    bool HandleStateError(int errNo, const std::string &dir);
    bool Stat(const std::string &target, struct stat &statInfo, int32_t &errNo);

private:
    std::string m_fileName;
    std::string m_mode;
    HANDLE m_fileHandle = INVALID_HANDLE_VALUE;
};
}
#endif
#endif