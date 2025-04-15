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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  NgModule,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormsModule } from '@angular/forms';
import { DomSanitizer } from '@angular/platform-browser';
import { Router } from '@angular/router';
import {
  CheckboxModule,
  MenuItem,
  MessageboxService,
  SortDirective
} from '@iux/live';
import {
  BaseModule,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  LANGUAGE,
  MODAL_COMMON,
  OperateItems,
  RoleType,
  UserRoleDescI18nMap,
  UserRoleI18nMap,
  UserRoleType,
  USER_ROLE_TYPE
} from 'app/shared';
import {
  RoleApiService,
  SecurityApiService,
  UsersApiService
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { WarningMessageService } from 'app/shared/services/warning-message.service';
import {
  assign,
  defer,
  each,
  extend,
  filter,
  includes,
  intersection,
  isEmpty,
  map as _map,
  toString
} from 'lodash';
import { Subject } from 'rxjs';
import { map } from 'rxjs/operators';
import { AssociatedusersComponent } from './associatedusers/associatedusers.component';
import { CreateuserComponent } from './createuser/createuser.component';
import { EdituserComponent } from './edituser/edituser.component';
import { ResetpwdComponent } from './resetpwd/resetpwd.component';
import { SetEmailComponent } from './set-email/set-email.component';
import { UnlockComponent } from './unlock/unlock.component';
import { UserdetailComponent } from './userdetail/userdetail.component';

@Component({
  selector: 'userrole',
  templateUrl: './user-role.component.html',
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class UserroleComponent implements OnInit {
  userName;
  filterParams = {} as any;
  userRoleType = UserRoleType;
  userRoleI18nMap = UserRoleI18nMap;
  userRoleDescI18nMap = UserRoleDescI18nMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  optWidth = CommonConsts.TABLE_OPERATION_WIDTH;
  sortSources = {};
  warningComponent = WarningComponent;

  userData: any[];
  roleData: any[];
  userItems: MenuItem[];
  currentUser: any;
  activeSort;

  userType = USER_ROLE_TYPE;
  tabActiveIndex = USER_ROLE_TYPE.USER;

  roleType = RoleType;
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  @ViewChild(SortDirective, { static: false }) lvSort: SortDirective;
  roleNameFilterMap = [
    {
      value: 'Role_SYS_Admin',
      label: this.i18n.get('common_sys_admin_label'),
      key: 'Role_SYS_Admin'
    },
    {
      value: 'Role_DP_Admin',
      label: this.i18n.get(
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
          ? 'common_user_cyber_label'
          : 'common_user_label'
      ),
      key: 'Role_DP_Admin'
    },
    {
      value: 'Role_Auditor',
      label: this.i18n.get('common_auditor_label'),
      key: 'Role_Auditor'
    },
    {
      value: 'Role_RD_Admin',
      label: this.i18n.get('common_remote_device_administrator_label'),
      key: 'Role_RD_Admin'
    },
    {
      value: 'Role_DR_Admin',
      label: this.i18n.get('common_dme_admin_label'),
      key: 'Role_DR_Admin'
    }
  ].filter(item => {
    const deployType = this.i18n.get('deploy_type');
    if (
      (['d3', 'd4', 'cloudbackup'].includes(deployType) &&
        ['Role_DP_Admin', 'Role_DR_Admin'].includes(item.key)) ||
      (this.appUtilsService.isDistributed &&
        ['Role_RD_Admin', 'Role_DR_Admin'].includes(item.key)) ||
      ([
        DataMap.Deploy_Type.decouple.value,
        DataMap.Deploy_Type.openServer.value
      ].includes(deployType) &&
        item.key === 'Role_DR_Admin')
    ) {
      return false;
    } else if (['d5', 'cyberengine'].includes(deployType)) {
      return item.key === 'Role_RD_Admin' || item.key === 'Role_DR_Admin'
        ? false
        : true;
    }
    return this.cookieService.isCloudBackup
      ? item.key !== 'Role_RD_Admin'
      : true;
  });
  neverExpireFilterMap = this.dataMapService.toArray('passwordType');
  userTypeFilterMap = this.dataMapService
    .toArray('loginUserType')
    .filter(item => {
      if (
        includes(
          [
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value,
            DataMap.Deploy_Type.hyperdetect.value
          ],
          this.i18n.get('deploy_type')
        )
      ) {
        return includes([DataMap.loginUserType.local.value], item.value);
      }
      // 安全一体机包含本地和ldap
      if (this.isCyberengine) {
        return includes(
          [
            DataMap.loginUserType.local.value,
            DataMap.loginUserType.ldap.value,
            DataMap.loginUserType.ldapGroup.value
          ],
          item.value
        );
      }
      return true;
    });
  openStorageGroupTip;

  @ViewChild('dpAdminTipTpl', { static: true }) dpAdminTipTpl;

  constructor(
    public i18n: I18NService,
    public router: Router,
    private messageBox: MessageboxService,
    public drawModalService: DrawModalService,
    public roleApiService: RoleApiService,
    public cookieService: CookieService,
    public usersApiService: UsersApiService,
    public securityApiService: SecurityApiService,
    public warningMessageService: WarningMessageService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private sanitizer: DomSanitizer,
    public appUtilsService?: AppUtilsService
  ) {
    this.openStorageGroupTip = this.sanitizer.bypassSecurityTrustHtml(
      i18n.get('system_open_storage_group_tip_label')
    );
  }

  statusLabel = this.i18n.get('common_status_label');
  userLabel = this.i18n.get('common_users_label');
  closeLabel = this.i18n.get('common_close_label');
  roleLabel = this.i18n.get('common_roles_label');
  lockLabel = this.i18n.get('common_lock_label');
  modifyLabel = this.i18n.get('common_modify_label');
  unlockLabel = this.i18n.get('common_unlock_label');
  deleteLabel = this.i18n.get('common_remove_label');
  createLabel = this.i18n.get('common_create_label');
  restPassword = this.i18n.get('system_user_restpwd_label');
  resetPswdEmail = this.i18n.get('system_reset_password_email_settings_label');
  disablelock1 = this.i18n.get('system_user_disablelock1_label');
  disablelock2 = this.i18n.get('system_user_disablelock2_label');
  disablelock3 = this.i18n.get('system_user_disablelock3_label');
  disabledelete1 = this.i18n.get('system_user_disabledelete1_label');
  disabledelete2 = this.i18n.get('system_user_disabledelete2_label');
  disableresetpwd1 = this.i18n.get('system_user_disableresetpwd1_label');
  disableresetpwd2 = this.i18n.get('system_user_disableresetpwd2_label');

  openStorageGroupLink() {
    const openStorageGroup = document.querySelector('#open-storage-group');
    if (!openStorageGroup) {
      return;
    }
    openStorageGroup.addEventListener('click', () => {
      this.router.navigateByUrl('/system/infrastructure/nas-backup-storage');
    });
  }

  ngOnInit() {
    this.getCurrentUser();
    this.initUser();
    this.virtualScroll.getScrollParam(300, 3);
    this.initUserRoleLabel();
  }

  initUserRoleLabel() {
    if (!this.isCyberengine) {
      return;
    }
    assign(this.userRoleI18nMap, {
      subadmin: 'common_user_cyber_label'
    });
    assign(this.userRoleDescI18nMap, {
      subadmin: 'common_user_cyber_desc_label'
    });
  }

  onChange() {
    this.ngOnInit();

    if (this.tabActiveIndex === this.userType.USER) {
      this.initUser();
    } else {
      this.initRole();
    }
  }

  beforeChange = (originTab: any, activeTab: any) => {
    this.userData = [];
    this.roleData = [];
  };

  activeIndexChange = activeIndex => {
    if (activeIndex === this.userType.USER) {
      setTimeout(() => {
        this.lvSort.clear();
      }, 0);
      this.initUser();
    } else {
      this.initRole();
    }
  };

  initDataProtectUser() {
    const userId = this.cookieService.get('userId');
    const currentCluster = this.cookieService.filterCluster
      ? this.cookieService.filterCluster
      : JSON.parse(
          decodeURIComponent(this.cookieService.get('currentCluster'))
        );

    const isLocal =
      isEmpty(currentCluster) ||
      (currentCluster['clusterId'] === DataMap.Cluster_Type.local.value &&
        currentCluster['clusterType'] === DataMap.Cluster_Type.local.value);
    if (isLocal) {
      this.usersApiService.getUsingGET2({ userId }).subscribe(res => {
        assign(res, {
          login: 'true'
        });
        this.userData = [res];
        this.cdr.detectChanges();
      });
    } else {
      this.usersApiService.getLoginUserUsingGET({}).subscribe(res => {
        assign(res, {
          login: 'true'
        });
        this.userData = [res];
        this.cdr.detectChanges();
      });
    }
  }

  getCurrentUser() {
    const userId = this.cookieService.get('userId');
    this.usersApiService.getUsingGET2({ userId }).subscribe(res => {
      this.currentUser = res;
      this.cdr.detectChanges();
    });
  }

  sortUserChange(source: any) {
    this.sortSources = source;
    this.initUser(source);
  }

  filterChange(e) {
    if (e.key === 'userType') {
      assign(this.filterParams, {
        userTypeList: e.value
      });
    } else {
      assign(this.filterParams, {
        [e.key]: e.value
      });
    }
    this.initUser();
  }

  searchByUserName(userName) {
    assign(this.filterParams, {
      userName: userName
    });
    this.initUser();
  }

  initUser(source?: any) {
    const params = {
      startIndex: this.pageIndex + 1,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (this.cookieService.isCloudBackup && !this.filterParams?.roleName) {
      this.filterParams['roleName'] = _map(this.roleNameFilterMap, 'value');
    }

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        filter: JSON.stringify(this.filterParams)
      });
    }

    if (!isEmpty(source)) {
      extend(params, { orderBy: source.key, orderType: source.direction });
    }

    if (this.cookieService.role === RoleType.DataProtectionAdmin) {
      this.initDataProtectUser();
    } else {
      this.usersApiService
        .getAllUserUsingGET(params)
        .pipe(
          map(res => {
            filter(res.userList, (user: any) => {
              user.login = toString(user.login);
              user.description =
                user.defaultUser &&
                includes(
                  ['sysadmin', 'mmdp_admin', 'mm_audit', 'cluster_admin'],
                  user.description
                )
                  ? this.i18n.get(`common_${user.description}_label`)
                  : user.description;
              return true;
            });
            this.cdr.detectChanges();
            return res;
          })
        )
        .subscribe(
          res => {
            this.total = res.total;
            this.userData = res.userList;
            this.cdr.detectChanges();
          },
          () => {
            this.total = 0;
            this.userData = [];
            this.cdr.detectChanges();
          }
        );
    }
  }

  initRole() {
    this.roleApiService.getUsingGET({}).subscribe(res => {
      this.roleData = filter(res.records, item => {
        const deployType = this.i18n.get('deploy_type');
        if (
          (['d3', 'd4', 'cloudbackup'].includes(deployType) &&
            ['Role_DP_Admin', 'Role_DR_Admin'].includes(item.roleName)) ||
          (this.appUtilsService.isDistributed &&
            ['Role_RD_Admin', 'Role_DR_Admin'].includes(item.roleName)) ||
          ([
            DataMap.Deploy_Type.decouple.value,
            DataMap.Deploy_Type.openServer.value
          ].includes(deployType) &&
            item.roleName === 'Role_DR_Admin')
        ) {
          return false;
        }
        return this.cookieService.isCloudBackup
          ? item.roleName !== 'Role_RD_Admin'
          : true;
      });
      this.cdr.detectChanges();
    });
  }

  getOptItems(user) {
    const menus = [
      {
        id: 'edit',
        label: this.modifyLabel,
        permission: OperateItems.ModifyingUser,
        disabled: user.defaultUser,
        onClick: (d: any) => {
          this.drawModalService.create(
            assign({}, MODAL_COMMON.generateDrawerOptions(), {
              lvWidth: MODAL_COMMON.normalWidth,
              lvClass: 'user-detail-header',
              lvHeader: this.i18n.get('common_modify_colon_label', [
                user.userName
              ]),
              lvOkDisabled: true,
              lvContent: EdituserComponent,
              lvComponentParams: {
                user
              },
              lvAfterOpen: modal => {
                const content = modal.getContentComponent() as EdituserComponent;
                const modalIns = modal.getInstance();
                content.formGroup.statusChanges.subscribe(res => {
                  modalIns.lvOkDisabled = res !== 'VALID';
                });
              },
              lvOk: modal => {
                return new Promise(resolve => {
                  const component = modal.getContentComponent() as EdituserComponent;
                  component.onOK().subscribe({
                    next: () => {
                      resolve(true);
                      this.initUser(this.sortSources);
                    },
                    error: error => resolve(false)
                  });
                });
              }
            })
          );
        }
      },
      {
        id: 'restPwd',
        label: this.restPassword,
        disabled:
          user.userId === this.cookieService.get('userId') ||
          includes(
            [
              DataMap.loginUserType.saml.value,
              DataMap.loginUserType.ldap.value,
              DataMap.loginUserType.ldapGroup.value,
              DataMap.loginUserType.hcs.value,
              DataMap.loginUserType.adfs.value
            ],
            user.userType
          ),
        permission: OperateItems.ResetPassword,
        onClick: (d: any) => {
          this.securityApiService.getUsingGET1({}).subscribe(res => {
            this.drawModalService.create(
              assign({}, MODAL_COMMON.generateDrawerOptions(), {
                lvWidth: MODAL_COMMON.normalWidth,
                lvHeader: this.i18n.get('system_user_restpwd_label'),
                lvContent: ResetpwdComponent,
                lvComponentParams: {
                  user: extend({ ...user, ...res })
                },
                lvOkDisabled: true,
                lvAfterOpen: modal => {
                  const content = modal.getContentComponent() as ResetpwdComponent;
                  const modalIns = modal.getInstance();
                  content.formGroup.statusChanges.subscribe(result => {
                    modalIns.lvOkDisabled = result !== 'VALID';
                  });
                },
                lvOk: modal => {
                  return new Promise(resolve => {
                    const content = modal.getContentComponent() as ResetpwdComponent;
                    content.onOK().subscribe({
                      next: () => {
                        resolve(true);
                        this.initUser(this.sortSources);
                      },
                      error: error => resolve(false)
                    });
                  });
                }
              })
            );
          });
        }
      },
      {
        id: 'setEmail',
        label: this.resetPswdEmail,
        disabled:
          user.rolesSet[0].roleId !== 1 ||
          user.userId !== this.cookieService.get('userId') ||
          includes(
            [
              DataMap.loginUserType.saml.value,
              DataMap.loginUserType.ldap.value,
              DataMap.loginUserType.ldapGroup.value,
              DataMap.loginUserType.hcs.value,
              DataMap.loginUserType.adfs.value
            ],
            user.userType
          ),
        permission: OperateItems.SetRestPswdEmail,
        hidden: includes(
          [
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value,
            DataMap.Deploy_Type.hyperdetect.value
          ],
          this.i18n.get('deploy_type')
        ),
        onClick: data => {
          this.usersApiService
            .getEmailUsingGET({ userId: user.userId })
            .subscribe(res => {
              const emailAddress = res.emailAddress;
              this.drawModalService.create(
                assign({}, MODAL_COMMON.generateDrawerOptions(), {
                  lvWidth: MODAL_COMMON.normalWidth,
                  lvHeader: this.i18n.get(
                    'system_reset_password_email_settings_label'
                  ),
                  lvContent: SetEmailComponent,
                  lvComponentParams: {
                    userId: user.userId,
                    emailAddress
                  },
                  lvOkDisabled: true,
                  lvAfterOpen: modal => {
                    const content = modal.getContentComponent() as SetEmailComponent;
                    const modalIns = modal.getInstance();
                    content.formGroup.statusChanges.subscribe(result => {
                      modalIns.lvOkDisabled = result !== 'VALID';
                    });
                  },
                  lvOk: modal => {
                    return new Promise(resolve => {
                      const content = modal.getContentComponent() as SetEmailComponent;
                      content.onOK().subscribe(
                        res => {
                          resolve(true);
                        },
                        err => {
                          resolve(false);
                        }
                      );
                    });
                  }
                })
              );
            });
        }
      },
      {
        id: 'lock',
        label: this.lockLabel,
        permission: OperateItems.LockingUser,
        disabled:
          user.lock ||
          user.defaultUser ||
          user.userName === 'sysadmin' ||
          user.userId === this.cookieService.get('userId') ||
          includes(
            [
              DataMap.loginUserType.ldap.value,
              DataMap.loginUserType.ldapGroup.value,
              DataMap.loginUserType.hcs.value,
              DataMap.loginUserType.adfs.value
            ],
            user.userType
          ),
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_user_lock_tip_label', [
              user.userName
            ]),
            onOK: () => {
              const userId = user.userId;
              this.usersApiService
                .lockUsingPUT({ userId })
                .subscribe(res => this.initUser(this.sortSources));
            }
          });
        }
      },
      {
        id: 'unLock',
        label: this.unlockLabel,
        permission: OperateItems.UnlockingUser,
        disabled:
          !user.lock ||
          user.userId === this.cookieService.get('userId') ||
          includes(
            [
              DataMap.loginUserType.ldap.value,
              DataMap.loginUserType.ldapGroup.value,
              DataMap.loginUserType.hcs.value
            ],
            user.userType
          ),
        onClick: (d: any) => {
          this.securityApiService.getUsingGET1({}).subscribe(res => {
            this.drawModalService.create(
              assign({}, MODAL_COMMON.generateDrawerOptions(), {
                lvWidth: MODAL_COMMON.normalWidth,
                lvHeader: this.i18n.get('system_permission_auth_label'),
                lvContent: UnlockComponent,
                lvComponentParams: {
                  user: extend({ ...user, ...res })
                },
                lvOkDisabled: true,
                lvAfterOpen: modal => {
                  const content = modal.getContentComponent() as UnlockComponent;
                  const modalIns = modal.getInstance();
                  content.formGroup.statusChanges.subscribe(result => {
                    modalIns.lvOkDisabled = result !== 'VALID';
                  });
                },
                lvOk: modal => {
                  return new Promise(resolve => {
                    const content = modal.getContentComponent() as UnlockComponent;
                    content.onOK().subscribe({
                      next: () => {
                        resolve(true);
                        this.initUser(this.sortSources);
                      },
                      error: error => resolve(false)
                    });
                  });
                }
              })
            );
          });
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeletingUser,
        disabled:
          user.defaultUser ||
          user.userId === this.cookieService.get('userId') ||
          includes([DataMap.loginUserType.hcs.value], user.userType),
        onClick: (d: any) => {
          let contentLabel = this.i18n.get('system_user_delete_label', [
            user.userName
          ]);
          if (
            includes(
              [
                DataMap.Deploy_Type.cyberengine.value,
                DataMap.Deploy_Type.hyperdetect.value
              ],
              this.i18n.get('deploy_type')
            )
          ) {
            contentLabel = this.i18n.get(
              'system_hyperdetect_user_delete_label',
              [user.userName]
            );
          }

          this.drawModalService.create({
            lvModalKey: 'warningMessage',
            lvType: 'dialog',
            lvDialogIcon: 'lv-icon-popup-danger-48',
            lvHeader: this.i18n.get('common_danger_label'),
            lvContent: this.warningComponent,
            lvComponentParams: {
              content: contentLabel,
              userRole: user?.rolesSet[0]?.roleName
            },
            lvWidth: MODAL_COMMON.normalWidth,
            lvOkType: 'primary',
            lvCancelType: 'default',
            lvOkDisabled: true,
            lvFocusButtonId: 'cancel',
            lvCloseButtonDisplay: true,
            lvAfterOpen: modal => {
              const component = modal.getContentComponent() as WarningComponent;
              const modalIns = modal.getInstance();
              component.isChecked$.subscribe(e => {
                modalIns.lvOkDisabled = !e;
              });
            },
            lvOk: modal => {
              const component = modal.getContentComponent() as WarningComponent;
              const userId = user.userId;
              this.usersApiService
                .deleteUserUsingDELETE1({
                  userId,
                  isCleanAllResources: component.deleteResource
                })
                .subscribe(res => this.initUser(this.sortSources));
            }
          });
        }
      }
    ];
    this.userItems = getPermissionMenuItem(menus, this.cookieService.role);
    return this.userItems;
  }

  optCallback: (user) => Array<MenuItem> = user => {
    return this.getOptItems(user);
  };

  createUser() {
    this.securityApiService.getUsingGET1({}).subscribe(res => {
      this.drawModalService.create(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvHeader:
            this.i18n.language === LANGUAGE.EN
              ? `${this.createLabel} ${this.i18n.get('insight_user_label')}`
              : `${this.createLabel}${this.i18n.get('insight_user_label')}`,
          lvContent: CreateuserComponent,
          lvWidth: MODAL_COMMON.normalWidth,
          lvComponentParams: {
            passLenVal: res.passLenVal,
            passComplexVal: res.passComplexVal
          },
          lvOkDisabled: true,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as CreateuserComponent;
            const modalIns = modal.getInstance();
            content.formGroup.statusChanges.subscribe(result => {
              modalIns.lvOkDisabled = result !== 'VALID';
            });
          },
          lvOk: modal => {
            return new Promise(resolve => {
              const content = modal.getContentComponent() as CreateuserComponent;
              content.onOK().subscribe({
                next: res => {
                  resolve(true);
                  this.dpAdminTipBox(res);
                  this.initUser();
                },
                error: error => resolve(false)
              });
            });
          }
        })
      );
    });
  }

  dpAdminTipBox(isDPAdmin) {
    if (!isDPAdmin) {
      return;
    }
    this.messageBox.info({
      lvContent: this.dpAdminTipTpl
    });
    defer(() => this.openStorageGroupLink());
  }

  queryUser(user) {
    const currentCluster = this.cookieService.filterCluster
      ? this.cookieService.filterCluster
      : JSON.parse(
          decodeURIComponent(this.cookieService.get('currentCluster'))
        );
    const params = { userId: user.userId };
    if (!isEmpty(currentCluster)) {
      assign(params, {
        clustersType: currentCluster.clusterType,
        clustersId: currentCluster.clusterId
      });
    }
    this.usersApiService.getUsingGET2(params).subscribe(res => {
      this.drawModalService.openDetailModal(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvModalKey: 'user-detail',
          lvWidth: MODAL_COMMON.normalWidth + 50,
          lvClass: 'user-detail-header',
          lvHeader: user.userName,
          lvContent: UserdetailComponent,
          lvComponentParams: {
            user: {
              ...res,
              name: user.userName,
              login: user.login,
              lock: user.lock,
              optItems: this.getOptItems(user)
            }
          },
          lvFooter: [
            {
              label: this.closeLabel,
              onClick: modal => {
                modal.close();
              }
            }
          ],
          lvAfterOpen: modal => {},
          lvOk: () => {}
        })
      );
    });
  }

  queryAssociatedUsers(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('system_role_associatedusers_label'),
        lvContent: AssociatedusersComponent,
        lvComponentParams: {
          role: data
        },
        lvFooter: [
          {
            id: 'close',
            label: this.closeLabel,
            onClick: modal => {
              modal.close();
            }
          }
        ]
      })
    );
  }

  userPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.initUser(this.sortSources);
  }
}

