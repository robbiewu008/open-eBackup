#!/usr/bin/python3
# -*- coding: utf-8 -*-
import copy
import json
import re
from urllib.parse import quote

import requests

from ansible_collections.oceanprotect.ansible.plugins.module_utils.log.log import logger


def __convert_index(text):
    """将驼峰格式的字符串转换为_的蛇形格式

    Args:
        text          （str）:需要转换的str

    Returns:
        text          （str）：_连接的蛇形格式

    Raises:
        None.

    Examples:
        __convert_index("deviceManager") 处理后返回device_manager

    """
    lst = []
    for index, char in enumerate(text):
        if char.isupper() and index != 0:
            lst.append("_")
        lst.append(char)
    return "".join(lst).lower()


def __convert_format(text):
    """处理字符串格式，大写转小写，空格转下划线，驼峰转蛇形

    Args:
        text          （str）:需要转换的str

    Returns:
        处理后的字符串

    Raises:
        None.

    Examples:
        __convert_format("deviceManager") 处理后返回device_manager

    """
    result = str(re.sub("\s+", "_", __convert_index(text).lower()))
    return result


def convertParserResult(sourceDict, ruleDict):
    """The conversion value or key in the dictionary

    Args:
        sourceDict  (Dict): 需要转换的字典
        ruleDict    (Dict): 转换规则
                            format {
                                'key1': {'converted_key': key1a, 'converted_value': method1},
                                'key2': ...
                                .
                                .
                            }
                            converted_key   : 可选，将sourceDict中key为key1修改为key1a
                            converted_value : 可选，将sourceDict中key为key1对应的value以参数传给method1，并转换为method1的返回值

                            注：
                            1.转换方式为，先进行value的转换，再对key进行修改
                            2.如果sourceDict中不存在key:key1，key2其中一个或多个，不报错

    Returns:
        Dict

    Raises:
        None

    Examples:
        参考commonParser:Examples:6,7,8
    """
    # template = {key: {'converted_key': str 'converted_value': str}}
    for key in ruleDict:
        if key in sourceDict:
            # 回显为非component
            if 'converted_value' in ruleDict[key]:
                sourceDict[key + '_raw'] = sourceDict[key]
                sourceDict[key] = ruleDict[key]['converted_value'](sourceDict[key])
            if 'converted_key' in ruleDict[key]:
                sourceDict[ruleDict[key]['converted_key']] = sourceDict[key]
        else:
            # 回显为component
            popElement = []
            for ele in list(sourceDict.keys()):
                if key in sourceDict[ele]:
                    if 'converted_value' in ruleDict[key]:
                        sourceDict[ele][key + '_raw'] = sourceDict[ele][key]
                        sourceDict[ele][key] = ruleDict[key]['converted_value'](sourceDict[ele][key])

                    if 'converted_key' in ruleDict[key]:
                        sourceDict[ele][ruleDict[key]['converted_key']] = sourceDict[ele][key]
                else:
                    break
            for _key in popElement:
                sourceDict.pop(_key)

    return sourceDict


