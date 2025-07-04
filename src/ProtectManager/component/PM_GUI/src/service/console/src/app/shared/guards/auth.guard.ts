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
import {
  ActivatedRouteSnapshot,
  CanActivate,
  CanActivateChild,
  Router,
  RouterStateSnapshot,
  UrlTree
} from '@angular/router';
import { MessageService } from '@iux/live';
import { Observable } from 'rxjs';
import { auditTime, map, tap } from 'rxjs/operators';
import { CookieService, GlobalService, I18NService } from '../services';
import { getAccessibleUrl } from '../utils';

@Injectable({
  providedIn: 'root'
})
export class AuthGuard implements CanActivate, CanActivateChild {
  constructor(
    private router: Router,
    private i18n: I18NService,
    private cookieService: CookieService,
    private globalService: GlobalService,
    private messageService: MessageService
  ) {}

  canActivate(
    next: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ):
    | Observable<boolean | UrlTree>
    | Promise<boolean | UrlTree>
    | boolean
    | UrlTree {
    // TODO: 待优化url
    const url: string = state.url.split('?')[0];
    return this.getRole().pipe(
      map(res => {
        const o = getAccessibleUrl(url, this.cookieService, this.i18n, true);
        return !!o.length;
      }),
      tap(res => {
        // 提示没有访问权限
        if (!res) {
          this.noAuthTips();
          this.router.navigate(['/home']);
        }
      })
    );
  }

  canActivateChild(
    childRoute: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ):
    | boolean
    | UrlTree
    | Observable<boolean | UrlTree>
    | Promise<boolean | UrlTree> {
    return this.canActivate(childRoute, state);
  }

  getRole() {
    return this.globalService.getUserInfo().pipe(
      auditTime(4),
      map(res => this.cookieService.role)
    );
  }

  noAuthTips() {
    this.messageService.error(this.i18n.get('common_no_auth_label'), {
      lvMessageKey: 'lvMsg_key_noauth_ex',
      lvShowCloseButton: true
    });
  }
}
