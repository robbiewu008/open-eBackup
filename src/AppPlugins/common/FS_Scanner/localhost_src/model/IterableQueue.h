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
#ifndef FS_SCANNER_ITERABLE_QUEUE_H
#define FS_SCANNER_ITERABLE_QUEUE_H

#include <queue>

template<typename T, typename Container=std::deque<T>>
class IterableQueue : public std::queue<T, Container> {
public:
    using iterator = typename Container::iterator;
    using constIterator = typename Container::const_iterator;

    iterator Begin()
    {
        return this->c.begin();
    }
    iterator End()
    {
        return this->c.end();
    }
    constIterator Begin() const
    {
        return this->c.begin();
    }
    constIterator End() const
    {
        return this->c.end();
    }
};

#endif // FS_SCANNER_ITERABLE_QUEUE_H