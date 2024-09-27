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
#pragma once

// std
#include <random>

// DataMove
#include <protocol/fuse/FuseMessage.h>

namespace DataMove {
std::uniform_int_distribution<std::mt19937_64::result_type> g_distribution(
    0, std::numeric_limits<std::mt19937_64::result_type>::max());

constexpr auto TOKEN = "ajhf72r61txygf71tr71t2y1hbhcjb;as0a=-s0d";
const auto TOKEN_LENGTH = std::strlen(TOKEN);
constexpr auto NEW_TOKEN = "kljhafjhjfhsjkhf89-24yur82ryu2-8ty28 327";
constexpr auto TARGET_SOURCE = "TargetSource";
const auto TARGET_SOURCE_LENGTH = std::strlen(TARGET_SOURCE);
constexpr auto ENTRY_NAME = "EntryName.txt";
const auto ENTRY_NAME_LENGTH = std::strlen(ENTRY_NAME);
constexpr auto ENTRY_NAME_TWO = "FOOBAR.txt";
const auto ENTRY_NAME_TWO_LENGTH = std::strlen(ENTRY_NAME_TWO);
constexpr int ERROR = 123;
constexpr int EXPECTED_SIZE = 4096;
constexpr int EXPECTED_OFFSET = 987;

constexpr std::size_t FUSE_MESSAGE_REQUEST_HEADER_SIZE =
    sizeof(Protocol::Fuse::FuseMessageClass) +  // Size of a fuse message class
    sizeof(Protocol::Fuse::FuseMessageType) +   // Size of a fuse message type
    sizeof(AuxDataSize) +                       // Size of a size of a token
    sizeof(AuxDataSize);                        // Size of a size of a target source

constexpr std::size_t FUSE_MESSAGE_RESPONSE_HEADER_SIZE =
    sizeof(Protocol::Fuse::FuseMessageClass) +  // Size of a fuse message class
    sizeof(Protocol::Fuse::FuseMessageType);    // Size of a fuse message type

fuse_entry_param GetEntryParams()
{
    std::random_device gDevice;
    std::mt19937_64 gGenerator(gDevice());
    fuse_entry_param params{};
    params.ino = g_distribution(gGenerator);
    params.generation = g_distribution(gGenerator);
    params.attr.st_ino = params.ino;
    params.attr.st_mode = S_IFREG;
    params.attr.st_dev = g_distribution(gGenerator);
    params.attr.st_uid = g_distribution(gGenerator);
    params.attr.st_gid = g_distribution(gGenerator);
    timespec time = { static_cast<__time_t>(g_distribution(gGenerator)),
                      static_cast<__syscall_slong_t>(g_distribution(gGenerator)) };
    params.attr.st_atim = time;
    params.attr.st_ctim = time;
    params.attr.st_mtim = time;
    params.attr.st_nlink = g_distribution(gGenerator);
    params.attr.st_size = static_cast<__off_t>(g_distribution(gGenerator));

    return params;
}

void CompareAttributes(const struct stat& first, const struct stat& second)
{
    ASSERT_EQ(first.st_ino, second.st_ino);
    ASSERT_EQ(first.st_mode, second.st_mode);
    ASSERT_EQ(first.st_dev, second.st_dev);
    ASSERT_EQ(first.st_uid, second.st_uid);
    ASSERT_EQ(first.st_gid, second.st_gid);
    ASSERT_EQ(first.st_atim.tv_nsec, second.st_atim.tv_nsec);
    ASSERT_EQ(first.st_ctim.tv_nsec, second.st_ctim.tv_nsec);
    ASSERT_EQ(first.st_mtim.tv_nsec, second.st_mtim.tv_nsec);
    ASSERT_EQ(first.st_nlink, second.st_nlink);
    ASSERT_EQ(first.st_size, second.st_size);
}

void CompareFuseEntryParams(const fuse_entry_param &first, const fuse_entry_param &second)
{
    ASSERT_EQ(first.ino, second.ino);
    ASSERT_EQ(first.generation, second.generation);
    ASSERT_EQ(first.attr_timeout, second.attr_timeout);
    ASSERT_EQ(first.entry_timeout, second.entry_timeout);
    CompareAttributes(first.attr, second.attr);
}
}
