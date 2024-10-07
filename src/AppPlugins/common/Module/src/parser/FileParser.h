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
#ifndef FS_SCANNER_FILE_PARSER_H
#define FS_SCANNER_FILE_PARSER_H

#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash.hpp>
#include "ParserUtils.h"
#include "securec.h"
#include <fstream>
#include "define/Defines.h"

namespace Module {
const uint32_t CTRL_WRITE_LINE_SIZE = 4608;

class  AGENT_API FileParser {
public:
    /**
    * Contructor
    */
    explicit FileParser(bool isBinaryFile) : m_binaryFlag(isBinaryFile) {};

    FileParser() {};
    /**
    *  Destructor
    */
    virtual ~FileParser() {};

    /**
    * Open file in read/write mode
    */
    CTRL_FILE_RETCODE Open(CTRL_FILE_OPEN_MODE mode);

    /**
    * flush all da  ta and Close file.
    */
    CTRL_FILE_RETCODE Close(CTRL_FILE_OPEN_MODE mode);

    // Close WriteFd
    void CloseWriteFD();

protected:
    std::mutex m_lock {};                       /* Lock */
    std::stringstream m_readBuffer {};          /* Read buffer */
    char *m_readBinaryBuffer = nullptr;         /* Read binary buffer */
    char *m_writeCtrlLine = nullptr;            /* Write control file line buffer */
    std::stringstream m_writeBuffer {};         /* Write buffer */
    std::ifstream m_readFd {};    /* Read FD */
    std::ofstream m_writeFd {};   /* Write FD */
    std::string m_fileName {};                  /* This file name */
    std::string m_fileParentDir {};             /* Parent dir of the file */
    bool m_binaryFlag = CTRL_BINARY_FILE;       /* Control file is binary or not */
    uint64_t m_readBufferSize = 0;              /* Read Buffer Size */

protected:
    virtual CTRL_FILE_RETCODE OpenWrite() = 0;
    virtual CTRL_FILE_RETCODE CloseWrite() = 0;

    /**
    * Write buffer data into file.
    */
    virtual CTRL_FILE_RETCODE FlushToFile() = 0;
    
    /**
    * Read the file header info from file and load to m_header
    */
    virtual CTRL_FILE_RETCODE ReadHeader() = 0;
    
    /**
    * Write the file header info to file from m_header
    */
    virtual CTRL_FILE_RETCODE WriteHeader() = 0;
    
    /**
    * Validate header information read from the file
    */
    virtual CTRL_FILE_RETCODE ValidateHeader() = 0;
    template<class FileStream>
    CTRL_FILE_RETCODE FileOpen(FileStream &strmFd, std::ios::openmode fileMode);
    CTRL_FILE_RETCODE WriteToAggrFile(bool isBinaryFile);
    CTRL_FILE_RETCODE ReadFromAggrFile();
    CTRL_FILE_RETCODE WriteToFile(std::stringstream& writeBuffer, bool isBinaryFile);
    CTRL_FILE_RETCODE ReadFromFile();
    CTRL_FILE_RETCODE ReadFromBinaryFile(uint64_t offset, uint32_t readLen);
};

}
#endif // FS_SCANNER_FILE_PARSER_H