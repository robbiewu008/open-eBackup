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
#ifndef _HTTP_STATUS_H_
#define _HTTP_STATUS_H_

/**
 * Server status codes; see RFC 2068.
 * Status code (100) indicating the client can continue.
 */
const int SC_CONTINUE = 100;

/**
 * Status code (101) indicating the server is switching protocols according
 * to Upgrade header.
 */
const int SC_SWITCHING_PROTOCOLS = 101;

/**
 * Status code (200) indicating the request succeeded normally.
 */
const int SC_OK = 200;

/**
 * Status code (201) indicating the request succeeded and created a new
 * resource on the server.
 */
const int SC_CREATED = 201;

/**
 * Status code (202) indicating that a request was accepted for processing,
 * but was not completed.
 */
const int SC_ACCEPTED = 202;

/**
 * Status code (203) indicating that the meta information presented by the
 * client did not originate from the server.
 */
const int SC_NON_AUTHORITATIVE_INFORMATION = 203;

/**
 * Status code (204) indicating that the request succeeded but that there
 * was no new information to return.
 */
const int SC_NO_CONTENT = 204;

/**
 * Status code (205) indicating that the agent <em>SHOULD</em> reset the
 * document view which caused the request to be sent.
 */
const int SC_RESET_CONTENT = 205;

/**
 * Status code (206) indicating that the server has fulfilled the partial
 * GET request for the resource.
 */
const int SC_PARTIAL_CONTENT = 206;

/**
 * Status code (300) indicating that the requested resource corresponds to
 * any one of a set of representations, each with its own specific location.
 */
const int SC_MULTIPLE_CHOICES = 300;

/**
 * Status code (301) indicating that the resource has permanently moved to a
 * new location, and that future references should use a new URI with their
 * requests.
 */
const int SC_MOVED_PERMANENTLY = 301;

/**
 * Status code (302) indicating that the resource has temporarily moved to
 * another location, but that future references should still use the
 * original URI to access the resource. This definition is being retained
 * for backwards compatibility. SC_FOUND is now the preferred definition.
 */
const int SC_MOVED_TEMPORARILY = 302;

/**
 * Status code (302) indicating that the resource reside temporarily under a
 * different URI. Since the redirection might be altered on occasion, the
 * client should continue to use the Request-URI for future
 * requests.(HTTP/1.1) To represent the status code (302), it is recommended
 * to use this variable.
 */
const int SC_FOUND = 302;

/**
 * Status code (303) indicating that the response to the request can be
 * found under a different URI.
 */
const int SC_SEE_OTHER = 303;

/**
 * Status code (304) indicating that a conditional GET operation found that
 * the resource was available and not modified.
 */
const int SC_NOT_MODIFIED = 304;

/**
 * Status code (305) indicating that the requested resource <em>MUST</em> be
 * accessed through the proxy given by the <code><em>Location</em></code>
 * field.
 */
const int SC_USE_PROXY = 305;

/**
 * Status code (307) indicating that the requested resource resides
 * temporarily under a different URI. The temporary URI <em>SHOULD</em> be
 * given by the <code><em>Location</em></code> field in the response.
 */
const int SC_TEMPORARY_REDIRECT = 307;

/**
 * Status code (400) indicating the request sent by the client was
 * syntactically incorrect.
 */
const int SC_BAD_REQUEST = 400;

/**
 * Status code (401) indicating that the request requires HTTP
 * authentication.
 */
const int SC_UNAUTHORIZED = 401;

/**
 * Status code (402) reserved for future use.
 */
const int SC_PAYMENT_REQUIRED = 402;

/**
 * Status code (403) indicating the server understood the request but
 * refused to fulfill it.
 */
const int SC_FORBIDDEN = 403;

/**
 * Status code (404) indicating that the requested resource is not
 * available.
 */
const int SC_NOT_FOUND = 404;

/**
 * Status code (405) indicating that the method specified in the
 * <code><em>Request-Line</em></code> is not allowed for the resource
 * identified by the <code><em>Request-URI</em></code>.
 */
const int SC_METHOD_NOT_ALLOWED = 405;

/**
 * Status code (406) indicating that the resource identified by the request
 * is only capable of generating response entities which have content
 * characteristics not acceptable according to the accept headers sent in
 * the request.
 */
const int SC_NOT_ACCEPTABLE = 406;

/**
 * Status code (407) indicating that the client <em>MUST</em> first
 * authenticate itself with the proxy.
 */
const int SC_PROXY_AUTHENTICATION_REQUIRED = 407;

/**
 * Status code (408) indicating that the client did not produce a request
 * within the time that the server was prepared to wait.
 */
const int SC_REQUEST_TIMEOUT = 408;

/**
 * Status code (409) indicating that the request could not be completed due
 * to a conflict with the current state of the resource.
 */
const int SC_CONFLICT = 409;

/**
 * Status code (410) indicating that the resource is no longer available at
 * the server and no forwarding address is known. This condition
 * <em>SHOULD</em> be considered permanent.
 */
const int SC_GONE = 410;

/**
 * Status code (411) indicating that the request cannot be handled without a
 * defined <code><em>Content-Length</em></code>.
 */
const int SC_LENGTH_REQUIRED = 411;

/**
 * Status code (412) indicating that the precondition given in one or more
 * of the request-header fields evaluated to false when it was tested on the
 * server.
 */
const int SC_PRECONDITION_FAILED = 412;

/**
 * Status code (413) indicating that the server is refusing to process the
 * request because the request entity is larger than the server is willing
 * or able to process.
 */
const int SC_REQUEST_ENTITY_TOO_LARGE = 413;

/**
 * Status code (414) indicating that the server is refusing to service the
 * request because the <code><em>Request-URI</em></code> is longer than the
 * server is willing to interpret.
 */
const int SC_REQUEST_URI_TOO_LONG = 414;

/**
 * Status code (415) indicating that the server is refusing to service the
 * request because the entity of the request is in a format not supported by
 * the requested resource for the requested method.
 */
const int SC_UNSUPPORTED_MEDIA_TYPE = 415;

/**
 * Status code (416) indicating that the server cannot serve the requested
 * byte range.
 */
const int SC_REQUESTED_RANGE_NOT_SATISFIABLE = 416;

/**
 * Status code (417) indicating that the server could not meet the
 * expectation given in the Expect request header.
 */
const int SC_EXPECTATION_FAILED = 417;

/**
 * Status code (500) indicating an error inside the HTTP server which
 * prevented it from fulfilling the request.
 */
const int SC_INTERNAL_SERVER_ERROR = 500;

/**
 * Status code (501) indicating the HTTP server does not support the
 * functionality needed to fulfill the request.
 */
const int SC_NOT_IMPLEMENTED = 501;

/**
 * Status code (502) indicating that the HTTP server received an invalid
 * response from a server it consulted when acting as a proxy or gateway.
 */
const int SC_BAD_GATEWAY = 502;

/**
 * Status code (503) indicating that the HTTP server is temporarily
 * overloaded, and unable to handle the request.
 */
const int SC_SERVICE_UNAVAILABLE = 503;

/**
 * Status code (504) indicating that the server did not receive a timely
 * response from the upstream server while acting as a gateway or proxy.
 */
const int SC_GATEWAY_TIMEOUT = 504;

/**
 * Status code (505) indicating that the server does not support or refuses
 * to support the HTTP protocol version that was used in the request
 * message.
 */
const int SC_HTTP_VERSION_NOT_SUPPORTED = 505;

#endif  // _AGENT_MESSAGE_PROCESS_H_
