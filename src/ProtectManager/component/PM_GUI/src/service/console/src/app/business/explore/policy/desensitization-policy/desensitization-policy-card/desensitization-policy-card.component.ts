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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  CookieService,
  DataMap,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  PolicyControllerService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { assign, isEmpty } from 'lodash';
import { combineLatest } from 'rxjs';
import { CreateDesensitizationPolicyComponent } from '../desensitization-policy-list/create-desensitization-policy/create-desensitization-policy.component';
import { RelatedObjectComponent } from '../related-object/related-object.component';

@Component({
  selector: 'aui-desensitization-policy-card',
  templateUrl: './desensitization-policy-card.component.html',
  styleUrls: ['./desensitization-policy-card.component.less']
})
export class DesensitizationPolicyCardComponent implements OnInit {
  @Input() showOptItems: boolean;
  @Input() cardItem;
  @Input() isChecked;
  @Output() onCardChange = new EventEmitter<any>();
  dataMap = DataMap;
  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private policyManagerApiService: PolicyControllerService,
    private cookieService: CookieService,
    private messageBox: MessageboxService,
    private infoMessageService: InfoMessageService
  ) {}

  optsCallback = data => {
    const menus = [
      {
        id: 'modify',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        permission: OperateItems.ModifyDesensitizationPolicy,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_modify_label'),
        onClick: () => this.create(data)
      },
      {
        id: 'delete',
        hidden:
          data.create_method ===
          DataMap.Senesitization_Create_Method.preset.value,
        permission: OperateItems.DeleteDesensitizationPolicy,
        disabled: data.ref_num > 0,
        label: this.i18n.get('common_delete_label'),
        onClick: () => this.delete(data)
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.CloneDesensitizationPolicy,
        onClick: () => this.create(data, true)
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  optsClick($event) {
    $event.stopPropagation
      ? $event.stopPropagation()
      : (window.event.cancelBubble = true);
  }

  create(data?, isClone?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isEmpty(data)
        ? this.i18n.get('explore_sensitive_policy_create_label')
        : isClone
        ? this.i18n.get('common_clone_label')
        : this.i18n.get('common_modify_label'),
      lvContent: CreateDesensitizationPolicyComponent,
      lvOkDisabled: isEmpty(data),
      lvWidth: MODAL_COMMON.largeWidth,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateDesensitizationPolicyComponent;
        const combined: any = combineLatest(
          content.formGroup.statusChanges,
          content.valid$
        );
        combined.subscribe(latestValues => {
          const [formGroupStatus, valid$] = latestValues;
          modal.lvOkDisabled = !(formGroupStatus === 'VALID' && valid$);
        });
        if (data) {
          content.formGroup.get('name').markAsTouched();
          content.formGroup.get('name').updateValueAndValidity();
          content.formGroup.get('description').updateValueAndValidity();
          content.valid$.next(true);
        }
      },
      lvComponentParams: {
        rowItem: assign({}, data, { isClone })
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateDesensitizationPolicyComponent;
          content.create().subscribe(
            res => {
              resolve(true);
              this.onCardChange.emit();
            },
            error => resolve(false)
          );
        });
      }
    });
  }

  delete(data) {
    this.infoMessageService.create({
      content: this.i18n.get('explore_desensitization_delete_label', [
        this.i18n.get('common_desensitization_policy_label'),
        this.i18n.get('common_desensitization_policy_label'),
        this.i18n.get('common_desensitization_policy_label')
      ]),
      onOK: () => {
        this.policyManagerApiService
          .deletePolicyUsingDELETE({
            id: data.id
          })
          .subscribe(res => this.onCardChange.emit());
      }
    });
  }

  getRelNum($event, item) {
    $event.stopPropagation
      ? $event.stopPropagation()
      : (window.event.cancelBubble = true);
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_associate_database_label'),
      lvContent: RelatedObjectComponent,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        rowItem: item
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }

  ngOnInit() {}
}
