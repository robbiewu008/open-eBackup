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
import { Injectable } from '@angular/core';
import { AbstractControl, ValidatorFn } from '@angular/forms';
import {
  isString,
  size,
  toString,
  isArray,
  includes,
  reject,
  find,
  isEmpty
} from 'lodash';
import { CommonConsts, DataMap } from '../consts';
import { I18NService } from './i18n.service';
import { WhiteboxService } from './whitebox.service';

const isEmptyInputValue = (value: any): boolean =>
  value == null || value.length === 0;

@Injectable({
  providedIn: 'root'
})
export class BaseUtilService {
  // 约定密码长度，该长度为最小长度
  passLenVal = 8;
  // 密码复杂度，
  // 2-必须包含特殊字符，并且至少包含大写字母、小写字母以及数字中任意两者的组合
  // 4-必须包含特殊字符、大写字母、小写字母和数字
  passComplexVal = 4;
  // 密码最大长度，默认为32，特殊地方需要自己传入
  maxLenVal = 64;

  constructor(private i18n: I18NService, private whitebox: WhiteboxService) {}

  nameValidLabel = this.i18n.get('common_valid_name_label');
  invalidInputLabel = this.i18n.get('common_invalid_input_label');
  rangValidLabel = this.i18n.get('common_valid_rang_label');
  maxSizeLabel = this.i18n.get('common_valid_maxsize_label');
  minSizeLabel = this.i18n.get('common_valid_minsize_label');
  maxLengthLabel = this.i18n.get('common_valid_maxlength_label');
  minLengthLabel = this.i18n.get('common_valid_minlength_label');
  requiredLabel = this.i18n.get('common_required_label');
  inValidTextabel = this.i18n.get('common_invalid_inputtext_label');
  sameHistoryPwdLabel = this.i18n.get('common_samehistorypwd_label');
  diffPwdLabel = this.i18n.get('common_diffpwd_label');
  integerLabel = this.i18n.get('common_valid_integer_label');
  cannotDecimalLabel = this.i18n.get('common_cannot_decimal_label');
  repeatNameLabel = this.i18n.get('common_duplicate_name_label');
  invalidPathLabel = this.i18n.get('common_path_error_label');
  invalidScriptLabel = this.i18n.get('common_script_error_label');
  invalidNameBeginLabel = this.i18n.get('common_valid_name_begin_label');
  invalidNameCombinationLabel = this.i18n.get(
    'common_valid_name_combination_label'
  );
  invalidLengthRangLabel = this.i18n.get('common_valid_length_rang_label', [
    1,
    64
  ]);
  invalidFilePathLabel = this.i18n.get('common_valid_file_path_label');

  // 名称校验提示信息
  nameErrorTip = {
    invalidName: this.nameValidLabel,
    required: this.requiredLabel,
    invalidRepeat: this.repeatNameLabel,
    invalidNameBegin: this.invalidNameBeginLabel,
    invalidNameCombination: this.invalidNameCombinationLabel,
    invalidNameLength: this.invalidLengthRangLabel
  };

  // 密码校验提示信息
  pwdErrorTip = {
    required: this.requiredLabel,
    invalidPwd: this.inValidTextabel,
    sameHistoryPwd: this.sameHistoryPwdLabel,
    diffPwd: this.diffPwdLabel
  };

  // 非空校验提示信息
  requiredErrorTip = {
    required: this.requiredLabel
  };

  // 范围校验提示信息
  rangeErrorTip = {
    required: this.requiredLabel,
    invalidInput: this.invalidInputLabel,
    invalidRang: this.rangValidLabel,
    invalidInteger: this.integerLabel
  };

  // 输入值大小校验提示信息
  sizeErrorTip = {
    invalidMaxSize: this.maxSizeLabel,
    invalidMinSize: this.minSizeLabel
  };

  // 输入值长度校验提示信息
  lengthErrorTip = {
    invalidMaxLength: this.maxLengthLabel,
    invalidMinLength: this.minLengthLabel
  };

  // 输入值整数校验提示信息
  integerErrorTip = {
    invalidInteger: this.integerLabel
  };

