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
import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable, NgZone } from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  ASYNC_TASK_URL_WHITE_LIST,
  CommonConsts,
  EMIT_TASK
} from 'app/shared/consts/common.const';
import { each, endsWith, first, includes, some } from 'lodash';
import { Observable } from 'rxjs';
import { delay, tap } from 'rxjs/operators';
import { DataMap } from '../consts/data-map.config';
import { I18NService, GlobalService } from '../services';
import { CookieService } from '../services/cookie.service';

@Injectable()
export class OperationTipsInterceptor implements HttpInterceptor {
  constructor(
    private message: MessageService,
    private i18n: I18NService,
    private router: Router,
    private ngZone: NgZone,
    private globalService: GlobalService,
    private cookieService: CookieService
  ) {}
  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    const opKey = 'akOperationTips',
      excludeMethod = ['get'],
      opContentKeys = 'akOperationTipsContent';
    const showOpTips = req.params.get(opKey) !== 'false',
      tipsContent = req.params.get(opContentKeys);
    const sftpTestUrl = ['^/console/rest/v1/sysbackup/sftp/connection$'];
    const isDataBackup = !includes(
      [
        DataMap.Deploy_Type.cyberengine.value,
        DataMap.Deploy_Type.cloudbackup.value,
        DataMap.Deploy_Type.cloudbackup2.value,
        DataMap.Deploy_Type.hyperdetect.value
      ],
      this.i18n.get('deploy_type')
    );
    // 删除akOperationTips akOperationTipsContent参数
    req.params.delete(opKey);
    req.params.delete(opContentKeys);
    req = req.clone({
      params: req.params
    });

    return next.handle(req).pipe(
      delay(100), // 延迟至动画完成
      tap(event => {
        const isHcsUser =
          this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
        if (event instanceof HttpResponse) {
          const method = req.method.toLowerCase();
          if (showOpTips && excludeMethod.indexOf(method) === -1) {
            const isAsyncTask =
              ASYNC_TASK_URL_WHITE_LIST.some(item => {
                if (isHcsUser) {
                  return (
                    new RegExp(
                      item.url.replace('/console/rest/', '/cbs/op/rest/')
                    ).test(req.url) && item.method === method
                  );
                }
                return (
                  new RegExp(item.url).test(req.url) && item.method === method
                );
              }) && !includes(['/console/rest/v1/copies/index'], req.url);
            const isSftpTestUrl = some(
              sftpTestUrl,
              url => new RegExp(url).test(req.url) && 'post' === method
            );
            this.message.success(
              tipsContent ||
                this.i18n.get(
                  isAsyncTask
                    ? 'common_command_successfully_label'
                    : isSftpTestUrl
                    ? 'common_sftp_test_success_label'
                    : 'common_operate_success_label'
                ),
              {
                lvShowCloseButton: true,
                lvOnShow: () => {
                  const resourceLink = document.getElementsByClassName(
                    'resource-link-btn'
                  );
                  if (resourceLink) {
                    each(resourceLink, item => {
                      item.addEventListener('click', () => {
                        this.ngZone.run(() => {
                          if (
                            includes(
                              [
                                DataMap.Deploy_Type.cloudbackup.value,
                                DataMap.Deploy_Type.cloudbackup2.value
                              ],
                              this.i18n.get('deploy_type')
                            )
                          ) {
                            this.router.navigate([
                              'protection/storage/local-file-system'
                            ]);
                          } else if (
                            includes(
                              [DataMap.Deploy_Type.hyperdetect.value],
                              this.i18n.get('deploy_type')
                            )
                          ) {
                            this.router.navigate([
                              '/protection/storage/local-resource'
                            ]);
                          } else {
                            if (isHcsUser && window.parent) {
                              const parentUrl = window.parent.location.href;
                              window.parent.location.href = `${first(
                                parentUrl.split('#')
                              )}#/overview`;
                            } else {
                              this.router.navigate(['protection/summary']);
                            }
                          }
                        });
                      });
                    });
                  }

                  const filesystemLink = document.getElementsByClassName(
                    'filesystem-link-btn'
                  );
                  if (filesystemLink) {
                    each(filesystemLink, item => {
                      item.addEventListener('click', () => {
                        this.ngZone.run(() => {
                          this.globalService.emitStore({
                            action: 'toFilesystem',
                            state: ''
                          });
                        });
                      });
                    });
                  }

                  if (isAsyncTask) {
                    setTimeout(() => {
                      this.globalService.emitStore({
                        action: EMIT_TASK,
                        state: ''
                      });
                    }, 500);
                    const taskLink = document.getElementsByClassName(
                      'task-link-btn'
                    );
                    if (taskLink) {
                      each(taskLink, item => {
                        item.addEventListener('click', () => {
                          this.ngZone.run(() => {
                            if (isHcsUser && window.parent) {
                              const parentUrl = window.parent.location.href;
                              window.parent.location.href = `${first(
                                parentUrl.split('#')
                              )}#/insight/jobs`;
                            } else {
                              this.router.navigate(['insight/jobs']);
                            }
                          });
                        });
                      });
                    }

                    const exportFilesResult = document.getElementsByClassName(
                      'export-files-result'
                    );
                    if (exportFilesResult) {
                      each(exportFilesResult, item => {
                        item.addEventListener('click', () => {
                          this.ngZone.run(() => {
                            if (isHcsUser && window.parent) {
                              const parentUrl = window.parent.location.href;
                              window.parent.location.href = `${first(
                                parentUrl.split('#')
                              )}#/system/export-query`;
                            } else {
                              this.router.navigate(['system/export-query']);
                            }
                          });
                        });
                      });
                    }
                  }
                }
              }
            );
          }

          // 通知更新应用资源数
          if (
            endsWith(req.url, '/rest/v2/resources') &&
            req.method === 'GET' &&
            isDataBackup &&
            showOpTips
          ) {
            this.globalService.emitStore({
              action: 'emitRefreshApp',
              state: true
            });
          }
        }
      })
    );
  }
}
