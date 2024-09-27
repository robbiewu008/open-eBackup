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
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  CookieService,
  DataMapService,
  extendDesesitizationInfo,
  extendSlaInfo,
  getPermissionMenuItem,
  I18NService,
  ProtectedResourceApiService,
  WarningMessageService
} from 'app/shared';
import {
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  OperateItems
} from 'app/shared/consts';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import {
  assign,
  difference,
  filter,
  size,
  trim,
  includes,
  find,
  isEmpty
} from 'lodash';
import { ChooseDensensitizationPolicyComponent } from '../choose-densensitization-policy/choose-densensitization-policy.component';
import { AnonymizationVerificateComponent } from '../start-densensitization/anonymization-verificate/anonymization-verificate.component';
import { StartDensensitizationComponent } from '../start-densensitization/start-densensitization.component';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { DesensitizationPolicyDetailComponent } from '../../policy/desensitization-policy/desensitization-policy-list/desensitization-policy-detail/desensitization-policy-detail.component';
import { MessageboxService } from '@iux/live';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-data-desensitization-list',
  templateUrl: './data-desensitization-list.component.html',
  styleUrls: ['./data-desensitization-list.component.less']
})
export class DataDesensitizationListComponent implements OnInit {
  ip;
  name;
  instanceNames;
  policyNames;
  columns = [];
  tableData = [];
  filterParams: any = {};
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  dataMap = DataMap;
  filterMap = {
    identification_status: 'identificationStatus',
    desesitization_status: 'desesitizationStatus'
  };

  @Input() header;
  @Input() resourceType;
  @Input() childResourceType;