  // 必须输入整数校验提示信息
  cannotDecimalErrorTip = {
    invalidInteger: this.cannotDecimalLabel
  };

  // 长度与规则校验提示信息
  emailErrorTip = {
    required: this.requiredLabel,
    invalidName: this.invalidInputLabel,
    invalidMaxLength: this.maxLengthLabel
  };

  testEmailErrorTip = {
    invalidName: this.invalidInputLabel,
    invalidMaxLength: this.maxLengthLabel
  };

  // IP校验提示信息
  ipErrorTip = {
    required: this.requiredLabel,
    invalidName: this.invalidInputLabel
  };

  // VMwareName
  vMwareNameErrorTip = {
    required: this.requiredLabel,
    invalidName: this.invalidInputLabel,
    invalidMaxLength: this.maxLengthLabel,
    invalidMinLength: this.minLengthLabel
  };

  // .bat和.sh脚本校验提示信息
  scriptNameErrorTip = {
    required: this.requiredLabel,
    invalidName: this.invalidScriptLabel,
    invalidPath: this.invalidScriptLabel
  };

  filePathErrorTip = {
    required: this.requiredLabel,
    invalidFilePath: this.invalidFilePathLabel
  };

  // 公共校验规则
  VALID = {
    /**
     * 名称校验
     *
     * @param   {[type]}  nameRe:       自定义名称正则表达式，可选参数
     * @param   {[type]}  isRequire:    是否必填项，可选参数
     * @param   {[type]}  errrorTipKey: 自定义错误提示信息，可选参数
     *
     */
    name: (
      nameRe?: RegExp | string,
      isRequire?: boolean | true,
      errrorTipKey?: string
    ): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          if (isRequire === false) {
            return null;
          }
          return { required: { value: control.value } };
        }

        if (!nameRe) {
          const value = control.value;
          // 1、只能以字母、中文或_开头。
          const reg1 = CommonConsts.REGEX.nameBegin;
          if (!reg1.test(value)) {
            return { invalidNameBegin: { value: control.value } };
          }

          // 2、由字母、数字、中文字符、“-”和“_”组成。
          const reg2 = CommonConsts.REGEX.nameCombination;
          if (!reg2.test(value)) {
            return { invalidNameCombination: { value: control.value } };
          }

          // 3、长度范围是1到64位。
          const reg3 = /^.{1,64}$/;
          if (!reg3.test(value)) {
            return { invalidNameLength: { value: control.value } };
          }

          return null;
        }

        if (isString(nameRe)) {
          nameRe = new RegExp('^(?:' + nameRe + ')$');
        }

        const invalidErrorObj = {};
        if (errrorTipKey) {
          invalidErrorObj[errrorTipKey] = { value: control.value };
        }

        if (toString(nameRe) === toString(CommonConsts.REGEX.nameBegin)) {
          invalidErrorObj['invalidNameBegin'] = { value: control.value };
        } else if (
          toString(nameRe) === toString(CommonConsts.REGEX.nameCombination)
        ) {
          invalidErrorObj['invalidNameCombination'] = { value: control.value };
        } else if (toString(nameRe) === toString(/^.{1,64}$/)) {
          invalidErrorObj['invalidNameLength'] = { value: control.value };
        } else {
          invalidErrorObj['invalidName'] = { value: control.value };
        }