@Component({
  selector: 'aui-warning',
  template: `
    <div class="warning-content">
      <span [innerHTML]="content"></span>
    </div>

    <div class="warning-checkbox">
      <label
        class="aui-gutter-column-sm"
        *ngIf="showDeleteResource"
        lv-checkbox
        [(ngModel)]="deleteResource"
        >{{ i18n.get('system_delete_saml_user_tips_label') }}</label
      >
      <label
        lv-checkbox
        [(ngModel)]="status"
        (ngModelChange)="warningConfirmChange($event)"
        >{{ i18n.get('common_warning_confirm_label') }}</label
      >
    </div>
  `,
  styles: [
    `
      .warning-content {
        max-height: 240px;
        overflow: auto;
      }
    `
  ]
})
export class WarningComponent implements OnInit {
  status;
  content;
  userRole;
  deleteResource = false;
  dataMap = DataMap;
  showDeleteResource = false;
  isChecked$ = new Subject<boolean>();
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    if (!this.isOceanProtect) {
      this.showDeleteResource =
        !includes(
          ['Role_Auditor', 'Role_RD_Admin', 'Role_DR_Admin'],
          this.userRole
        ) &&
        !includes(
          [
            DataMap.Deploy_Type.cyberengine.value,
            DataMap.Deploy_Type.hyperdetect.value
          ],
          this.i18n.get('deploy_type')
        );
    } else {
      this.showDeleteResource = !intersection(
        ['Role_Auditor', 'Role_RD_Admin', 'Role_DR_Admin', 'Role_SYS_Admin'],
        this.userRole
      ).length;
    }
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }
}

@NgModule({
  imports: [FormsModule, CheckboxModule, BaseModule],
  declarations: [WarningComponent],

  exports: [WarningComponent]
})
export class WarningModule {}
