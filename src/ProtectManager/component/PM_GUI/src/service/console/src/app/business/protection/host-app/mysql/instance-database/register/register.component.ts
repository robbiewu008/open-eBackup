import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  getPermissionMenuItem,
  I18NService,
  InstanceType,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  findIndex,
  first,
  get,
  isEqual,
  isNumber,
  map,
  reject,
  remove,
  replace,
  set,
  size,
  isEmpty,
  filter,
  includes
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AddHostComponent } from './add-host/add-host.component';
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
  item;
  optsConfig;
  optItems = [];
  hostOptions = [];
  originalTableData;
  clusterOptions = [];
  dataMap = DataMap;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  charsetErrorTip = {
    invalidName: this.baseUtilService.invalidInputLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  confPathErrorTip = {
    invalidName: this.baseUtilService.invalidInputLabel,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private drawModalService: DrawModalService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig();
    this.updateData();
    this.getHostOptions();
    this.getclusterOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  showClusterGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  updateData() {
    if (!this.item) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.item.uuid })
      .subscribe(res => {
        const data =
          res.subType === DataMap.Resource_Type.MySQLInstance.value
            ? {
                name: res.name,
                type: DataMap.Instance_Type.single.value,
                agents: first(map(res['dependencies']['agents'], 'uuid')),
                instanceIp: res.extendInfo?.instanceIp,
                userName: res['auth']['authKey'],
                port: res?.extendInfo?.instancePort,
                charset: res?.extendInfo?.charset,
                configPath: res?.extendInfo?.myCnfPath
              }
            : {
                name: res.name,
                type: DataMap.Instance_Type.cluster.value,
                cluster: res.parentUuid,
                children: res['dependencies']['children']
              };
        this.formGroup.patchValue(data);
        if (res.subType === DataMap.Resource_Type.MySQLClusterInstance.value) {
          data?.children.filter(item => {
            assign(item, {
              hostName: item.dependencies?.agents[0]?.name,
              ip: item.dependencies?.agents[0]?.endpoint,
              port: item.extendInfo?.instancePort
            });

            set(item, 'auth.extendInfo', {
              instancePort: item.extendInfo?.instancePort
            });
          });
          this.originalTableData = data?.children;
          this.tableData = {
            data: data?.children,
            total: size(data?.children)
          };
        }
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      cluster: new FormControl(''),
      agents: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      instanceIp: new FormControl('', {
        validators: [this.baseUtilService.VALID.ip()]
      }),
      port: new FormControl('3306', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      charset: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.charset, false),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      configPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.templatLinuxPath,
            false
          ),
          this.baseUtilService.VALID.maxLength(2048)
        ]
      }),
      children: new FormControl([])
    });

    this.watch();
  }

  setChildrenVaild(resourceId) {
    if (
      !resourceId ||
      this.formGroup.value.type === DataMap.Instance_Type.single.value
    ) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId })
      .subscribe(res => {
        this.formGroup
          .get('children')
          .setValidators([
            this.baseUtilService.VALID.minLength(
              size(res['dependencies']['agents'])
            )
          ]);
        this.formGroup.get('children').updateValueAndValidity();
      });
  }

  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.updateSingle();
      } else {
        this.updateCluster();
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('children').updateValueAndValidity();
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (
        find(this.clusterOptions, { value: res })?.extendInfo?.clusterType ===
        DataMap.Mysql_Cluster_Type.eapp.value
      ) {
        setTimeout(() => this.setChildrenVaild(res));
      }
      if (!size(this.formGroup.value.children)) {
        return;
      }
      this.formGroup.get('children').setValue([]);
      this.tableData = {
        data: [],
        total: 0
      };
    });
  }

  updateSingle() {
    this.formGroup
      .get('agents')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('port')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.baseUtilService.VALID.rangeValue(1, 65535)
      ]);
    this.formGroup
      .get('userName')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.maxLength(32)
      ]);
    this.formGroup
      .get('password')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.maxLength(32)
      ]);
    this.formGroup.get('cluster').clearValidators();
    this.formGroup.get('children').clearValidators();
  }

  updateCluster() {
    this.formGroup
      .get('cluster')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('children')
      .setValidators([this.baseUtilService.VALID.minLength(1)]);
    this.formGroup.get('agents').clearValidators();
    this.formGroup.get('port').clearValidators();
    this.formGroup.get('userName').clearValidators();
    this.formGroup.get('password').clearValidators();
    setTimeout(() => {
      this.dataTable.fetchData();
    }, 0);
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.MySQLClusterInstance.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.item)) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = hostArray;
      }
    );
  }

  getclusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.MySQLCluster.value
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
      this.getclusterOptions(recordsTemp, startPage);
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
        }
      }
    ];

    this.optsConfig = getPermissionMenuItem(opts);
    this.optItems = cloneDeep(reject(this.optsConfig, { id: 'add' }));

    const cols: TableCols[] = [
      {
        key: 'hostName',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
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
    if (!this.formGroup.value.cluster) {
      this.formGroup.get('cluster').markAsTouched();
      return;
    }
    if (!this.formGroup.value.name) {
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
          parentUuid: this.formGroup.value.cluster,
          name: this.formGroup.value.name,
          children: this.formGroup.value.children,
          rowData: data,
          isEapp: isEqual(
            get(
              find(this.clusterOptions, { uuid: this.formGroup.value.cluster }),
              'extendInfo.clusterType'
            ),
            'EAPP'
          )
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddHostComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
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
                    item.extendInfo?.hostId ===
                    get(first(content.data), 'extendInfo.hostId')
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
                this.formGroup.get('children').updateValueAndValidity();
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
    this.formGroup.get('children').updateValueAndValidity();
  }

  getParams() {
    return this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.MySQLInstance.value,
          parentUuid: this.formGroup.value.agents,
          extendInfo: {
            hostId: this.formGroup.value.agents,
            instancePort: this.formGroup.value.port,
            isTopInstance: InstanceType.TopInstance,
            instanceIp: this.formGroup.value.instanceIp,
            // 如果输入为空 说明用户想删除字符集 传null接口会报错只能改为''
            charset: this.formGroup.value.charset,
            myCnfPath: this.formGroup.value.configPath
          },
          dependencies: {
            agents: [{ uuid: this.formGroup.value.agents }]
          },
          auth: {
            authType: DataMap.Database_Auth_Method.db.value,
            authKey: this.formGroup.value.userName,
            authPwd: this.formGroup.value.password,
            extendInfo: { instancePort: this.formGroup.value.port }
          }
        }
      : {
          name: this.formGroup.value.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.MySQLClusterInstance.value,
          parentUuid: this.formGroup.value.cluster,
          rootUuid: this.formGroup.value.cluster,
          extendInfo: {
            clusterType: find(this.clusterOptions, {
              uuid: this.formGroup.value.cluster
            })?.extendInfo?.clusterType,
            isTopInstance: InstanceType.TopInstance
          },
          dependencies: {
            children: map(this.formGroup.value.children, item => {
              set(
                item,
                'extendInfo.clusterType',
                find(this.clusterOptions, {
                  uuid: this.formGroup.value.cluster
                })?.extendInfo?.clusterType
              );
              if (this.item) {
                set(item, 'parentName', this.formGroup.value.name);
                set(
                  item,
                  'name',
                  replace(item.name, this.item.name, this.formGroup.value.name)
                );
              }
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
      if (this.item) {
        if (
          this.item.subType === DataMap.Resource_Type.MySQLClusterInstance.value
        ) {
          const deleteInstance = [];

          each(this.originalTableData, item => {
            if (
              !find(
                this.formGroup.value.children,
                child => item.uuid === child.uuid
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
            resourceId: this.item.uuid,
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
