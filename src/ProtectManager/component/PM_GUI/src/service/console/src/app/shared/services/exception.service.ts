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
import { HttpErrorResponse, HttpResponse } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Router } from '@angular/router';
import { MessageboxService, MessageService } from '@iux/live';
import { isString } from 'lodash';
import { TimeoutError } from 'rxjs';
import { HTTP_STATUS, ErrorCode } from '../consts';
import { AuthApiService } from './auth-api.service';
import { CookieService } from './cookie.service';
import { I18NService } from './i18n.service';
import { GlobalService } from './store.service';
import { isJson } from '../utils';

@Injectable({
  providedIn: 'root'
})
export class ExceptionService {
  constructor(
    private router: Router,
    private i18n: I18NService,
    private messageService: MessageService,
    private messageBox: MessageboxService,
    private cookieService: CookieService,
    private authApiService: AuthApiService,
    private globalService: GlobalService
  ) {}

  isException(res: HttpResponse<any>) {
    return false;
  }

  doException(error) {
    if (error instanceof HttpErrorResponse) {
      const status = error.status;
      switch (status) {
        case HTTP_STATUS.NOT_FOUND:
        case HTTP_STATUS.UNAUTHORIZED:
          this.messageService.error(
            this.i18n.get(`common_http_${status}_label`),
            {
              lvMessageKey: 'lvMsg_key_404_401',
              lvShowCloseButton: true
            }
          );
          break;
        case HTTP_STATUS.TIMEOUT:
          this.timeoutLogout();
          break;
        case HTTP_STATUS.GATEWAYTIMOUT:
          this.messageService.error(this.i18n.get('common_timeout_label'), {
            lvMessageKey: 'lvMsg_key_timeout_ex',
            lvShowCloseButton: true
          });
          break;
        case HTTP_STATUS.FORBIDDEN:
          this.messageService.error(
            this.i18n.get(`common_http_${status}_label`),
            {
              lvMessageKey: 'lvMsg_key_403',
              lvShowCloseButton: true
            }
          );
          break;
        default:
          this.parseErrorCode(error);
          break;
      }
    } else if (error instanceof TimeoutError) {
      this.messageService.error(this.i18n.get('common_timeout_label'), {
        lvMessageKey: 'lvMsg_key_timeout_ex',
        lvShowCloseButton: true
      });
    } else {
      this.messageService.error(
        this.i18n.get('common_unknown_exception_label'),
        {
          lvMessageKey: 'lvMsg_key_unknown_ex',
          lvShowCloseButton: true
        }
      );
    }
  }

  timeoutLogout() {
    this.messageBox.error({
      lvModalKey: 'errorMsgKey',
      lvHeader: this.i18n.get('common_error_label'),
      lvContent: this.i18n.get('common_timeout_logout_label'),
      lvAfterClose: () => {
        this.router.navigate(['/login']).then(() => {
          this.cookieService.removeAll(this.i18n.languageKey);
          if (this.router.url === '/login') {
            this.globalService.emitStore({
              state: true,
              action: 'isLoginView'
            });
            window.location.reload();
            return;
          }

          // 跳转失败后再次跳转
          if (this.router.url === '/') {
            this.router.navigateByUrl('/login').then(() => {
              window.location.reload();
            });
            return;
          }
        });
      },
      lvAfterOpen: modal => {
        let timer = 10;
        modal.lvOkText = `${this.i18n.get('common_ok_label')}(` + timer + `s)`;
        const interval = setInterval(() => {
          timer--;
          modal.lvOkText =
            `${this.i18n.get('common_ok_label')}(` + timer + `s)`;
        }, 1e3);

        const timeOut = setTimeout(() => {
          clearInterval(interval);
          clearTimeout(timeOut);
          modal.close();
        }, 10 * 1e3);
      }
    });
  }

  getErrorMessage(error): string {
    let message: string;
    if (!error) {
      return this.i18n.get('common_unknown_exception_label');
    }
    if (isJson(error)) {
      error = JSON.parse(error);
    }
    const errorCode = error.errorCode;
    const errorMessage = error.errorMessage;
    let parameters = error.parameters || [];
    if (isString(error.detailParam)) {
      parameters = [parameters];
    }
    message = this.i18n.get(errorCode, parameters, false, true);
    if (errorMessage && (errorCode === message || !message)) {
      message = this.i18n.get(errorMessage, parameters, false, true);
    }
    if (!message) {
      message = this.i18n.get('common_unknown_exception_label');
    }
    return message;
  }

  parseErrorCode(httpResponse) {
    let response: any = httpResponse.error;
    try {
      if (response instanceof Blob) {
        this.parseBlob(httpResponse.error).then(res => {
          response = JSON.parse(res);
          this.alertMsg(response);
        });
        return;
      }

      if (response && isString(response)) {
        response = JSON.parse(response);
      }

      this.alertMsg(response);
    } catch (ex) {
      this.messageService.error(this.i18n.get('common_exception_label'), {
        lvMessageKey: 'lvMsg_key_unknown_ex',
        lvShowCloseButton: true
      });
    }
  }

  alertMsg(response: any) {
    const errorCode = response.errorCode;
    const errorMessage = response.errorMessage;
    let parameters = response.parameters || [];
    if (isString(response.detailParam)) {
      parameters = [parameters];
    }

    let errorMsg = this.i18n.get(errorCode, parameters);
    if (errorMessage && (errorCode === errorMsg || !errorMsg)) {
      errorMsg = this.i18n.get(errorMessage, parameters);
    }

    if (errorMsg === 'undefined') {
      throw new Error(`unknow exception, errorMsg is undefined`);
    }
    this.messageService.error(
      errorMsg || this.i18n.get('common_unknown_exception_label'),
      {
        lvShowCloseButton: true,
        lvMessageKey: 'exceptionMesageKey'
      }
    );
    if (response.errorCode === ErrorCode.ReachedTheMaxFailed) {
      this.timeoutLogout();
    } else if (response.errorCode === ErrorCode.UserPasswordNotChange) {
      this.cookieService.removeAll(this.i18n.languageKey);
      this.router.navigateByUrl('/login');
    } else if (response.errorCode === ErrorCode.UserStatusAbnormal) {
      this.cookieService.removeAll(this.i18n.languageKey);
      this.globalService.emitStore({
        action: response.errorCode,
        state: ''
      });
      this.router.navigateByUrl('/login').then(() => {
        this.messageService.error(errorMsg, {
          lvShowCloseButton: true,
          lvMessageKey: 'exceptionMesageKey'
        });
      });
    }
  }

  parseBlob(blob: Blob): Promise<any> {
    const fileReader = new FileReader();
    return new Promise((resolve, reject) => {
      fileReader.onloadend = (event: any) => {
        resolve(event.target!.result);
      };
      fileReader.onerror = reject;
      fileReader.readAsText(blob);
    });
  }
}