class RestApi(object):
    HttpErrorCode = {
        200: 'OK action with content returned.',
        201: 'OK Create with content returned',
        202: 'Accepted as an async process',
        204: 'OK no content returned',
        400: 'Bad Request, Badly formed URI, parameters, headers, or body content.',
        403: 'Forbidden, authentication or authorization failure.',
        404: "Not Found, Resource doesn't exist.",
        405: 'Method Not Allowed.',
        406: 'Not Acceptable, Accept headers are not satisfiable.',
        409: 'The request could not be completed due to a.',
        422: 'Unprocessable Entity, Semantically invalid content on a POST.',
        500: 'Internal Server Error.',
        503: 'Service Unavailable.'
    }

    def __init__(self, ip, username, password, scope):
        self.session = requests.Session()
        self.user_type = None
        self.language = None
        self.is_manage_one = False
        self.dynamic_code = None
        self.username = username
        self.password = password
        self.scope = scope
        self.ip = ip

    def login(self):
        """Connect device, if success, update Http header to set the session id
        """

        self.updateHeaders({'Content-type': 'application/json'})

        data = {'userName': self.username, 'password': self.password, 'scope': self.scope}
        if self.dynamic_code is not None:
            data['dynamicCode'] = self.dynamic_code
        if self.user_type is not None:
            data['userType'] = self.user_type
        if self.language is not None:
            data['language'] = self.language
        url = "https://%s:25081/v1/auth/token" % self.ip

        rsp = self.request('post', url, data=data, verify=False)
        if rsp.ok:
            _json = rsp.json()
            if 'data' in _json:
                data = _json['data']
                if 'iBaseToken' in data:
                    self.updateHeaders({'iBaseToken': data['iBaseToken']})
                    return _json['token']
            else:
                if 'token' in _json:
                    self.updateHeaders({'X-Auth-Token': _json['token']})
                    return _json['token']
        return self.__formatResponse(rsp)

    def updateHeaders(self, params):
        """Update restful request headers
        """

        self.session.headers.update(params)

    def __convertDictItemsToJson(self, params):
        """Convert dict type parameters in given Http parameters to json type
        """

        for key, value in list(params.items()):
            if key.lower() not in ['data', 'params']:
                continue

            if isinstance(value, dict) or isinstance(value, list):
                params[key] = json.dumps(value)
        return params

    def request(self, method, url, skip_convert=False, **kwargs):
        """Make a Http request according to given method

        Args:
            method               (str) : Http method, values: get/post/put/delete
            url:                 (str) : Url for the request
            kwargs:             (dict) : (Optional). Parameters see bellow

                Field                   Type     Optional       Description
                ==============================================================================
                params                  dict     True           Dictionary to be sent in the query string for the request.
                data                    dict     True           Dictionary to send in the body of the request.
                headers                 dict     True           Dictionary of HTTP Headers to send with the request.

        Returns:
            Http response   (Response) : A response object.
            Usage:
                r.ok            (bool) : Whether the request is ok. Return True/False
                r.status_code    (int) : Http response code. Int
                r.elapsed       (time) : Request used time. e.g. 0:00:01.662000
                r.json()        (json) : Response content as json format
                r.text           (str) : Response content as text string format

        Raises:
            None

        Examples:

        """

        kwargs['verify'] = False
        if not skip_convert:
            kwargs = self.__convertDictItemsToJson(kwargs)
        method = str(method).lower()

        def sendRequest():
            msg = 'Rest request. method: %s, url: %s, params: %s' % (method, url, kwargs.get("params"))
            logger.debug(str(msg))
            if method == 'get':
                r = self.session.get(url, **kwargs)
            elif method == 'post':
                r = self.session.post(url, **kwargs)
            elif method == 'put':
                r = self.session.put(url, **kwargs)
            elif method == 'delete':
                r = self.session.delete(url, **kwargs)
            else:
                raise Exception("RestBase not implement '%s' method" % method)

            self.__logResponse(r)
            return r

        try:
            result = sendRequest()
        except Exception:
            logger.error('sendrequest fail, retry...')
            self.login()
            result = sendRequest()

        return result

    def isResponseOK(self, status_code):
        return status_code == requests.codes.ok

    def handleHttpErrorCode(self, statusCode):
        """Return error message according to Http status code
        """

        statusCode = int(statusCode)
        if statusCode in self.HttpErrorCode:
            return self.HttpErrorCode[statusCode]
        else:
            return "Non-defined Error."

    def __logResponse(self, response):
        """Log out response message according to response status code
        """
        if not self.isResponseOK(response.status_code):
            msg = 'Rest response error. Status code: %s, elapsed: %s, message: %s' % \
                  (response.status_code, response.elapsed, self.handleHttpErrorCode(response.status_code))
            logger.error(str(msg))
        else:
            if response.request.method.lower() == 'get':
                if response.text.find('{') < 0:
                    msg = 'Rest response ok. Status code: %s, elapsed: %s, message: %s' % \
                          (response.status_code, response.elapsed,
                           'This interface is used to download file, no json response data')
                else:
                    msg = 'Rest response ok. Status code: %s, elapsed: %s' % \
                          (response.status_code, response.elapsed)
            else:
                if response.text.find('{') < 0:
                    msg = 'Rest response ok. Status code: %s, elapsed: %s, message: %s' % \
                          (response.status_code, response.elapsed,
                           'This interface is used to download file, no json response data')
                else:
                    msg = 'Rest response ok. Status code: %s, elapsed: %s, message: %s' % \
                          (response.status_code, response.elapsed, response.text)

            # 部分告警信息中有中文句号，要特殊处理一下
            # msg = unicodedata.normalize('NFKC', msg)
            logger.debug(str(msg))

    def __formatResponse(self, response, parser=None):
        """Format response to wrapper standard output

        Args:
            response        (Response) : Restful response object
            parser      (commonParser) : User define parser, more info please see Wrapper\Tool\AdminCli\Parser.py

        Returns:
            Formatted returns. Type is List<dict>, the same to AdminCli
            Such as: [{'partial': 0, 'parser': xxx, 'rc': xx, 'stderr': xxx, 'stdout': xxx}, {more}]
                parser : Parsed info by user define result parser
                rc     : Return code, 0 when request returns success, otherwise set by Http request status code
                stderr : Return error section in restful api response content, otherwise set by Http request status code and error message
                stdout : Return data section in restful api response content, otherwise set by response text

            more information please see Examples
        """

        status_code = response.status_code
        # 1xx: Informational
        # 2xx: Success
        # 3xx: Redirection
        # 4xx: Client Error
        # 5xx: Server Error
        r = {}
        content = {}
        if self.isResponseOK(status_code):
            try:
                content = self.encodeJsonToUtf8(response.json())
            except Exception:
                logger.debug('This interface is used to download file, no json response data')

            # 目前REST返回可能存在无data字段情况，需人为处理一下
            if isinstance(content, dict) and 'data' in content:
                data = content['data']
            else:
                data = content

            error = content.get("error") if isinstance(content, dict) else None
            _code = error.get("code") if error else status_code
            r = self.__setStandardResult(_code, data, error, data, response)

            if data:
                table = ""
                if isinstance(data, dict) and data.get('items'):
                    _data = data.get('items')
                else:
                    _data = data
                if parser is not None:
                    try:
                        r['parser'] = parser(_data)
                    except Exception as e:
                        logger.warn(e)
        else:
            r = self.__setStandardResult(status_code,
                                         response.content,
                                         {'code': status_code, 'description': self.handleHttpErrorCode(status_code)},
                                         response.content,
                                         response)
        return r

    def __setStandardResult(self, returnCode, stdout, stderr, parsedResult, response):
        """Convert result to the same pattern to adminCli
        """
        return {'partial': 1, 'rc': returnCode, 'stdout': stdout, 'stderr': stderr, 'parser': parsedResult,
                'rsp': response}

    def encodeJsonToUtf8(self, unicodeJson):
        """Convert unicode json encoding to utf-8
        """

        if isinstance(unicodeJson, dict):
            return {self.encodeJsonToUtf8(key): self.encodeJsonToUtf8(value) for key, value in
                    list(unicodeJson.items())}
        elif isinstance(unicodeJson, list):
            return [self.encodeJsonToUtf8(item) for item in unicodeJson]
        elif isinstance(unicodeJson, str):
            # deal with special character in the response
            return unicodeJson.encode('utf-8').decode('utf-8') \
                .replace('&#40;', '(') \
                .replace('&#41;', ')') \
                .replace('&gt;', '>') \
                .replace('&lt;', '<') \
                .replace('&amp;', '&') \
                .replace('\r', '') \
                .replace('\n', '')
        else:
            return unicodeJson

    def __generate_url_path(self, url, key, value):
        """url拼接参数

        Args:
            url              (str) : 接口url
            key              (str) : 参数名
            value            (str|dict) : 对应参数的值

        Returns:
            url: 拼接后的url

        Examples:
            url = self.__generate_url_path(url, "type", "BACKUP")

        """
        if isinstance(value, dict):
            value = quote(json.dumps(value))
        if url.find('?') > 0:
            url += '&%s=%s' % (key, str(value))
        else:
            url += '?%s=%s' % (key, str(value))
        return url

    def restRequest(self, apiTemplate, paramsInput):
        params = {}
        for _iterm in paramsInput:
            params[_iterm] = paramsInput[_iterm]
        # A8000版本存在部分参数值需要传空字符串场景，处理参数时，不剔除值为空字符串的参数
        # params = self.deleteNoneParams(params)
        # 获取接口访问方法
        requestMethod = ['post', 'get', 'put', 'delete', 'POST', 'GET', 'PUT', 'DELETE']
        # 带过滤条件的方法只能通过URL方式访问，需要将参数拼接到URI中
        api = copy.copy(apiTemplate)

        # 如果resource包含https://，则传入的是完整的url，针对api/v2，v1和deviceManager/rest类型进行处理
        # 替换掉https://${ip}:${port}/api/v2这一段，兼容老的resource格式。例如 'resource': 'file/buildrun/example'
        if api.get("resource", "").startswith("https://"):
            if "/api/v2" in api["resource"]:
                self.api_version = "v2"
                api["resource"] = re.sub("https://\$\{ip\}:(\$\{port\}|\d+)/api/v2/", "", api["resource"])
            elif "/v1" in api["resource"]:
                self.api_version = "v1"
                api["resource"] = re.sub("https://\$\{ip\}:(\$\{port\}|\d+)/v1/", "", api["resource"])
            elif "/v2/" in api["resource"] and "/api/" not in api["resource"]:
                self.api_version = "v2_pm"
                api["resource"] = re.sub("https://\$\{ip\}:(\$\{port\}|\d+)/v2/", "", api["resource"])

        if 'filter' in params and 'range' in params:
            if "filter=" in params['filter'] and "range=" in params['range']:
                api['resource'] += '?' + str(params['filter']) + '&' + str(params['range'])
                params.pop('filter')
                params.pop('range')
            else:
                api['resource'] += '?filter=' + str(params['filter']) + '&range=' + str(params['range'])
                params.pop('filter')
                params.pop('range')
        elif 'filter' in params and 'range' not in params:
            if "filter=" in params['filter']:
                api['resource'] += '?' + str(params['filter'])
                params.pop('filter')
            else:
                api['resource'] += '?filter=' + str(params['filter'])
                params.pop('filter')
        elif 'range' in params and 'filter' not in params:
            if "range=" in params['range']:
                api['resource'] += '?' + str(params['range'])
                params.pop('range')
            else:
                api['resource'] += '?range=' + str(params['range'])
                params.pop('range')

        # 调用接口访问方法
        if api['method'] not in requestMethod:
            raise Exception("Request method : '%s' is error. Please check params file" % api['method'])
        logger.debug('Rest request>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>')
        logger.debug('Rest params:%s' % params)

        # 'api/v2/pms/performance_chart/${chart_id}'这个格式的url，需要拼接成'api/v2/pms/performance_chart/2'
        for _key in list(params.keys()):
            math_str = '\${,}\{%s\}' % _key
            if re.search(math_str, api['resource']):
                api['resource'] = re.sub(math_str, params[_key], api['resource'])
                params.pop(_key)
        # 如果参数类型是path或者query，则将其拼接到url中
        _file_params = {}
        _need_pop = []

        for _key in _need_pop:
            params.pop(_key)
        if 'header_update_info' in params:
            self.updateHeaders(params['header_update_info'])
            params.pop('header_update_info')
        tmp_updated = {}
        if 'tmp_update_header' in params:
            self.updateHeaders(params['tmp_update_header'])
            tmp_updated = params.pop('tmp_update_header')
        if 'body' in params:
            params = params['body']
        # 调用post方法
        if 'post' == api['method'].lower():
            rsp = self.post(api['resource'], data=params, files=_file_params, )

        # 调用put方法
        if 'put' == api['method'].lower():
            rsp = self.put(api['resource'], data=params, files=_file_params)

        # 调用get方法
        if 'get' == api['method'].lower():
            for k, v in api["params"].items():
                if v["location"] == "query" and params.get(k):
                    api['resource'] = self.__generate_url_path(api['resource'], k, params[k])
                    params.pop(k)
                elif params.get(k) == 0 and k == "page_no":
                    api['resource'] = self.__generate_url_path(api['resource'], k, params[k])
                    params.pop(k)

            rsp = self.get(api['resource'], data=params)

        # 调用delete方法
        if 'delete' == api['method'].lower():
            for k, v in api["params"].items():
                if v["location"] == "query" and params.get(k):
                    api['resource'] = self.__generate_url_path(api['resource'], k, params[k])
                    params.pop(k)
            rsp = self.delete(api['resource'], data=params)
        for each in tmp_updated:
            self.session.headers.pop(each)
        if rsp.status_code == 200:
            res = json.loads(rsp.content)
            logger.debug(" %s url:%s success content is %s" % (api['method'], api['resource'], res))
            return rsp.status_code, res
        else:
            logger.error(' %s url:%s fail,code:%s ' % (api['method'], api['resource'], rsp.status_code))
            return rsp.status_code, rsp

    def post(self, suffix, **kwargs):
        return self.request('post', suffix, **kwargs)

    def put(self, suffix, **kwargs):
        return self.request('put', suffix, **kwargs)

    def delete(self, suffix, **kwargs):

        return self.request('delete', suffix, **kwargs)

    def get(self, suffix, **kwargs):
        """Get method
        """
        return self.request('get', suffix, **kwargs)
