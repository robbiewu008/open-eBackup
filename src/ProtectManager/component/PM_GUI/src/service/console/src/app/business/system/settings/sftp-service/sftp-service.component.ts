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
import { ChangeDetectorRef, Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageboxService, MessageService } from '@iux/live';
import {
  BackupClustersApiService,
  BaseUtilService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  ColorConsts,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import {
  SftpManagerApiService,
  SystemApiService
} from 'app/shared/api/services';
import { ProcessLoadingComponent } from 'app/shared/components/process-loading/process-loading.component';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNull,
  map,
  map as _map,
  set,
  size
} from 'lodash';
import { AddUserComponent } from './add-user/add-user.component';
import { ChangePasswordComponent } from './change-password/change-password.component';
import { StartSftpComponent } from './start-sftp/start-sftp.component';
import { ThresholdModifyComponent } from './threshold-modify/threshold-modify.component';
import { UserDetailComponent } from './user-detail/user-detail.component';

@Component({
  selector: 'aui-sftp-service',
  templateUrl: './sftp-service.component.html',
  styleUrls: ['./sftp-service.component.less'],
  providers: [CapacityCalculateLabel]
})
export class SftpServiceComponent implements OnInit {
  dataMap = DataMap;
  formGroup: FormGroup;
  wormGroup: FormGroup;
  sftpIp;
  abStatus;
  sftpStatus;
  queryName;
  usersData = [];
  selection = [];
  _isNull = isNull;
  isLoading = false;
  loading = false;
  clickDisabled = false;
  sftpSwitch = false;
  sftpDisable = false;
  disablePort = false;
  unitconst = CAPACITY_UNIT;
  colorConsts = ColorConsts;
  deleteBtnDisabled = false;
  total = CommonConsts.PAGE_TOTAL;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  standard_Service_Status = DataMap.Standard_Service_Status;
  progressBarColor = [[0, ColorConsts.NORMAL]];
  enableLabel = this.i18n.get('common_enable_label');
  disableLabel = this.i18n.get('common_disable_label');
  switchOffContent = this.i18n.get('system_sftp_switch_off_tip_label');
  columns = [
    {
      key: 'username',
      label: this.i18n.get('common_username_label')
    },
    {
      key: 'limitSpaceQuota',
      label: this.i18n.get('system_sftp_quota_label')
    }
  ];
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  nodeName = '';
  clusterMenus = [];
  activeNode;
  modifying = false;

  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild(ProcessLoadingComponent, { static: false })
  processLoadingComponent: ProcessLoadingComponent;

