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
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  DataMap,
  I18NService,
  UserRoleI18nMap,
  UserRoleType
} from 'app/shared';
import { ModalRef } from '@iux/live';
import { includes, isEmpty } from 'lodash';

@Component({
  selector: 'cdm-userdetail',
  templateUrl: './userdetail.component.html'
})
export class UserdetailComponent implements OnInit {
  user;
  userRoleI18nMap = UserRoleI18nMap;
  userRoleType = UserRoleType;
  dataMap = DataMap;
  eyeIcon = 'aui-icon-eye-close';
  dynamicCodeEmail;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;
  constructor(public i18n: I18NService, private modal: ModalRef) {}

  ngOnInit() {
    this.getModalHeader();
    this.getDynamicCodeEmail();
    this.getDescription();
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  getDynamicCodeEmail() {
    this.dynamicCodeEmail = this.maskEmail(this.user.dynamicCodeEmail);
  }

  getDescription() {
    this.user.description =
      this.user.defaultUser &&
      includes(
        ['sysadmin', 'mmdp_admin', 'mm_audit', 'cluster_admin'],
        this.user.description
      )
        ? this.i18n.get(`common_${this.user.description}_label`)
        : this.user.description;
  }

  changeEmail() {
    if (this.eyeIcon === 'aui-icon-eye-open') {
      this.eyeIcon = 'aui-icon-eye-close';
      this.dynamicCodeEmail = this.maskEmail(this.user.dynamicCodeEmail);
    } else {
      this.eyeIcon = 'aui-icon-eye-open';
      this.dynamicCodeEmail = this.user.dynamicCodeEmail;
    }
  }

  maskEmail(email) {
    if (isEmpty(email)) {
      return '';
    }
    const prefixStr = email.split('@')[0];
    return prefixStr.slice(0, 3) + '***' + '@' + email.split('@')[1];
  }
}
