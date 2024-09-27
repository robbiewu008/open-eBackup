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
/**
 * 用于侧拉展开的modalService
 * 具有嵌套收缩展开功能
 * 前提: 所有的modals带上key(uuid)
 */

import { Injectable } from '@angular/core';
import { ModalService, ModalRef } from '@iux/live';
import { includes, mapValues, isFunction, assign, last } from 'lodash';
import { CommonConsts, keyWidthCache } from '../consts';

@Injectable({
  providedIn: 'root'
})
export class DrawModalService extends ModalService {
  readonly deference = 16;

  create<T>(options: any): ModalRef {
    if ('drawer' === options.lvType) {
      const targetWidth = options.lvWidth;
      const modals = this.modals;
      setTimeout(() => {
        this.updateWidthBeforeCreate(modals, targetWidth);
      }, 0);
      // Add default options lvAfterClose fn
      const afterClose = options.lvAfterClose;
      options.lvAfterClose = result => {
        if (isFunction(afterClose)) {
          afterClose(result);
        }
        this.updateWidthAfterClose(options);
      };
    }
    if (
      this.getUserCookie() === CommonConsts.HCS_USER_TYPE &&
      options.lvType === 'drawer'
    ) {
      assign(options, {
        lvDrawerPositionOffset: [0, 0, 0, 0],
        positionOffset: ['0px', '0px', '0px', '0px']
      });
    }
    const modalRef = super.create(options);
    return modalRef;
  }

  private getUserCookie() {
    const cookies = document.cookie.split(';');
    for (let i = 0; i < cookies.length; i++) {
      const cookiePair = cookies[i].split('=');
      if (cookiePair[0].trim() === 'userType') {
        return decodeURIComponent(cookiePair[1]);
      }
    }
  }

  private updateWidthBeforeCreate(modals, targetWidth) {
    let index, latestWidth, latestKey;

    if (modals.length) {
      index = modals.length - 1;
      latestWidth = modals[index].modal.lvWidth;
      latestKey = modals[index].key;
      keyWidthCache.push({
        latestKey,
        latestWidth,
        isLatestDetail: !modals[index].lvModality
      });
      for (; index >= 0; index--) {
        latestWidth = modals[index].modal.lvWidth;
        latestKey = modals[index].key;
        this.update(latestKey, { lvWidth: targetWidth });
        targetWidth += this.deference;
      }
    }
  }

  private updateWidthAfterClose(options) {
    if (keyWidthCache.length) {
      const { latestKey, latestWidth } = keyWidthCache.pop();
      this.update(latestKey, { lvWidth: latestWidth });
      let tempWidth = last(keyWidthCache)?.latestWidth;
      for (let i = keyWidthCache.length - 1; i >= 0; i--) {
        this.update(keyWidthCache[i].latestKey, {
          lvWidth: tempWidth
        });
        tempWidth += this.deference;
        isFunction(options.restoreWidth) &&
          options.restoreWidth(options.lvComponentParams);
      }
    }
  }

  /**
   * 用于detail弹框
   * 约定id="outerClosable" 为outercloseable排除元素
   * 点击除弹出位置外的其他位置关闭
   * 不收起前面的
   * 若发生更新，以更新的那一级为准，关闭往后的。
   * @param param 见官网modal配置
   */
  openDetailModal(param) {
    param = { ...param, lvModality: false, lvHeader: ' ' };
    if (includes(mapValues(this.modals, 'key'), param.lvModalKey)) {
      const start = (this.modals as any).findIndex(
        item => item.key === param.lvModalKey
      );
      const modalLen = this.modals.length;
      for (let i = modalLen - 1; i > start; i--) {
        (this.modals[i] as any).modal.getInstance().close();
      }
      if (
        this.getUserCookie() === CommonConsts.HCS_USER_TYPE &&
        param.lvType === 'drawer'
      ) {
        assign(param, {
          lvDrawerPositionOffset: [0, 0, 0, 0],
          positionOffset: ['0px', '0px', '0px', '0px']
        });
      }
      this.update(param.lvModalKey, param);
    } else {
      const modalKeys = [];
      this.modals.forEach(item => {
        modalKeys.push((item as any).key);
      });
      param = {
        ...param,
        lvParentModalKeys: modalKeys,
        lvOuterClosable: {
          exclusions: [
            ...Array.from(document.querySelectorAll('#outerClosable'))
          ]
        }
      };
      this.create(param);
    }
  }
}