  @ViewChild('identifiedTpl', { static: false }) identifiedTpl;
  @Output() onStatusChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private detailService: ResourceDetailService,
    private cookieService: CookieService,
    private warningMessageService: WarningMessageService,
    private messageBox: MessageboxService,
    public virtualScroll: VirtualScrollService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getColumns();
    this.virtualScroll.getScrollParam(220);
  }

  getResource() {
    this.onStatusChange.emit();
  }

  onChange() {
    this.getResource();
  }

  getColumns() {
    this.columns = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'environment_endpoint',
        label: this.i18n.get('common_ip_address_label')
      },
      {
        label: this.i18n.get('protection_instance_name_label'),
        key: 'instance_names'
      },
      {
        key: 'identification_status',
        filter: true,
        label: this.i18n.get('explore_identification_status_label'),
        filterMap: this.dataMapService.toArray('Identification_Status')
      },
      {
        key: 'desesitization_status',
        filter: true,
        label: this.i18n.get('explore_desensitization_status_label'),
        filterMap: this.dataMapService.toArray('Desensitization_Status')
      }
    ];

    this.columns = filter(this.columns, column => {
      return !(
        column.resourceType &&
        size(difference(this.childResourceType, column.resourceType)) ===
          size(this.childResourceType)
      );
    });
  }

  filterChange(e) {
    if (!isEmpty(e.value)) {
      assign(this.filterParams, {
        resourceDesesitization: {
          ...this.filterParams.resourceDesesitization,
          [this.filterMap[e.key]]: [['in'], ...e.value]
        }
      });
    } else {
      if (this.filterParams.resourceDesesitization) {
        delete this.filterParams.resourceDesesitization[this.filterMap[e.key]];
      }
    }
    this.getResource();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getResource();
  }

  searchByName(name) {
    if (trim(name)) {
      assign(this.filterParams, {
        name: [['~~'], trim(name)]
      });
    } else {
      delete this.filterParams.name;
    }
    this.getResource();
  }

  searchByIp(ip) {
    if (trim(ip)) {
      assign(this.filterParams, {
        environment: {
          endpoint: [['~~'], trim(ip)]
        }
      });
    } else {
      delete this.filterParams.environment;
    }

    this.getResource();
  }

  searchByInstanceNames(instanceNames) {
    if (trim(instanceNames)) {
      assign(this.filterParams, {
        inst_name: [['~~'], trim(instanceNames)]
      });
    } else {
      delete this.filterParams.inst_name;
    }

    this.getResource();
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(data) {
    const menus = [
      {
        id: 'choosePolicy',
        disabled: !(
          (includes(
            [
              DataMap.Identification_Status.identified.value,
              DataMap.Identification_Status.not_identified.value,
              DataMap.Identification_Status.failed_identified.value,
              DataMap.Identification_Status.stopped.value
            ],
            data.identification_status
          ) &&
            DataMap.Desensitization_Status.not_desesitize.value ===
              data.desesitization_status) ||
          (data.identification_status ===
            DataMap.Identification_Status.identified.value &&
            data.desesitization_status ===
              DataMap.Desensitization_Status.desesitized.value)
        ),
        label: this.i18n.get('explore_sensitive_data_identified_label'),
        permission: OperateItems.IdentitySensitiveData,
        onClick: () => this.sensitivDataIdentified(data)
      },
      {
        id: 'startDesensitize',
        disabled: !(
          data.identification_status ===
            DataMap.Identification_Status.identified.value &&
          includes(
            [
              DataMap.Desensitization_Status.not_desesitize.value,
              DataMap.Desensitization_Status.failed_desesitized.value,
              DataMap.Desensitization_Status.stopped.value
            ],
            data.desesitization_status
          )
        ),
        label: this.i18n.get('explore_start_desensitize_label'),
        permission: OperateItems.StartDataAnonymization,
        onClick: () =>
          this.startDesensitize(
            data,
            this.i18n.get('explore_start_desensitize_label')
          )
      },
      {
        id: 'checkResult',
        disabled: !(
          data.identification_status !==
            DataMap.Identification_Status.identifing.value &&
          data.desesitization_status ===
            DataMap.Desensitization_Status.desesitized.value
        ),
        label: this.i18n.get('explore_check_desensitization_label'),
        permission: OperateItems.CheckAnonymization,
        onClick: () =>
          this.startDesensitize(
            assign({}, data, { isCheckResult: true }),
            this.i18n.get('common_data_desensitization_report_label')
          )
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  getResourceDetail(item) {
    const params = {
      pageNo: 0,
      pageSize: 1,
      queryDependency: true
    };
    this.protectedResourceApiService
      .ListResources(
        assign(params, {
          conditions: JSON.stringify({
            uuid: item.uuid,
            isDesesitization: true
          })
        })
      )
      .subscribe(
        res => {
          const info = res.records[0];
          assign(info, {
            link_status: info.extendInfo.linkStatus,
            ip: item.path || item.endpoint,
            sub_type: info.subType
          });
          this.protectedResourceApiService
            .ListResources(
              assign(params, {
                conditions: JSON.stringify({
                  uuid: item.uuid
                })
              })
            )
            .subscribe(item => {
              info.protectedObject = item.records[0].protectedObject;
              extendSlaInfo(info);
              extendDesesitizationInfo(info);
              this.detailService.openDetailModal(info.subType, {
                data: assign(info, { optItems: this.getOptItems(info) })
              });
            });
        },
        () => {
          this.detailService.openDetailModal(item.subType, {
            data: assign(item, { optItems: this.getOptItems(item) })
          });
        }
      );
  }

  getPolicyDetail(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: item.desesitization_policy_name,
      lvContent: DesensitizationPolicyDetailComponent,
      lvWidth: MODAL_COMMON.largeWidth,
      lvComponentParams: {
        rowItem: {
          id: item.desesitization_policy_id,
          name: item.desesitization_policy_name
        }
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

  sensitivDataIdentified(item) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_sensitive_data_identified_label'),
      lvContent: ChooseDensensitizationPolicyComponent,
      lvOkDisabled: !item.desesitization_policy_id,
      lvWidth: MODAL_COMMON.normalWidth + 200,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ChooseDensensitizationPolicyComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        rowItem: item
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as ChooseDensensitizationPolicyComponent;
        const policyName = find(content.policyData, {
          id: content.selectedPolicy
        })['name'];
        this.verificateDataBase(
          assign({}, item, {
            policy_id: content.selectedPolicy,
            policy_name: policyName
          }),
          modal,
          () => {
            this.getResource();
          }
        );
        return false;
      }
    });
  }

  verificateDataBase(item, oldModal, callback: () => void) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_auth_label'),
      lvContent: AnonymizationVerificateComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvComponentParams: {
        rowItem: item
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AnonymizationVerificateComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AnonymizationVerificateComponent;
          this.messageBox.info({
            lvContent: this.identifiedTpl,
            lvFooter: [
              {
                label: this.i18n.get('common_ok_label'),
                onClick: modal => {
                  content.identify().subscribe(
                    res => {
                      resolve(true);
                      if (oldModal.close) {
                        oldModal.close();
                      }
                      callback();
                    },
                    error => resolve(false)
                  );
                  modal.close();
                }
              },
              {
                label: this.i18n.get('common_cancel_label'),
                onClick: modal => {
                  resolve(false);
                  modal.close();
                }
              }
            ],
            lvAfterClose: result => {
              if (result && result.trigger === 'close') {
                resolve(false);
              }
            }
          });
        });
      }
    });
  }

  startDesensitize(item, title?) {
    if (item.isCheckResult) {
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvHeader: title || this.i18n.get('common_data_desensitization_label'),
        lvContent: StartDensensitizationComponent,
        lvOkDisabled: false,
        lvWidth: MODAL_COMMON.largeWidth - 100,
        lvComponentParams: {
          rowItem: item
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      });
    } else {
      this.drawModalService.create({
        ...MODAL_COMMON.generateDrawerOptions(),
        lvHeader: title || this.i18n.get('common_data_desensitization_label'),
        lvContent: StartDensensitizationComponent,
        lvOkDisabled: false,
        lvWidth: MODAL_COMMON.largeWidth - 100,
        lvComponentParams: {
          rowItem: item
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as StartDensensitizationComponent;
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as StartDensensitizationComponent;
          if (item.isCheckResult) {
            return true;
          }
          content.desensitizate(modal, () => {
            this.getResource();
          });
          return false;
        }
      });
    }
  }

  trackByUuid = (index, item) => {
    return item.uuid;
  };
}