        const forbidden = nameRe.test(control.value);
        return forbidden ? null : invalidErrorObj;
      };
    },
    /**
     * 检查密码复杂度, 根据安全策略的设置来检查当前密码是否符合密码复杂度要求
     *
     * @param   {[type]}  passLenVal       约定密码长度，该长度为最小长度
     * @param   {[type]}  passComplexVal  密码复杂度
     *                                    2-必须包含特殊字符，并且至少包含大写字母、小写字母以及数字中任意两者的组合
     *                                    4-必须包含特殊字符、大写字母、小写字母和数字
     * @param   {[type]}  maxLenVal       密码最大长度，默认为32，特殊地方需要自己传入
     * @param   {[type]}  repeatCount     口令中不能包含连续的相同字符个数，默认为2，如：111是不允许的
     * @param   {[type]}  pwdRe        自定义正则校验规则，可选参数
     *
     * @return  {ValidatorFn}          [return description]
     */
    password: (
      passLenVal = 8,
      passComplexVal = 4,
      maxLenVal = 64,
      repeatCount = 2,
      pwdRe?: RegExp,
      specialWordReg?: RegExp
    ): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return { required: { value: control.value } };
        }
        const pwd = control.value;
        passLenVal = passLenVal || 8;
        passComplexVal = passComplexVal || 2;
        maxLenVal = maxLenVal || 32;
        if (!pwd) {
          return { invalidPwd: { value: control.value } };
        }

        if (pwd.length < passLenVal || pwd.length > maxLenVal) {
          return { invalidPwd: { value: control.value } };
        }

        const regUpWord = /[A-Z]+/;
        const regDownWord = /[a-z]+/;
        const regNumber = /[0-9]+/;
        const regSpecialWord = !!specialWordReg
          ? specialWordReg
          : /[`|~\!@#\$%\^&\*\(\)\-_=+\\[\{\}\]\;:\'\"\,\<\.\>\/\?\u0020]+/;

        let weight = 0;
        let isValid = false;

        if (regNumber.test(pwd)) {
          weight += 1;
        }

        if (regDownWord.test(pwd)) {
          weight += 1;
        }

        if (regUpWord.test(pwd)) {
          weight += 1;
        }

        if (regSpecialWord.test(pwd)) {
          weight += 8;
        }

        if (repeatCount) {
          isValid = new RegExp('(.)\\1{' + repeatCount + '}').test(pwd);
          if (isValid) {
            return { invalidPwd: { value: control.value } };
          }
        }

        if (+passComplexVal === 2) {
          return weight >= 10 ? null : { invalidPwd: { value: control.value } };
        }

        if (+passComplexVal === 4) {
          return weight === 11
            ? null
            : { invalidPwd: { value: control.value } };
        }

        return { invalidName: { value: control.value } };
      };
    },
    /**
     * 数字区间范围校验，支持整数，浮点数
     *
     * @param   {any}          minValue  最小值
     * @param   {any}          maxValue  最大值
     *
     */
    rangeValue: (minValue: any, maxValue: any): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value && control.value !== 0) {
          return null;
        }

        if (!/^-?(?:\d+|\d{1,3}(?:,\d{3})+)?(?:\.\d+)?$/.test(control.value)) {
          return { invalidInput: { value: control.value } };
        }

        return parseFloat(control.value) >= parseFloat(minValue) &&
          parseFloat(control.value) <= parseFloat(maxValue)
          ? null
          : { invalidRang: { value: control.value } };
      };
    },
    /**
     * 最大值校验,支持整数、浮点数
     *
     * @param   {any}          maxValue  最大值
     *
     * @return  {ValidatorFn}            [return description]
     */
    maxSize: (maxValue: any): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        return parseFloat(control.value) <= parseFloat(maxValue)
          ? null
          : { invalidMaxSize: { value: control.value } };
      };
    },
    /**
     * 最小值校验,支持整数、浮点数
     *
     * @param   {any}          minValue  最小值
     *
     * @return  {ValidatorFn}            [return description]
     */
    minSize: (minValue: any): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        return parseFloat(control.value) <= parseFloat(minValue)
          ? { invalidMinSize: { value: control.value } }
          : null;
      };
    },
    /**
     * 字符最大长度校验
     *
     * @param   {any}          maxLength  最大值
     *
     * @return  {ValidatorFn}             [return description]
     */
    maxLength: (maxLength: any): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }
        return (control.value && control.value.length) <=
          parseInt(maxLength, 10)
          ? null
          : { invalidMaxLength: { value: control.value } };
      };
    },
    /**
     * 字符最小长度校验
     *
     * @param   {any}          minLength  最小值
     *
     * @return  {ValidatorFn}             [return description]
     */
    minLength: (minLength: any): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        return (control.value && control.value.length) < parseInt(minLength, 10)
          ? { invalidMinLength: { value: control.value } }
          : null;
      };
    },
    /**
     * 内容为空校验
     *
     * @return  {ValidatorFn}  [return description]
     */
    required: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (isArray(control.value)) {
          return size(control.value) === 0
            ? { required: { value: control.value } }
            : null;
        }
        return !control.value && control.value !== 0
          ? { required: { value: control.value } }
          : null;
      };
    },
    /**
     * 有效整数校验
     *
     * @return  {ValidatorFn}         [return description]
     */
    integer: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }
        return !/^[1-9]\d*$/.test(control.value) &&
          !/^-[1-9]{1}\d*$/.test(control.value) &&
          '0' !== control.value
          ? { invalidInteger: { value: control.value } }
          : null;
      };
    },
    /**
     * IP两种格式校验
     *
     * @param   {any}          ipv4  正则表达式1
     * @param   {any}          ipv6  正则表达式2
     *
     */
    ip: (ipv4Re?: RegExp, ipv6Re?: RegExp): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }
        if (!ipv4Re) {
          ipv4Re = CommonConsts.REGEX.ipv4;
        }
        if (!ipv6Re) {
          ipv6Re = CommonConsts.REGEX.ipv6;
        }

        if (ipv4Re.test(control.value) || ipv6Re.test(control.value)) {
          return null;
        }
        return { invalidName: { value: control.value } };
      };
    },

    /**
     * 路径校验
     * @param {RegExp}           pathRe 路径正则表达式
     */
    path: (
      osType: string,
      linuxPathRe?: RegExp,
      windowsPath?: RegExp
    ): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (control.value) {
          if (control.value[0] === '+') {
            return null;
          }
        } else {
          return null;
        }
        if (!linuxPathRe) {
          linuxPathRe = CommonConsts.REGEX.linuxPath;
        }
        if (!windowsPath) {
          windowsPath = CommonConsts.REGEX.windowsPath;
        }
        if (
          (linuxPathRe.test(control.value) && osType !== 'windows') ||
          (windowsPath.test(control.value) && osType === 'windows')
        ) {
          return null;
        }
        return { invalidPath: { value: control.value } };
      };
    },

    /**
     * 对ipv4地址进行复杂校验
     * 1.不能以127开头
     * 2.不能够输入0.0.0.0
     * 3.最后一位不能够为0或者255
     */
    ipv4: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const reg = /^(25[0-5]|2[0-4][0-9]|[1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])$/;

        // 不能为0.0.0.0、最后一位不能够为0
        if (reg.test(control.value)) {
          // 不能够以127开头和255结尾
          const firstIp = control.value.split('.')[0];
          const endIp = control.value.split('.')[3];
          if (firstIp !== '127' && endIp !== '255') {
            return null;
          }
        }

        return {
          invalidName: {
            value: control.value
          }
        };
      };
    },

    /**
     * 验证IPV4合法性，取值范围[1-223].[1-255].[1-255].[1-255]
     */
    _ipv4: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const isValid = BaseUtilService.validateIpv4(control.value);
        return isValid
          ? null
          : {
              invalidName: {
                value: control.value
              }
            };
      };
    },

    /**
     * 验证IPV6合法性
     *  fe80:0000:0000:0000:0204:61ff:fe9d:f156 // full form of IPv6
     *  fe80:0:0:0:204:61ff:fe9d:f156 // drop leading zeroes
     *  fe80::204:61ff:fe9d:f156 // collapse multiple zeroes to :: in the IPv6 address
     *  fe80:0000:0000:0000:0204:61ff:254.157.241.86 // IPv4 dotted quad at the end
     *  fe80:0:0:0:0204:61ff:254.157.241.86 // drop leading zeroes, IPv4 dotted quad at the end
     *  fe80::204:61ff:254.157.241.86 // dotted quad at the end, multiple zeroes collapsed
     *
     *  In addition, the regular expression matches these IPv6 forms:
     *
     *  ::1 // localhost
     *  fe80:: // link-local prefix
     *  2001:: // global unicast prefix
     */
    _ipv6: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const isValid = BaseUtilService.validateIpv6(control.value);
        return isValid
          ? null
          : {
              invalidName: {
                value: control.value
              }
            };
      };
    },

    _ip: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const isValid =
          BaseUtilService.validateIpv4(control.value) ||
          BaseUtilService.validateIpv6(control.value);

        return isValid
          ? null
          : {
              invalidName: {
                value: control.value
              }
            };
      };
    },

    /**
     * 同时校验ipv4和ipv6地址合法性，支持多播地址
     *
     */
    _ipWithMulticast: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const isValid =
          BaseUtilService.validateIpv4(control.value) ||
          BaseUtilService.validateIpv6WithMulticast(control.value);

        return isValid
          ? null
          : {
              invalidName: {
                value: control.value
              }
            };
      };
    },

    /**
     * 验证文件路径合法性
     *
     */
    filePath: (): ValidatorFn => {
      return (control: AbstractControl): { [key: string]: any } | null => {
        if (!control.value) {
          return null;
        }

        const FILE_PATH_REGEXP = {
          include: /^(?![\s\.])[^':\?\*\u0022<>\|]*[^'\s:\?\*\u0022<>\|]$/,
          exclude: /(\\\.)|(\/\.)/
        };

        const isValid =
          FILE_PATH_REGEXP.include.test(control.value) &&
          !FILE_PATH_REGEXP.exclude.test(control.value);
        return isValid
          ? null
          : {
              invalidFilePath: {
                value: control.value
              }
            };
      };
    }
  };

  /**
   * 同时验证IPV4 IPV6合法性。支持多播地址
   */
  static ipWithMulticast(value) {
    return (
      BaseUtilService.validateIpv4(value) ||
      BaseUtilService.validateIpv6WithMulticast(value)
    );
  }

  static validateIpv4(value) {
    // IPV4地址(ipv4)正则匹配
    const IPV4_REGEXP = {
      // 注意：(22[0-3]|2[01][0-9]|[01]?[0-9][0-9]?)(\.) 校验(1-223)
      include: new RegExp(
        '^(22[0-3]|2[01][0-9]|1\\d{2}|[1-9]\\d|[1-9])(\\.)' + // 验证(1-223)
        '((1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)(\\.)){2}' + // 验证中间两位
          '(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$'
      ), // 验证末尾一位
      loopBackAddr: /^127/, // 环回地址
      fullZeroIp: /^0.0.0.0$/ // 全0 地址
    };

    if (isEmptyInputValue(value)) {
      return null; // 在可选控制器下不验证
    }

    const isValid =
      IPV4_REGEXP.include.test(value) &&
      !IPV4_REGEXP.fullZeroIp.test(value) &&
      !IPV4_REGEXP.loopBackAddr.test(value);
    return isValid;
  }

  static validateIpv6(value) {
    // IPV6地址(ipv6)正则匹配
    const IPV6_REGEXP = {
      include: /^((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?$/,
      loopBackAddr: /^(((0{1,4}:){1,6}|:):0{0,3}1$)|(0{1,4}:){7}0{0,3}1$/, // 环回地址
      fullZeroIp: /^(((0{1,4}:){1,6}|:):0{0,4}$)|((0{1,4}:){7}0{1,4})|(((0{1,4}:){1,6}|:):0.0.0.0)$/, // 全0 地址
      multicast: /^([Ff][Ff]00::|[Ff][Ff]00:0{1,4}|[Ff][Ff]02((:0{1,4}){0,3}::(0{1,4}:){0,3}|(:0{1,4}){4}:)0{0,3}1:[Ff][Ff]([0-9a-fA-F]{2})(::|:([0-9a-fA-F]{1,4})))/i // 过滤多播(ff00::/8或者ff02::1:FF/26)
    };

    if (isEmptyInputValue(value)) {
      return null; // 在可选控制器下不验证
    }

    const isValid =
      IPV6_REGEXP.include.test(value) &&
      !IPV6_REGEXP.fullZeroIp.test(value) &&
      !IPV6_REGEXP.loopBackAddr.test(value) &&
      !IPV6_REGEXP.multicast.test(value);
    return isValid;
  }

  /**
   * IPV6支持多播
   */
  static validateIpv6WithMulticast(value) {
    if (isEmptyInputValue(value)) {
      return null;
    }

    const ipv4Reg =
      '((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)';
    // tslint:disable-next-line:max-line-length
    const reg =
      '^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|' +
      '([0-9a-fA-F]{1,4}:){1,7}:|' +
      '([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|' +
      '([0-9a-fA-F]{1,4}:){1,5}:' +
      ipv4Reg +
      '|' +
      '([0-9a-fA-F]{1,4}:){6,6}' +
      ipv4Reg +
      '|' +
      '([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|' +
      '([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|' +
      '([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,1}:' +
      ipv4Reg +
      '|' +
      '([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|' +
      '([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,2}:' +
      ipv4Reg +
      '|' +
      '([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|' +
      '([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,3}:' +
      ipv4Reg +
      '|' +
      '[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|' +
      '[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,4}):' +
      ipv4Reg +
      '|' +
      ':((:[0-9a-fA-F]{1,4}){1,5}):' +
      ipv4Reg +
      '|' +
      ':((:[0-9a-fA-F]{1,4}){1,7}|:)|' +
      '[Ff][Ee]08:(:[0-9a-fA-F]{1,4}){2,2}%[0-9a-zA-Z]{1,}|' +
      '(0{1,4}:){6,6}' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,5}:' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,4}:(0{1,4}:)' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,3}:(0{1,4}:){1,2}' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,2}:(0{1,4}:){1,3}' +
      ipv4Reg +
      '|' +
      '(0{1,4}:):(0{1,4}:){1,4}' +
      ipv4Reg +
      '|' +
      '::(0{1,4}:){1,5}' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){5,5}' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,4}:' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,3}:(0{1,4}:)' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '(0{1,4}:){1,2}:(0{1,4}:){1,2}' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '(0{1,4}:):(0{1,4}:){1,3}' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '::(0{1,4}:){1,4}' +
      '[Ff]{4}:' +
      ipv4Reg +
      '|' +
      '::([Ff]{4}:){0,1}' +
      ipv4Reg +
      ')$';
    const multicast = /^([Ff][Ff]00::|[Ff][Ff]00:0{1,4}|[Ff][Ff]02((:0{1,4}){0,3}::(0{1,4}:){0,3}|(:0{1,4}){4}:)0{0,3}1:[Ff][Ff]([0-9a-fA-F]{2})(::|:([0-9a-fA-F]{1,4})))/i;
    const isValid =
      (new RegExp(reg).test(value) && value !== '::') || multicast.test(value);

    return isValid;
  }

  getProductName(): string {
    // 白牌化
    const isWhitebox = this.whitebox.isWhitebox;
    // 云备份
    const isCloudBackup = includes(
      [
        DataMap.Deploy_Type.cloudbackup2.value,
        DataMap.Deploy_Type.cloudbackup.value,
        DataMap.Deploy_Type.hyperdetect.value
      ],
      this.i18n.get('deploy_type')
    );
    // 安全一体机
    const isCyberEngine =
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
    // 开源
    const isOpenBackup = includes(
      [DataMap.Deploy_Type.openOem.value, DataMap.Deploy_Type.openServer.value],
      this.i18n.get('deploy_type')
    );

    if (isWhitebox) {
      return this.whitebox.oem['vendor'];
    }
    if (isCloudBackup) {
      return DataMap.productName.protectManager.value;
    }
    if (isCyberEngine) {
      return DataMap.productName.cyberEngine.value;
    }
    if (isOpenBackup) {
      return DataMap.productName.openSource.value;
    }
    return DataMap.productName.oceanProtect.value;
  }

  rejectAgentsByVersion(agents, targetVersion: string | string[]) {
    return reject(agents, item => {
      if (isString(targetVersion)) {
        return (
          isString(item.environment?.version) &&
          item.environment?.version.indexOf(targetVersion) !== -1
        );
      } else {
        return (
          isString(item.environment?.version) &&
          !isEmpty(
            find(
              targetVersion,
              v => item.environment?.version.indexOf(v) !== -1
            )
          )
        );
      }
    });
  }
}
