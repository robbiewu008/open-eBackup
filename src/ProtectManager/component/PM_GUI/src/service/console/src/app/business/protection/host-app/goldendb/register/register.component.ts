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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isObject,
  map,
  mapKeys,
  set,
  size,
  some,
  uniq
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  instances;
  dataDetail;
  optsConfig;
  gtmOptItems = [];
  gtmTableData = {
    data: [],
    total: 0
  };
  gtmTableConfig: TableConfig;
  dataOptItems = [];
  dataTableData = {
    data: [],
    total: 0
  };
  dataTableConfig: TableConfig;
  clusterOptions = [];
  instanceOptions = [];
  proxyOptions = [];
  typeOptions = this.dataMapService.toArray('dbTwoType').map(item => {
    return {
      ...item,
      isLeaf: true
    };
  });
  fileFilters = [];
  selectedFile = '';
  dataMap = DataMap;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  validNode$ = new Subject<boolean>();
  commonErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    ...this.commonErrorTip
  };

  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    spaceErrorTip: this.i18n.get('common_valid_space_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @Input() rowData;
  @ViewChild('dataNodeProxyHostExtraTpl', { static: true })
  dataNodeProxyHostExtraTpl: TemplateRef<any>;
  @ViewChild('dataNodeUserNameExtraTpl', { static: true })
  dataNodeUserNameExtraTpl: TemplateRef<any>;
  @ViewChild('customRequiredTHTpl', { static: true })
  customRequiredTHTpl: TemplateRef<any>;
  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
    this.initDataConfig();
    this.initGTMConfig();
    this.getProxyOptions();
    this.getClusterOptions();
    this.updateData();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.name),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instance: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      dataNode: new FormControl([]),
      gtmNode: new FormControl([])
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.formGroup.get('instance').setValue('');
      this.getInstanceOptions(res);
    });

    this.formGroup.get('instance').valueChanges.subscribe(res => {
      this.dataTableData = { data: [], total: 0 };
      this.gtmTableData = { data: [], total: 0 };
      this.getDataNode(res);
    });
  }

  initFilters() {
    this.fileFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['ini'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          this.selectedFile = '';
          this.valid$.next(false);

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['ini']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectedFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.valid$.next(true);
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  filesChange(file) {
    if (!size(file)) {
      this.selectedFile = '';
      this.valid$.next(false);
    }
  }

  setValid() {
    this.validNode$.next(
      !(
        some(
          this.gtmTableData.data,
          item => isEmpty(item.parentUuid) || item.osUserControl.invalid
        ) ||
        some(
          this.dataTableData.data,
          item => isEmpty(item.parentUuid) || item.osUserControl.invalid
        )
      )
    );
  }

  initDataConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('protection_data_node_label'),
        width: 120,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'group',
        name: this.i18n.get('protection_goldendb_part_label'),
        width: 130,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'dataNodeParentName',
        name: this.i18n.get('protection_client_label'),
        width: 270,
        thExtra: this.customRequiredTHTpl,
        cellRender: this.dataNodeProxyHostExtraTpl
      },
      {
        key: 'dataNodeOsUser',
        name: this.i18n.get('common_username_label'),
        thExtra: this.customRequiredTHTpl,
        cellRender: this.dataNodeUserNameExtraTpl
      }
    ];

    this.dataTableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'id',
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.id;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initGTMConfig() {
    const cols: TableCols[] = [
      {
        key: 'gtmId',
        name: 'GTMID',
        width: 100,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'dataNodeParentName',
        name: this.i18n.get('protection_clients_label'),
        thExtra: this.customRequiredTHTpl,
        cellRender: this.dataNodeProxyHostExtraTpl
      },
      {
        key: 'dataNodeOsUser',
        name: this.i18n.get('common_username_label'),
        thExtra: this.customRequiredTHTpl,
        cellRender: this.dataNodeUserNameExtraTpl
      }
    ];

    this.gtmTableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'gtmId',
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.gtmId;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  updateData() {
    if (!this.rowData) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.dataDetail = res;
        this.formGroup.get('name').setValue(res.name);
        this.formGroup.get('cluster').setValue(res.parentUuid);
        this.formGroup.get('userName').setValue(get(res, 'auth.authKey'));
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.goldendbCluter.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.goldendbCluter.value]
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
        startPage ===
          Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
        res.totalCount === 0
      ) {
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });

        this.clusterOptions = clusterArray;

        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  getInstanceOptions(clusterId, recordsTemp?, startPage?) {
    if (!clusterId) {
      return;
    }

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: clusterId,
      resourceType: DataMap.Resource_Type.goldendbCluter.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          const clusterArray = [];
          each(recordsTemp, item => {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          });

          this.instanceOptions = clusterArray;

          if (!!this.rowData) {
            const instance = JSON.parse(
              get(this.dataDetail, 'extendInfo.clusterInfo', '{}')
            );
            this.formGroup.get('instance').setValue(instance.id);
          }
          return;
        }
        this.getInstanceOptions(clusterId, recordsTemp, startPage);
      });
  }

  getDataNode(instanceId) {
    if (!instanceId) {
      return;
    }
    let tableData = [];
    this.instances = JSON.parse(
      get(
        find(this.instanceOptions, { uuid: instanceId }),
        'extendInfo.clusterInfo',
        '{}'
      )
    );
    each(get(this.instances, 'group', []), item => {
      each(item.mysqlNodes, node => {
        assign(node, {
          group: `DBGroup${item.groupId}`
        });
      });

      tableData = [...tableData, ...item.mysqlNodes];
    });

    if (!!this.rowData) {
      const originalInstance = JSON.parse(
        get(this.dataDetail, 'extendInfo.clusterInfo', '{}')
      );

      each(get(originalInstance, 'group', []), item => {
        each(item.mysqlNodes, node => {
          const matchNode = find(tableData, { id: node.id });

          if (!!matchNode) {
            assign(matchNode, {
              parentUuid: node?.parentUuid,
              parentName: node?.parentName,
              osUser: node?.osUser
            });
          }
        });
      });
    }
    this.createFormControl(tableData);
    this.dataTableData = {
      data: tableData,
      total: size(tableData)
    };
    this.getGTMNode();
  }

  getGTMNode() {
    let tableData = get(this.instances, 'gtm', []);

    if (!!this.rowData) {
      const originalInstance = JSON.parse(
        get(this.dataDetail, 'extendInfo.clusterInfo', '{}')
      );

      each(get(originalInstance, 'gtm', []), node => {
        const matchNode = find(tableData, { gtmId: node.gtmId });

        if (!!matchNode) {
          assign(matchNode, {
            parentUuid: node?.parentUuid,
            parentName: node?.parentName,
            nodeType: DataMap.goldendbNodeType.gtmNode.value,
            osUser: node?.osUser
          });
        }
      });
    }
    this.createFormControl(tableData, DataMap.goldendbNodeType.gtmNode.value);
    this.gtmTableData = {
      data: tableData,
      total: size(tableData)
    };
    if (!!this.rowData) {
      this.setValid();
    }
  }

  createFormControl(data, nodeType?) {
    each(data, item => {
      if (nodeType) {
        set(item, 'nodeType', nodeType);
      }
      assign(item, {
        parentUuidControl: new FormControl(item?.parentUuid || '', {
          validators: [this.baseUtilService.VALID.required()]
        }),
        osUserControl: new FormControl(item?.osUser || '', {
          validators: [
            this.validSpace(),
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]
        })
      });
      item.parentUuidControl.valueChanges.subscribe(res => {
        const host = find(this.proxyOptions, { uuid: res });
        item.parentUuid = res || null;
        item.parentName = host ? `${host.name}(${host.endpoint})` : null;
        this.setValid();
      });
      item.osUserControl.valueChanges.subscribe(res => {
        item.osUser = res;
        this.setValid();
      });
    });
  }

  validSpace(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const spaceReg = /\s/;
      if (spaceReg.test(control.value)) {
        return { spaceErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  // 定义一个函数来递归地删除指定的属性
  cloneExcludeFormControls(obj, hash = new WeakMap()) {
    if (obj == null) return obj;
    if (hash.has(obj)) return hash.get(obj);
    const cloneObj = {};
    hash.set(obj, cloneObj);
    each(obj, (value, key) => {
      if (
        (key === 'gtm' || key === 'group' || key === 'mysqlNodes') &&
        isArray(value)
      ) {
        cloneObj[key] = this.cloneArrayExcludeFormControls(value, hash);
      } else if (!(value instanceof FormControl)) {
        cloneObj[key] = value;
      }
    });
    return cloneObj;
  }

  cloneArrayExcludeFormControls(arr: any[], hash = new WeakMap()) {
    const cloneArr = [];
    each(arr, element => {
      if (isObject(element) && !(element instanceof FormControl)) {
        cloneArr.push(this.cloneExcludeFormControls(element, hash));
      } else {
        cloneArr.push(element);
      }
    });
    return cloneArr;
  }

  getParams() {
    const agents = uniq([
      ...map(this.gtmTableData.data, 'parentUuid'),
      ...map(this.dataTableData.data, 'parentUuid')
    ]);
    const deletedNode = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(agents, val => val === item.uuid)
    );
    const clusterInfo = this.cloneExcludeFormControls(this.instances);
    return {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.goldendbInstance.value,
      parentUuid: this.formGroup.value.cluster,
      auth: {
        authType: DataMap.Database_Auth_Method.db.value,
        authKey: this.formGroup.value.userName,
        authPwd: this.formGroup.value.password
      },
      extendInfo: {
        clusterInfo: JSON.stringify(clusterInfo),
        local_ini_cnf:
          this.selectedFile || get(this.rowData, 'extendInfo.local_ini_cnf')
      },
      dependencies: {
        agents: map(agents, item => {
          return {
            uuid: item
          };
        }),
        '-agents': map(deletedNode, item => {
          return {
            uuid: item.uuid
          };
        })
      }
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (find(this.dataTableData.data, item => isEmpty(item.parentUuid))) {
        this.messageService.error(
          this.i18n.get('protection_data_node_error_tips_label'),
          {
            lvMessageKey: 'goldendb_data_node',
            lvShowCloseButton: true
          }
        );
        observer.error(new Error());
        observer.complete();
        return;
      }

      if (find(this.gtmTableData.data, item => isEmpty(item.parentUuid))) {
        this.messageService.error(
          this.i18n.get('protection_gtm_node_error_tips_label'),
          {
            lvMessageKey: 'goldendb_gtm_node',
            lvShowCloseButton: true
          }
        );
        observer.error(new Error());
        observer.complete();
        return;
      }

      const params = this.getParams();
      if (this.rowData) {
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
      } else {
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
      }
    });
  }
}
