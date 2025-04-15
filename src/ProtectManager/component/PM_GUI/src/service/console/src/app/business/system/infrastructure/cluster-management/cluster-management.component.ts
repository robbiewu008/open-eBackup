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
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CookieService,
  DataMapService,
  I18NService,
  WarningMessageService,
  DataMap
} from 'app/shared';
import {
  ApiMultiClustersService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, filter } from 'lodash';
import { TargetClusterComponent } from './target-cluster/target-cluster.component';
import { BackupClusterComponent } from './backup-cluster/backup-cluster.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Pipe({ name: 'selecTable' })
export class SelectionPipe implements PipeTransform {
  constructor(private dataMapService: DataMapService) {}
  transform(value: any[], exponent: string = 'role') {
    return filter(
      value,
      item => item[exponent] !== DataMap.Target_Cluster_Role.primaryNode.value
    );
  }
}

@Component({
  selector: 'app-modify-cluster',
  template: `
    <lv-form
      [formGroup]="modifyFormGroup"
      class="formGroup"
      [lvLabelColon]="false"
    >
      <lv-form-item>
        <lv-form-label>{{ 'common_type_label' | i18n }} </lv-form-label>
        <lv-form-control>
          {{ 'system_local_user_label' | i18n }}
        </lv-form-control>
      </lv-form-item>
      <lv-form-item>
        <lv-form-label
          >{{ usernameLabel
          }}<i
            lv-icon="aui-icon-help"
            lv-tooltip="{{ 'system_storage_user_help_label' | i18n }}"
            lvTooltipPosition="rightTop"
            lvTooltipTheme="light"
            class="configform-constraint"
            lvColorState="true"
          ></i>
        </lv-form-label>
        <lv-form-control>
          admin
        </lv-form-control>
      </lv-form-item>
      <lv-form-item>
        <lv-form-label>{{ 'common_role_label' | i18n }} </lv-form-label>
        <lv-form-control>
          {{ 'system_administrator_label' | i18n }}
        </lv-form-control>
      </lv-form-item>
      <lv-form-item>
        <lv-form-label lvRequired>{{ passwordLabel }}</lv-form-label>
        <lv-form-control [lvErrorTip]="ipPwdErrorTip">
          <input
            lv-input
            type="password"
            formControlName="password"
            [lvPasteAllowed]="false"
            autocomplete="new-password"
          />
        </lv-form-control>
      </lv-form-item>
    </lv-form>
  `
})
export class ModifyClusterModalComponent implements OnInit {
  modifyFormGroup: FormGroup;
  usernameLabel = this.i18n.get('common_username_label');
  passwordLabel = this.i18n.get('common_password_label');
  ipPwdErrorTip = assign(
    {
      invalidMinLength: this.i18n.get('common_valid_minlength_label', [8]),
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
    },
    this.baseUtilService.requiredErrorTip
  );

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.modifyFormGroup = this.fb.group({
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(8),
          this.baseUtilService.VALID.maxLength(64)
        ],
        updateOn: 'change'
      })
    });
  }
}

@Component({
  selector: 'aui-cluster-management',
  templateUrl: './cluster-management.component.html',
  styles: [
    `
      .local-refresh {
        justify-content: flex-end;
      }
    `
  ],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClusterManagementComponent implements OnInit {
  activeIndex: string = 'backup';
  showDistributedCluster =
    this.appUtilsService.isDistributed || this.appUtilsService.isDecouple;

  @ViewChild(TargetClusterComponent, { static: false })
  targetclusterComponent: TargetClusterComponent;

  @ViewChild(BackupClusterComponent, { static: false })
  backupclusterComponent: BackupClusterComponent;

  constructor(
    public appUtilsService: AppUtilsService,
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public clusterApiService: ClustersApiService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public infoMessageService: InfoMessageService,
    private cookieService: CookieService,
    public virtualScroll?: VirtualScrollService,
    private cdr?: ChangeDetectorRef,
    private multiClustersServiceApi?: ApiMultiClustersService
  ) {}

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 'target') {
      this.targetclusterComponent.ngOnInit();
    } else {
      this.backupclusterComponent.ngOnInit();
    }
  }
}
