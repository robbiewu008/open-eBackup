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
#include "BuildQueryCondition.h"
#include <sstream>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "log/Log.h"

namespace Module {
struct Range {
    enum class Token {
        PRE_WHITE = 0,
        BEGIN_RANGE_TOKEN,
        PRE_MIN_WHITE,
        MIN_TOKEN,
        POST_MIN_WHITE,
        DELIM_TOKEN,
        PRE_MAX_TOKEN,
        MAX_TOKEN,
        POST_MAX_TOKEN,
        END_RANGE_TOKEN,
        POST_WHITE,
        END_TOKEN,

        TOKEN_MAX
    };

    char bc;
    char ec;
    std::string minv, maxv;

    std::string &v;

    explicit Range(std::string &str) : bc(0), ec(0), v(str)
    {}

    bool RangeStatePreWhite(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::BEGIN_RANGE_TOKEN;
        }
        return true;
    }
    bool RangeStateBeginRangeToken(int &i, Token &status)
    {
        if (v[i] != '(' && v[i] != '[') {
            return false;
        } else {
            bc = v[i];
            status = Token::PRE_MIN_WHITE;
        }
        return true;
    }
    bool RangeStatePreMinWhite(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::MIN_TOKEN;
        }
        return true;
    }
    bool RangeStateMinToken(int &i, Token &status)
    {
        if (!isdigit(v[i])) {
            --i;
            status = Token::POST_MIN_WHITE;
        } else {
            minv += v[i];
        }
        return true;
    }
    bool RangeStatePostMinWhite(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::DELIM_TOKEN;
        }
        return true;
    }
    bool RangeStateDelimToken(int &i, Token &status)
    {
        if (v[i] != ',') {
            return false;
        } else {
            status = Token::PRE_MAX_TOKEN;
        }
        return true;
    }
    bool RangeStatePreMaxToken(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::MAX_TOKEN;
        }
        return true;
    }
    bool RangeStateMaxToken(int &i, Token &status)
    {
        if (!isdigit(v[i])) {
            --i;
            status = Token::POST_MAX_TOKEN;
        } else {
            maxv += v[i];
        }
        return true;
    }
    bool RangeStatePostMaxToken(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::END_RANGE_TOKEN;
        }
        return true;
    }
    bool RangeStateEndRangeToken(int &i, Token &status)
    {
        if (v[i] != ')' && v[i] != ']') {
            return false;
        } else {
            ec = v[i];
            status = Token::POST_WHITE;
        }
        return true;
    }
    bool RangeStatePostWhite(int &i, Token &status)
    {
        if (v[i] != ' ') {
            --i;
            status = Token::END_TOKEN;
        }
        return true;
    }
    bool RangeStateEndToken(int &i, Token &status)
    {
        if (v[i] != 0) {
            return false;
        }
        status = Token::TOKEN_MAX;
        return true;
    }

    bool GetRange(std::string &rv)
    {
        if (bc && ec && (!minv.empty() || !maxv.empty())) {
            rv = bc;
            rv += minv;
            rv += ",";
            rv += maxv;
            std::string tmp(1, ec);
            rv += tmp;

            return true;
        }

        return false;
    }
};

bool DoParseRange(Range &r)
{
    using RangeStatus = bool (Range::*)(int &, Range::Token &);
    RangeStatus rs[static_cast<uint64_t>(Range::Token::TOKEN_MAX)];

    rs[static_cast<uint64_t>(Range::Token::PRE_WHITE)] = &Range::RangeStatePreWhite;
    rs[static_cast<uint64_t>(Range::Token::BEGIN_RANGE_TOKEN)] = &Range::RangeStateBeginRangeToken;
    rs[static_cast<uint64_t>(Range::Token::PRE_MIN_WHITE)] = &Range::RangeStatePreMinWhite;
    rs[static_cast<uint64_t>(Range::Token::MIN_TOKEN)] = &Range::RangeStateMinToken;
    rs[static_cast<uint64_t>(Range::Token::POST_MIN_WHITE)] = &Range::RangeStatePostMinWhite;
    rs[static_cast<uint64_t>(Range::Token::DELIM_TOKEN)] = &Range::RangeStateDelimToken;
    rs[static_cast<uint64_t>(Range::Token::PRE_MAX_TOKEN)] = &Range::RangeStatePreMaxToken;
    rs[static_cast<uint64_t>(Range::Token::MAX_TOKEN)] = &Range::RangeStateMaxToken;
    rs[static_cast<uint64_t>(Range::Token::POST_MAX_TOKEN)] = &Range::RangeStatePostMaxToken;
    rs[static_cast<uint64_t>(Range::Token::END_RANGE_TOKEN)] = &Range::RangeStateEndRangeToken;
    rs[static_cast<uint64_t>(Range::Token::POST_WHITE)] = &Range::RangeStatePostWhite;
    rs[static_cast<uint64_t>(Range::Token::END_TOKEN)] = &Range::RangeStateEndToken;

    Range::Token status = Range::Token::PRE_WHITE;
    int s = static_cast<int>(r.v.size());
    for (int p = 0; p < s; p++) {
        if (!(r.*rs[static_cast<uint64_t>(status)])(p, status)) {
            return false;
        };

        if (static_cast<uint64_t>(status) >= static_cast<uint64_t>(Range::Token::TOKEN_MAX)) {
            break;
        }
    }

    return true;
}

bool IsRange(std::string &v)
{
    Range r(v);

    return DoParseRange(r) && r.GetRange(v);
}

bool GetRange(std::string &v, std::string &f, std::string &t)
{
    Range r(v);
    if (DoParseRange(r) && r.GetRange(v)) {
        f = r.minv;
        t = r.maxv;
        return true;
    } else {
        return false;
    }
}
}
