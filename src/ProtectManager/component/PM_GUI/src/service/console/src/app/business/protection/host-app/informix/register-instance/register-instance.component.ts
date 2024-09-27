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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  InstanceType,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  findIndex,
  first,
  get,
  includes,
  isEqual,
  isNumber,
  map,
  remove,
  set,
  size,
  toNumber
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AddHostComponent } from './add-host/add-host.component';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-register-instance',
  templateUrl: './register-instance.component.html',
  styleUrls: ['./register-instance.component.less']
})
export class RegisterInstanceComponent implements OnInit {
  rowData;
  subType;
  optsConfig;
  hostOptions = [];
  originalTableData;
  dataMap = DataMap;
  namePlaceholder = this.i18n.get('protection_instance_name_tips_label');
  tableData = {
    data: [],
    total: 0
  };
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    })
    .filter(item => {
      return item.value === DataMap.Database_Auth_Method.os.value;
    });
  tableConfig: TableConfig;
  formGroup: FormGroup;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig();
    this.updateData();
    this.getHostOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      agents: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      authMode: new FormControl(DataMap.Database_Auth_Method.os.value),
      sqlhost: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      onconfig: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      children: new FormControl([])
    });

    this.watch();
  }

  updateData() {
    if (!this.rowData) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        const data = includes(
          [
            DataMap.Resource_Type.informixInstance.value,
            DataMap.Resource_Type.gbaseInstance.value
          ],
          res.subType
        )
          ? {
              name: res.name,
              type: DataMap.Instance_Type.single.value,
              agents: get(res, 'parentUuid'),
              sqlhost: get(res, 'extendInfo.sqlhostsPath'),
              onconfig: get(res, 'extendInfo.onconfigPath')
            }
          : {
              name: res.name,
              type: DataMap.Instance_Type.cluster.value,
              children: res['dependencies']['children']
            };
        this.formGroup.patchValue(data);
        this.formGroup.get('name').disable();

        if (
          includes(
            [
              DataMap.Resource_Type.informixClusterInstance.value,
              DataMap.Resource_Type.gbaseClusterInstance.value
            ],
            res.subType
          )
        ) {
          this.originalTableData = map(data?.children, item => {
            const host = get(item, 'dependencies.agents[0]');

            return {
              name: get(item, 'name'),
              host: get(host, 'name'),
              endpoint: get(host, 'endpoint'),
              linkStatus: toNumber(get(host, 'linkStatus')),
              parentUuid: get(item, 'environment.uuid'),
              auth: get(item, 'auth'),
              extendInfo: get(item, 'extendInfo'),
              dependencies: get(item, 'dependencies'),
              uuid: get(item, 'uuid')
            };
          });
          this.tableData = {
            data: cloneDeep(this.originalTableData),
            total: size(this.originalTableData)
          };
          this.formGroup.get('children').setValue(this.tableData.data);
        }
      });
  }

  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.namePlaceholder = this.i18n.get(
          'protection_instance_name_tips_label'
        );
        this.formGroup
          .get('agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('sqlhost')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(128)
          ]);
        this.formGroup
          .get('onconfig')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(128)
          ]);
        this.formGroup.get('children').clearValidators();
      } else {
        this.namePlaceholder = this.i18n.get('common_please_enter_label', [
          this.i18n.get('protection_instance_name_label')
        ]);
        this.formGroup.get('agents').clearValidators();
        this.formGroup.get('sqlhost').clearValidators();
        this.formGroup.get('onconfig').clearValidators();
        this.formGroup
          .get('children')
          .setValidators([
            this.baseUtilService.VALID.minLength(2),
            this.baseUtilService.VALID.required()
          ]);
      }

      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('sqlhost').updateValueAndValidity();
      this.formGroup.get('onconfig').updateValueAndValidity();
      this.formGroup.get('children').updateValueAndValidity();
    });
  }

  getHostOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType:
          this.subType === DataMap.Resource_Type.gbaseInstance.value
            ? DataMap.Resource_Type.gbaseCluster.value
            : DataMap.Resource_Type.informixService.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(recordsTemp, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
        return;
      }
      this.getHostOptions(recordsTemp, startPage);
    });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.add();
        },
        disableCheck: () => {
          return this.rowData || size(this.formGroup.value.children) === 2;
        }
      }
    ];

    this.optsConfig = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'host',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('protection_host_linkstatus_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_Host_LinkStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_Host_LinkStatus')
        }
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                onClick: data => {
                  this.add(first(data));
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_remove_label'),
                displayCheck: () => {
                  return !this.rowData;
                },
                onClick: data => {
                  this.delete(data);
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  add(data?) {
    if (this.formGroup.get('name').status === 'INVALID') {
      this.formGroup.get('name').markAsTouched();
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-host',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: !!data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_add_label'),
        lvContent: AddHostComponent,
        lvComponentParams: {
          hostOptions: this.hostOptions,
          name: this.formGroup.get('name').value,
          children: this.formGroup.value.children,
          rowData: data,
          subType: this.subType
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddHostComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });

          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddHostComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                const modifiedIndex = findIndex(
                  currentTableData,
                  item =>
                    get(item, 'parentUuid') ===
                    get(first(content.data), 'parentUuid')
                );
                if (modifiedIndex !== -1) {
                  currentTableData[modifiedIndex] = cloneDeep(
                    first(content.data)
                  );
                  currentTableData = [...currentTableData];
                } else {
                  currentTableData = currentTableData.concat([...content.data]);
                }

                this.formGroup.get('children').setValue(currentTableData);
                this.tableData = {
                  data: currentTableData,
                  total: size(currentTableData)
                };
              }
            });
          });
        }
      })
    );
  }

  delete(data) {
    const currentTableData = cloneDeep(this.tableData.data);

    remove(currentTableData, item => isEqual(data[0], item));
    this.tableData = {
      data: currentTableData,
      total: size(currentTableData)
    };
    this.formGroup.get('children').setValue(currentTableData);
  }

  getParams() {
    return this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType:
            this.subType === DataMap.Resource_Type.gbaseInstance.value
              ? DataMap.Resource_Type.gbaseInstance.value
              : DataMap.Resource_Type.informixInstance.value,
          parentUuid: this.formGroup.value.agents,
          extendInfo: {
            onconfigPath: this.formGroup.value.onconfig,
            sqlhostsPath: this.formGroup.value.sqlhost
          },
          dependencies: {
            agents: [
              {
                uuid: this.formGroup.value.agents
              }
            ]
          },
          auth: {
            authType: DataMap.Database_Auth_Method.os.value
          }
        }
      : {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType:
            this.subType === DataMap.Resource_Type.gbaseInstance.value
              ? DataMap.Resource_Type.gbaseClusterInstance.value
              : DataMap.Resource_Type.informixClusterInstance.value,
          parentUuid: get(this.formGroup.value.children, '[0].parentUuid'),
          dependencies: {
            children: map(this.formGroup.value.children, item => {
              return item;
            })
          }
        };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        if (
          includes(
            [
              DataMap.Resource_Type.informixClusterInstance.value ||
                DataMap.Resource_Type.gbaseInstance.value
            ],
            this.rowData.subType
          )
        ) {
          const deleteInstance = [];

          each(this.originalTableData, item => {
            if (
              !find(
                this.formGroup.value.children,
                child => item.endpoint === child.endpoint
              )
            ) {
              deleteInstance.push(item);
            }
          });

          if (!!size(deleteInstance)) {
            set(params.dependencies, '#children', deleteInstance);
          }
        }

        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
        return;
      }
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody: params
        })
        .subscribe({
          next: res => {
            cacheGuideResource(res);
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
