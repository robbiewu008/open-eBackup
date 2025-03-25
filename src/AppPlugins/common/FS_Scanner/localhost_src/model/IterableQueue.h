/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 6/24/2022.
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