  constructor(
    private cdr: ChangeDetectorRef,
    private BackupClustersApiService: BackupClustersApiService,
    public fb: FormBuilder,
    public i18n: I18NService,
    public messageBox: MessageboxService,
    public cookieService: CookieService,
    public dataMapService: DataMapService,
    public messageService: MessageService,
    public baseUtilService: BaseUtilService,
    public drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    public systemApiService: SystemApiService,
    public batchOperateService: BatchOperateService,
    public sftpManagerApiService: SftpManagerApiService,
    public warningMessageService: WarningMessageService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    if (this.isDataBackup) {
      this.getClusterNodes();
    } else {
      this.getSftpData();
    }
    this.virtualScroll.getScrollParam(260, 3);
    this.setClusterMenuHeight();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(260, 3);
      this.setClusterMenuHeight();
    });
  }

  setClusterMenuHeight() {
    defer(() => {
      const clusterMenu = first(
        document.getElementsByClassName('cluster-menus')
      );
      if (clusterMenu) {
        clusterMenu.setAttribute(
          'style',
          `max-height: ${parseInt(this.virtualScroll.scrollParam.y) + 30}px`
        );
      }
    });
  }

  //获取节点
  getClusterNodes(nodeName?) {
    const params = {};
    if (nodeName) {
      set(params, 'clusterName', nodeName || '');
    }

    this.BackupClustersApiService.queryAllMembers(params).subscribe(res => {
      if (!res.length) {
        this.clickDisabled = true;
      } else {
        this.clickDisabled = false;
      }

      // 排序规则：
      // 第一层:按照节点角色：主节点、备、成员节点
      // 第二层：按照节点状态：在线、设置中、离线、删除中
      const rule = [
        DataMap.Node_Status.online.value,
        DataMap.Node_Status.setting.value,
        DataMap.Node_Status.offline.value,
        DataMap.Node_Status.deleting.value
      ];
      res.sort((a, b) => {
        if (a.role !== b.role) {
          return b.role - a.role;
        } else {
          return rule.indexOf(a.status) - rule.indexOf(b.status);
        }
      });
      this.clusterMenus = map(res, item => {
        return {
          label: item.clusterName,
          id: get(item, 'remoteEsn'),
          ...item,
          disabled: item.status !== DataMap.Cluster_Status.online.value
        };
      });
      this.activeNode = get(this.clusterMenus, '[0].id');
      this.getSftpData(this.activeNode);
      this.cdr.detectChanges();
    });
  }

  getCurrentNodeName() {
    return find(this.clusterMenus, { id: this.activeNode })?.clusterName || '';
  }

  onChange() {
    this.sftpSwitch = false;
    this.ngOnInit();
  }

  getServiceStatus() {
    setTimeout(() => {
      this.processLoadingComponent.getStatus();
    }, 1);
  }
  search() {
    this.getClusterNodes(this.nodeName);
  }

  refresh() {
    this.getClusterNodes(this.nodeName);
  }

  nodeChange(event) {
    this.activeNode = event.data.id;
    this.getSftpData(event.data.id);
  }

  getSftpData(node?) {
    const memberEsn = node || this.activeNode;
    this.sftpManagerApiService
      .queryServiceUsingGET({ memberEsn: memberEsn })
      .subscribe(res => {
        this.sftpIp = res.ip;
        this.sftpStatus = res.status;
        this.sftpSwitch = res.status === 1;
        if (this.sftpStatus === 1) {
          this.refreshUser(memberEsn);
        }
      });
  }

  refreshUser(memberEsn?) {
    const node = memberEsn || this.activeNode;
    this.queryName = '';
    this.selection = [];
    this.getUsers(node);
  }

  getUsers(memberEsn?) {
    const params = {
      pageNum: this.pageIndex + 1,
      pageSize: this.pageSize
    };
    if (!isEmpty(this.queryName)) {
      assign(params, {
        username: this.queryName
      });
    }
    set(params, 'memberEsn', memberEsn);
    this.sftpManagerApiService
      .queryNodeSftpByPageUsingGET(params)
      .subscribe(res => {
        this.usersData = res.users;
        this.total = res.count;
      });
  }

  optsCallback = data => {
    return this.getOptsItems(data);
  };

  getOptsItems(user) {
    const menus = [
      {
        id: 'modifyThreshold',
        disabled: false,
        label:
          this.i18n.get('common_modify_label') +
          this.i18n.get('common_threshold_label'),
        permission: OperateItems.ModifySFTPPassword,
        onClick: () => this.modifyThreshold(user)
      },
      {
        id: 'changePassword',
        disabled: false,
        label: this.i18n.get('common_update_password_label'),
        permission: OperateItems.ModifySFTPPassword,
        onClick: () => this.changePassword(user)
      },
      {
        id: 'delete',
        disabled: false,
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteSFTPUser,
        onClick: () => this.deleteUsers([user])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }
  modifyThreshold(data) {
    const node = this.activeNode || '';
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader:
        this.i18n.get('common_modify_label') +
        this.i18n.get('common_threshold_label'),
      lvContent: ThresholdModifyComponent,
      lvWidth: MODAL_COMMON.smallWidth,
      lvComponentParams: {
        data,
        node
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const component = modal.getContentComponent() as ThresholdModifyComponent;
        const modalIns = modal.getInstance();
        component.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent() as ThresholdModifyComponent;
          component.onOK().subscribe(
            res => {
              resolve(true);
              this.getUsers(node);
            },
            err => resolve(false)
          );
        });
      }
    });
  }

  changePassword(user) {
    const node = this.activeNode || '';
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader:
          this.i18n.get('common_update_password_label', [], true) +
          user.username,
        lvModalKey: 'change-sftp-pwd',
        lvContent: ChangePasswordComponent,
        lvComponentParams: {
          user,
          node
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ChangePasswordComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result === 'INVALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ChangePasswordComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.getUsers(node);
              },
              error => {
                resolve(false);
              }
            );
          });
        }
      })
    );
  }

  onComplete(isOk) {
    this.modifying = false;
    this.loading = !isOk;
    this.isLoading = !isOk;
    this.sftpSwitch = isOk;
    this.sftpSwitch && this.getSftpData();
  }

  onTetry() {
    this.enableChange();
  }

  configSftp(type) {
    this.loading = true;
    const isModify = type === 'modify';
    this.sftpManagerApiService
      .queryServiceUsingGET({
        memberEsn: this.activeNode || ''
      })
      .subscribe((res: any) => {
        this.drawModalService.create(
          assign({}, MODAL_COMMON.drawerOptions, {
            lvModalKey: 'startSftp',
            lvHeader: isModify
              ? this.i18n.get('system_modify_sftp_label')
              : this.i18n.get('system_start_sftp_label'),
            lvWidth: MODAL_COMMON.xLargeWidth - 50,
            lvContent: StartSftpComponent,
            lvOkDisabled: isEmpty(res.ip),
            lvComponentParams: {
              data: res,
              activeNode: this.activeNode,
              isModify
            },
            lvOk: modal => {
              return new Promise(resolve => {
                if (isModify) {
                  this.warningMessageService.create({
                    content: this.i18n.get('system_modify_sftp_tip_label'),
                    width: 500,
                    onOK: () => {
                      const content = modal.getContentComponent() as StartSftpComponent;
                      content.onOK().subscribe(
                        () => {
                          resolve(true);
                          this.modifying = true;
                          this.isLoading = true;
                          defer(() => this.processLoadingComponent.getStatus());
                        },
                        () => {
                          resolve(false);
                          this.loading = false;
                        }
                      );
                    },
                    onCancel: () => resolve(false),
                    lvAfterClose: result => {
                      if (result && result.trigger === 'close') {
                        resolve(false);
                      }
                    }
                  });
                } else {
                  const content = modal.getContentComponent() as StartSftpComponent;
                  content.onOK().subscribe(
                    () => {
                      resolve(true);
                      this.isLoading = true;
                      defer(() => this.processLoadingComponent.getStatus());
                    },
                    () => {
                      resolve(false);
                      if (res.ip) {
                        this.loading = false;
                        this.sftpSwitch = false;
                      } else {
                        this.loading = false;
                        this.isLoading = false;
                      }
                    }
                  );
                }
              });
            },
            lvCancel: () => {
              this.loading = false;
            },
            lvAfterClose: modal => {
              if (modal && modal.trigger === 'close') {
                this.loading = false;
              }
            }
          })
        );
        return;
      });
  }

  enableChange() {
    if (this.loading) {
      return;
    }
    if (!this.sftpSwitch) {
      this.configSftp('start');
    } else {
      this.warningMessageService.create({
        content: this.switchOffContent,
        onOK: () => {
          this.sftpManagerApiService
            .switchSftpStatusUsingPUT({
              SftpSwitchRequest: {
                status: 0,
                ip: this.sftpIp
              },
              memberEsn: this.activeNode || ''
            })
            .subscribe(res => {
              this.isLoading = false;
              this.getSftpData();
            });
        }
      });
    }
  }

  getDetail(user) {
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'sftp-user-detail',
      lvHeader: user.username,
      lvContent: UserDetailComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        userId: user.id,
        node: this.activeNode || ''
      },
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  searchByName() {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    this.getUsers(this.activeNode);
  }

  addUser() {
    const node = this.activeNode || '';
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('system_sftp_add_user_label'),
        lvContent: AddUserComponent,
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddUserComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result !== 'VALID';
          });
        },
        lvComponentParams: {
          node
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddUserComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.getUsers(node);
              },
              error => {
                resolve(false);
              }
            );
          });
        }
      })
    );
  }

  deleteUsers(userList) {
    const userNames = [];
    userList.forEach(user => {
      userNames.push(user.username);
    });
    this.warningMessageService.create({
      content: this.i18n.get('system_authorized_user_delete_label', [
        userNames.toString()
      ]),
      onOK: () => {
        if (size(userList) === 1) {
          const user = userList[0];
          this.sftpManagerApiService
            .deleteUserUsingDELETE({
              userList: [{ userId: user.id, username: user.username }],
              akOperationTips: false,
              memberEsn: this.activeNode || ''
            })
            .subscribe(res => {
              if (res.failCount && res.results[0] && res.results[0].errorCode) {
                this.messageService.error(
                  this.i18n.get(res.results[0].errorCode),
                  {
                    lvMessageKey: 'lvMsg_key_delete_sftp_ex',
                    lvShowCloseButton: true
                  }
                );
              } else {
                this.messageService.success(
                  this.i18n.get('common_operate_success_label'),
                  {
                    lvShowCloseButton: true
                  }
                );
              }
              this.refreshUser();
            });
          return;
        }

        this.batchOperateService.selfGetResults(
          item => {
            return this.sftpManagerApiService.deleteUserUsingDELETE({
              userList: [{ userId: item.id, username: item.username }],
              akDoException: false,
              akOperationTips: false,
              akLoading: false,
              memberEsn: this.activeNode || ''
            });
          },
          _map(cloneDeep(this.selection), item => {
            return assign(item, {
              name: item.username,
              isAsyn: false
            });
          }),
          () => {
            this.refreshUser();
          },
          '',
          true
        );
      }
    });
  }

  pageChange(source) {
    this.pageIndex = source.pageIndex;
    this.pageSize = source.pageSize;
    this.getUsers(this.activeNode);
  }
}
