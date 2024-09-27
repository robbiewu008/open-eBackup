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
#ifndef HCS_HTTP_STATSU_H
#define HCS_HTTP_STATSU_H

#include <set>
#include "HcsMacros.h"

namespace HcsPlugin {
// 运营面对外http状态码定义
enum class HcsExternalStatusCode {
    // 请求已成功，请求所希望的响应头或数据体将随此响应返回
    OK = 200,
    // 当使用POST或PUT成功创建一个新的资源后，服务器应该返回201 Created同时在header的Location字段给出创建好的这个资源的URI
    CREATEED = 201,
    // 请求已经被实现，而且有一个新的资源已经依据请求的需要而建立，且其URI已经随Location头信息返回
    ACCEPTED = 202,
    // 服务器成功处理了请求，但不需要返回任何实体内容
    NO_CONTENT = 204,
    // 服务器已经成功处理了部分GET请求
    PARITAL_CONTENT = 206,
    // 请求的资源现在临时从不同的URI响应请求。由于这样的重定向是临时的，客户端应当继续向原有地址发送以后的请求
    FOUNED = 302,
    // 对应当前请求的响应可以在另一个URI上被找到，而且客户端应当采用GET的方式访问那个资源
    SEE_OTHER = 303,
    // 未按预期修改文档。客户端有缓冲的文档并发出了一个条件性的请求（一般是提供If-Modified-Since头表示客户只想比指定日期更新
    // 的文档）。服务器告诉客户，原来缓冲的文档还可以继续使用
    NOT_MODIFIED = 304,
    // 1.语义有误，当前请求无法被服务器理解，客户端不应该重复提交这个请求,2.请求参数有误
    BAD_REQUEST = 400,
    // 仅在HTTPS（BASIC认证、DIGET认证）认证的场景使用，若为其他认证机制，认证失败后则返回403状态码。
    // 响应需包含WWW-Authenticate首部用以质询用户信息
    UNAUTHORIZED = 401,
    // 如果客户端使用HEAD以外的方法请求，403 Forbidden须同时在body中返回禁止访问的原因。
    // 如果原因不能够公开，则应该使用404 NotFound
    FORIBDDEN = 403,
    // 请求失败，请求所希望得到的资源未被在服务器上发现。没有信息能够告诉用户这个状况是暂时的还是永久的。
    // 假如服务器知道情况，应当使用410状态码来告知旧资源因为某些内部的配置机制问题，已经永久的不可用，而且没有任何可以跳转的地址。
    // 404这个状态码被广泛应用于当服务器不想揭示为何请求被拒绝或者没有其他适合的响应可用的情况下
    NOT_FOUND = 404,
    // 请求行中指定的请求方法不能被用于请求相应的资源。该响应必须返回一个Allow头信息用以表示出当前资源能够接收的请求方法的列表。
    // 鉴于PUT、DELETE方法会对服务器上的资源进行写操作，因而绝大部分的网页服务器都不支持或者在默认配置下不允许上述请求方法，
    // 对于此类请求均会返回405错误
    METHOD_NOT_ALLOWED = 405,
    // 通常发生在PUT请求时，如果要更新的资源已经被其他人更新过了，再更新就可能产生冲突
    CONFICT = 409,
    // URL长度应小于2083字符
    REQUEST_URI_TOO_LONG = 414,
    // 请求频率超出流控阈值的场景
    TOO_MANY_REQUEST = 429,
    // 服务器遇到了一个未曾预料的状况，导致了它无法完成对请求的处理
    INTERNAL_SERVER_ERROR = 500,
    // 作为网关或者代理工作的服务器尝试执行请求时，从上游服务器接收到无效的响应
    BAD_GATEWAY = 502,
    // 由于临时的服务器维护或者过载，服务器当前无法处理请求
    SERVICE_UNAVAILABLE = 503,
};
}

#endif