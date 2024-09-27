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
import { BaseUtilService } from 'app/shared';
import { Injectable } from '@angular/core';

export enum IPTypeE {
  IPV4 = 'IPV4',
  IPV6 = 'IPV6'
}

@Injectable({
  providedIn: 'root'
})
export class IpUtilService {
  /**
   * 获取IP类型，IPV6必须冒号
   */
  static getType(ip: string): IPTypeE {
    return ip.includes(':') ? IPTypeE.IPV6 : IPTypeE.IPV4;
  }

  /**
   * 获取IPV4的值权重值
   */
  static getIPV4Value(ip: string): string {
    return ip
      .split('.')
      .map(a => a.padStart(3, '0'))
      .join('.');
  }

  /**
   * 获取IPV6的权重值
   */
  static getIPV6Value(ip: string): string {
    // 检测是否包含IPV4地址，如果是需要先转换为IPV6
    const ipv4 = ip.substring(ip.lastIndexOf(':') + 1);
    if (BaseUtilService.validateIpv4(ipv4)) {
      ip =
        ip.substring(0, ip.lastIndexOf(':') + 1) +
        IpUtilService.convertIPV4ToIPV6(ipv4);
    }

    const arr = ip.split('::');
    const segmentLength = arr.reduce<number>((acc, ipSegment) => {
      if (ipSegment) {
        acc += ipSegment.split(':').length;
      }
      return acc;
    }, 0);

    ip = ip.replace('::', ':' + '0:'.repeat(8 - segmentLength)).toUpperCase();
    if (ip.startsWith(':')) {
      ip = ip.substring(1);
    }
    if (ip.endsWith(':')) {
      ip = ip.substring(0, ip.length - 1);
    }

    return ip
      .split(':')
      .map(a => a.padStart(4, '0'))
      .join(':');
  }

  /**
   * 将IPV4转换为IPV6
   */
  static convertIPV4ToIPV6(ipv4: string): string {
    return ipv4.split('.').reduce<string>((acc, value, index) => {
      if (index === 2) {
        acc += ':';
      }
      acc += (+value).toString(16).padStart(2, '0');
      return acc;
    }, '');
  }

  /**
   * 比较两个IP地址的值大小
   *
   * @param   {string}  ip1
   * @param   {string}  ip2
   *
   * @return  {number}  大于0，等于0或小于0
   */
  static compare(ip1: string, ip2: string): number {
    if (!ip1 && !ip2) {
      return 0;
    }
    if (ip1 && !ip2) {
      return 1;
    }
    if (!ip1 && ip2) {
      return -1;
    }

    const ipType1 = IpUtilService.getType(ip1);
    const ipType2 = IpUtilService.getType(ip2);

    // 两个IP类型不同，按照普通字符串比较
    if (ipType1 !== ipType2) {
      return ip1.localeCompare(ip2);
    }

    const getValueFn =
      ipType1 === IPTypeE.IPV4
        ? IpUtilService.getIPV4Value
        : IpUtilService.getIPV6Value;
    const ipValue1 = getValueFn(ip1);
    const ipValue2 = getValueFn(ip2);

    return ipValue1.localeCompare(ipValue2);
  }

  /**
   * 比较两个IP段的关系
   *  @param ip1 IP地址段1，以-分隔
   *  @param ip2 IP地址段2，以-分隔
   *  @returns 相交返回1，等于返回0，不相交返回-1
   */
  static compareIPSegment(ip1: string, ip2: string): number {
    if (ip1 === ip2) {
      return 0;
    }

    const [ipStart1, ipEnd1] = ip1.split('-');
    const [ipStart2, ipEnd2] = ip2.split('-');

    const ipType1 = IpUtilService.getType(ipStart1);
    const ipType2 = IpUtilService.getType(ipStart2);

    if (ipType1 !== ipType2) {
      return -1;
    }

    // 两个都不是IP段，只有等于和不相交的关系
    if (!ipEnd1 && !ipEnd2) {
      if (IpUtilService.compare(ipStart1, ipStart2) === 0) {
        return 0;
      }
      return -1;
    } else if (!ipEnd1 && ipEnd2) {
      // 第一个不是IP段，第二个是IP段，则判断第一个IP是否在第二个IP段范围内
      // 即第一个IP大于等于第二个IP段的开始IP，小于等于第二个IP段的结束IP
      if (
        IpUtilService.compare(ipStart1, ipStart2) >= 0 &&
        IpUtilService.compare(ipStart1, ipEnd2) <= 0
      ) {
        return 1;
      }
      return -1;
    } else if (ipEnd1 && !ipEnd2) {
      // 第一个是IP段，第二个不是IP段，则判断第二个IP是否在第一个IP段范围内
      // 即第二个IP大于等于第一个IP段的开始IP，小于等于第一个IP段的结束IP
      if (
        IpUtilService.compare(ipStart2, ipStart1) >= 0 &&
        IpUtilService.compare(ipStart2, ipEnd1) <= 0
      ) {
        return 1;
      }
      return -1;
    } else {
      // 输入的IP段，当前已存在的也是IP段

      // 先判断是否相等
      if (
        IpUtilService.compare(ipStart1, ipStart2) === 0 &&
        IpUtilService.compare(ipEnd1, ipEnd2) === 0
      ) {
        return 0;
      } else if (
        IpUtilService.compare(ipEnd1, ipStart2) < 0 ||
        IpUtilService.compare(ipStart1, ipEnd2) > 0
      ) {
        // 再判断是否不相交，即IP段1在IP段2的左边，或IP段1在IP段2的右边
        return -1;
      }
      return 1;
    }
  }
}
