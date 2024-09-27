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
import { BehaviorSubject } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class CookieService {
  private role$ = new BehaviorSubject<number>(-1);
  private isInited$ = new BehaviorSubject<boolean>(false);
  private initedStatus$ = new BehaviorSubject<number>(2);
  private isCloudBackup$ = new BehaviorSubject<boolean>(false);
  private filterCluster$ = new BehaviorSubject<any>(null);

  get role() {
    return this.role$.value;
  }

  get isInited() {
    return this.isInited$.value;
  }

  get initedStatus() {
    return this.initedStatus$.value;
  }

  get isCloudBackup() {
    return this.isCloudBackup$.value;
  }

  get filterCluster() {
    return this.filterCluster$.value;
  }

  getRoleStream() {
    return this.role$;
  }

  // 获取Cookie值
  get(key: string) {
    if (!key) {
      return null;
    }

    const cookies = document.cookie.split(';');

    // tslint:disable-next-line: prefer-for-of
    for (let i = 0; i < cookies.length; i++) {
      const cookiePair = cookies[i].split('=');

      if (key === cookiePair[0].trim()) {
        return decodeURIComponent(cookiePair[1]);
      }
    }

    return null;
  }

  // 设置Cookie值
  set(key: string, value: string) {
    if (!key) {
      return;
    }

    window.document.cookie = `${key}=${encodeURIComponent(
      value
    )}; path=/console/;secure=true`;
  }

  // 移除Cookie值
  remove(key: string) {
    if (!key) {
      return;
    }

    window.document.cookie =
      `${key}=''; path=/console/;expires=` + new Date(0).toUTCString();
  }

  // 移除所有cookie值
  removeAll(exclude?) {
    const keys = document.cookie.match(/[^ =;]+(?=\=)/g);
    if (keys) {
      for (let i = keys.length; i--; ) {
        if (exclude === keys[i]) {
          continue;
        }
        document.cookie =
          keys[i] + '=0;path=/console/;expires=' + new Date(0).toUTCString();
      }
    }
  }

  // 设置临时变量
  setRole(value) {
    this.role$.next(+value);
  }

  setIsInited(value) {
    this.isInited$.next(value);
  }

  setInitedStatus(value) {
    this.initedStatus$.next(value);
  }

  setIsCloudBackup(value) {
    this.isCloudBackup$.next(value);
  }

  setFilterCluster(value) {
    this.filterCluster$.next(value);
  }
}
