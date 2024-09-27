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
  AfterViewInit,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  ApiMultiClustersService,
  BaseUtilService,
  CommonConsts,
  DataMapService,
  I18NService,
  RoleApiService,
  UserRoleI18nMap,
  UserRoleType,
  UsersApiService,
  WarningMessageService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig
} from 'app/shared/components/pro-table';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  each,
  find,
  first,
  isEmpty,
  isUndefined,
  size,
  toString
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-auth-user',
  templateUrl: './auth-user.component.html',
  styleUrls: ['./auth-user.component.less']
})
export class AuthUserComponent implements OnInit, AfterViewInit {
  @Input() rowItem;
  @Input() localCluster;
  userTableData;
  userTableConfig: TableConfig;
  formGroup: FormGroup;
  userRoleType = UserRoleType;
  userRoleI18nMap = UserRoleI18nMap;
  selectionData = [];
  userValid$ = new Subject<boolean>();
  passwordErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 64])
  });

  usernameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [5, 64])
  });
  @ViewChild('userTable', { static: false }) userTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private roleApiService: RoleApiService,
    private userApiService: UsersApiService,
    private baseUtilService: BaseUtilService,
    private infoMessageService: InfoMessageService,
    private warningMessageService: WarningMessageService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngAfterViewInit() {
    if (!this.rowItem.isModify) {
      this.userTable.fetchData();
    }
  }

  ngOnInit() {
    this.initForm();
    this.initTableConfig();
  }

  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(5, 64)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validLength(8, 64)
        ]
      })
    });
  }

  getUser() {
    this.userApiService
      .getAllUserUsingGET({
        startIndex: 1,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        filter: JSON.stringify({
          isDefault: true,
          isNormalUser: true
        })
      })
      .subscribe(res => {
        each(res.userList, item => {
          assign(item, {
            login: toString(item.login),
            disabled: !isEmpty(
              find(this.rowItem.authUserList, { userId: item.userId })
            )
          });
        });
        this.userTableData = {
          total: res.total,
          data: res.userList
        };
      });
  }

  initTableConfig() {
    this.userTableConfig = {
      table: {
        async: false,
        columns: this.rowItem.isModify
          ? [
              {
                key: 'userName',
                name: this.i18n.get('system_local_cluster_username_label'),
                sort: true,
                filter: {
                  type: 'search',
                  filterMode: 'contains'
                }
              },
              {
                key: 'managedUserName',
                name: this.i18n.get('system_managed_cluster_username_label'),
                sort: true,
                filter: {
                  type: 'search',
                  filterMode: 'contains'
                }
              }
            ]
          : [
              {
                key: 'userName',
                name: this.i18n.get('system_local_cluster_username_label'),
                sort: true,
                filter: {
                  type: 'search',
                  filterMode: 'contains'
                }
              }
            ],
        compareWith: 'userId',
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        rows: {
          selectionMode: 'single'
        },
        fetchData: (filter: Filters, params) => {
          this.getUser();
        },
        selectionChange: data => {
          this.selectionData = data;
          this.userValid$.next(!!size(data));
          if (this.rowItem.isModify && !!size(data)) {
            this.formGroup
              .get('username')
              .setValue(first(data).managedUserName);
          }
        }
      },
      pagination: {
        mode: 'simple',
        pageIndex: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    if (this.rowItem.isModify) {
      this.userTableData = {
        data: this.rowItem.authUserList,
        total: size(this.rowItem.authUserList)
      };
    }
  }

  modifyClustersAuth(observer) {
    this.multiClustersServiceApi
      .modifyManagedClustersAuth({
        userId: first(this.selectionData).userId,
        clusterId: this.rowItem.clusterId,
        updateManagedClustersRelationReq: {
          managedUserName: this.formGroup.value.username,
          managedPassword: this.formGroup.value.password
        }
      })
      .subscribe(
        res => {
          observer.next();
          observer.complete();
        },
        err => {
          observer.error(err);
          observer.complete();
        }
      );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.rowItem.isModify) {
        // 修改了用户名
        if (
          this.formGroup.value.username !==
          first(this.selectionData).managedUserName
        ) {
          this.warningMessageService.create({
            content: this.i18n.get('system_modify_auth_warn_label', [
              first(this.selectionData).userName
            ]),
            onOK: () => this.modifyClustersAuth(observer),
            onCancel: () => {
              observer.error(null);
              observer.complete();
            },
            lvAfterClose: result => {
              if (result && result.trigger === 'close') {
                observer.error(null);
                observer.complete();
              }
            }
          });
        } else {
          this.modifyClustersAuth(observer);
        }
      } else {
        this.infoMessageService.create({
          content: this.i18n.get('system_add_auth_info_label', [
            first(this.selectionData).userName,
            this.rowItem.clusterName,
            this.formGroup.value.username
          ]),
          noBreak: true,
          onOK: () => {
            this.multiClustersServiceApi
              .createManagedClustersAuth({
                clusterId: this.rowItem.clusterId,
                anagedClustersRelationReq: {
                  managedPassword: this.formGroup.value.password,
                  managedUserName: this.formGroup.value.username,
                  userId: first(this.selectionData).userId
                }
              })
              .subscribe(
                res => {
                  observer.next();
                  observer.complete();
                },
                err => {
                  observer.error(err);
                  observer.complete();
                }
              );
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      }
    });
  }
}
