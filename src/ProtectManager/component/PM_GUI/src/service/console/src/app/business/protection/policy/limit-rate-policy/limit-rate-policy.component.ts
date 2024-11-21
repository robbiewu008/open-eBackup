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
  OnInit
} from '@angular/core';
import {
  CommonConsts,
  I18NService,
  MODAL_COMMON,
  ProtectResourceAction,
  QosService,
  WarningMessageService,
  getPermissionMenuItem,
  CookieService,
  OperateItems,
  LANGUAGE,
  GROUP_COMMON,
  RoleOperationMap,
  hasSpeedLimitPermission
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, trim } from 'lodash';
import { CreateLimitRatePolicyComponent } from './create-limit-rate-policy/create-limit-rate-policy.component';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-limit-rate-policy',
  templateUrl: './limit-rate-policy.component.html',
  styleUrls: ['./limit-rate-policy.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class LimitRatePolicyComponent implements OnInit {
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  policyName;
  qosData = [];
  qosSelection = [];
  language = LANGUAGE;

  groupCommon = GROUP_COMMON;

  roleOperationMap = RoleOperationMap;
  isDataBackup = this.appUtilsService.isDataBackup;

  constructor(
    public i18n: I18NService,
    private drawmodalservice: DrawModalService,
    private qosServiceApi: QosService,
    private warningMessageService: WarningMessageService,
    private cookieService: CookieService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService
  ) {}

  policyAction(actionType, item?) {
    this.drawmodalservice.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'create-qos-moadl',
        lvHeader:
          actionType === ProtectResourceAction.Create
            ? this.i18n.get('protection_create_limit_rate_policy_label')
            : this.i18n.get('common_modify_label'),
        lvWidth: MODAL_COMMON.smallModal,
        lvContent: CreateLimitRatePolicyComponent,
        lvOkDisabled: actionType === ProtectResourceAction.Create,
        lvComponentParams: {
          rowItem: actionType === ProtectResourceAction.Create ? null : item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateLimitRatePolicyComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateLimitRatePolicyComponent;
            if (actionType === ProtectResourceAction.Create) {
              content.create().subscribe({
                next: () => {
                  resolve(true);
                  this.getQos();
                },
                error: () => resolve(false)
              });
            } else {
              content.modify().subscribe({
                next: () => {
                  resolve(true);
                  this.getQos();
                },
                error: () => resolve(false)
              });
            }
          });
        }
      })
    );
  }

  createQos() {
    this.policyAction(ProtectResourceAction.Create);
  }

  deleteQos(items) {
    const uuids = [];
    const names = [];
    each(items, v => {
      uuids.push(v.uuid);
      names.push(v.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get('protection_delete_qos_label', [names.join(',')]),
      onOK: () => {
        this.qosServiceApi
          .deleteQosV1QosDelete({ body: uuids })
          .subscribe(res => {
            this.getQos();
          });
      }
    });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getQos();
  }

  optsCallback = item => {
    const menus = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyQos,
        disabled: !hasSpeedLimitPermission(item),
        onClick: () => this.policyAction(ProtectResourceAction.Modify, item)
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteQos,
        disabled: !hasSpeedLimitPermission(item),
        onClick: () => this.deleteQos([item])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  searchByName(name) {
    this.policyName = name;
    this.getQos();
  }

  getQos() {
    this.qosSelection = [];
    const params = { pageNo: this.pageIndex, pageSize: this.pageSize };
    if (this.policyName) {
      assign(params, {
        conditions: JSON.stringify({ name: trim(this.policyName) })
      });
    }
    this.qosServiceApi.queryResourcesV1QosGet(params).subscribe(
      res => {
        this.qosData = res.items;
        this.total = res.total;
        this.cdr.detectChanges();
      },
      err => {
        this.qosData = [];
        this.total = 0;
        this.cdr.detectChanges();
      }
    );
  }

  ngOnInit() {
    this.getQos();
    this.virtualScroll.getScrollParam(200, 3);
  }

  onChange() {
    this.ngOnInit();
  }
}
