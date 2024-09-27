import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getMultiHostOps,
  getPermissionMenuItem,
  I18NService,
  InstanceType,
  MODAL_COMMON,
  MultiCluster,
  ProtectedResourceApiService,
  ResourceType
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
  filter,
  find,
  first,
  includes,
  isEmpty,
  isEqual,
  isNumber,
  map,
  omit,
  remove,
  set,
  size,
  trim,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { PostgreAddHostComponent } from './add-host/postgre-add-host.component';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-postgre-register',
  templateUrl: './postgre-register.component.html',
  styleUrls: ['./postgre-register.component.less']
})
export class PostgreRegisterComponent implements OnInit {
  item;
  optItems = [];
  hostOptions = [];
  clusterOptions = [];
  dataMap = DataMap;
  hostNum = 0;
  _isEmpty = isEmpty;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  extendsAuth = {};
  isPgpool = true;
  chosenClusterType: string; // 选中的集群类型
  cols: TableCols[] = [];
  extraCols: TableCols[];
  clusterInfo; // 缓存集群信息
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
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32]),
    invalidUserNameBegin: this.i18n.get(
      'common_valid_linux_user_name_begin_label'
    ),
    invalidUserName: this.i18n.get('common_valid_linux_user_name_label')
  };
  dataBaseErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameBegin: this.i18n.get('system_valid_sftp_username_begin_label'),
    invalidDataBaseName: this.i18n.get('common_valid_database_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  // 路径校验
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label'),
    pathError: this.i18n.get('common_path_error_label')
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public dataMapService: DataMapService,
    private drawModalService: DrawModalService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig(); // 只做表格初始化操作
    this.updateData(); // 回显数据
    this.getHostOptions();
    this.getClusterOptions();
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
          res.subType === DataMap.Resource_Type.PostgreSQLInstance.value
            ? {
                name: res.name,
                type: DataMap.Instance_Type.single.value,
                agents: first(map(res['dependencies']['agents'], 'uuid')),
                userName: res['extendInfo']['osUsername'],
                client: res['extendInfo']['clientPath'],
                business_ip: res['extendInfo']['serviceIp'],
                port: res['extendInfo']['instancePort'],
                database_username: res['auth']['authKey']
              }
            : {
                name: res.name,
                type: DataMap.Instance_Type.cluster.value,
                cluster: res.parentUuid,
                client: res['extendInfo']['clientPath'],
                port: res['extendInfo']['instancePort'],
                database_username: res['auth']['authKey'],
                databaseStreamUserName: res['auth']['dbStreamRepUser'],
                userName: res['extendInfo']['osUsername'],
                children: res['dependencies']['children']
              };
        this.formGroup.patchValue(data);
        if (
          res.subType === DataMap.Resource_Type.PostgreSQLClusterInstance.value
        ) {
          data?.children.filter(item => {
            assign(item, {
              hostName: item.dependencies?.agents[0]?.name,
              ip: item.dependencies?.agents[0]?.endpoint,
              port: item.extendInfo?.instancePort,
              business_ip: item.extendInfo?.serviceIp,
              client: item.extendInfo?.clientPath,
              pgpoolClientPath: item.extendInfo?.pgpoolClientPath
            });
          });
          setTimeout(() => {
            this.tableData = {
              data: data?.children,
              total: size(data?.children)
            };
          }, 500);
        }
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl(
        { value: '', disabled: !!this.item },
        {
          validators: [
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      cluster: new FormControl(''),
      agents: new FormControl('', [this.baseUtilService.VALID.required()]),
      port: new FormControl(
        { value: '5432', disabled: !!this.item },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32),
          this.validUserName()
        ]
      }),
      client: new FormControl({ value: '', disabled: !!this.item }, [
        this.baseUtilService.VALID.required(),
        this.validPath()
      ]),
      business_ip: new FormControl({ value: '', disabled: !!this.item }, [
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]),
      database_username: new FormControl('', [
        this.validDataBaseName(),
        this.baseUtilService.VALID.maxLength(63),
        this.baseUtilService.VALID.required()
      ]),
      database_password: new FormControl('', [
        this.baseUtilService.VALID.required()
      ]),
      databaseStreamUserName: new FormControl(''),
      databaseStreamPassword: new FormControl(''),
      children: new FormControl([])
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === 0) {
        this.formGroup.get('port').setValue('5432');
      }
      this.formGroup.get('port').updateValueAndValidity();
    });

    this.watch();
  }

  validDataBaseName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const name = control.value;
      if (!CommonConsts.REGEX.opengaussRestoreName.test(name)) {
        return { invalidNameBegin: { value: control.value } };
      }
      if (!CommonConsts.REGEX.dataBaseName.test(name)) {
        return { invalidDataBaseName: { value: control.value } };
      }
      return null;
    };
  }

  validUserName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const userName = control.value;
      if (!CommonConsts.REGEX.linuxUserNameBegin.test(userName)) {
        return { invalidUserNameBegin: { value: control.value } };
      }
      if (!CommonConsts.REGEX.linuxUserName.test(userName)) {
        return { invalidUserName: { value: control.value } };
      }
      return null;
    };
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });
      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }

      if (
        find(paths, path => {
          return !CommonConsts.REGEX.templatLinuxPath.test(path);
        }) ||
        find(paths, path => {
          return path.length > 1024;
        })
      ) {
        return { pathError: { value: control.value } };
      }
    };
  }

  getProxyOptions(uuid) {
    if (!uuid) {
      return;
    }
    const params = {
      resourceId: uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      this.hostNum = res['dependencies']['agents'].length;
      if (
        res.extendInfo?.installDeployType ===
        DataMap.PostgreSqlDeployType.CLup.value
      ) {
        this.tableData = {
          data: map(res['dependencies']['agents'], item => ({
            hostName: item?.name,
            ip: item?.endpoint
          })),
          total: this.hostNum
        };
      }
      this.clusterInfo = res;
    });
  }

  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.updateSingle();
      } else {
        this.updateCluster();
      }
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('business_ip').updateValueAndValidity();
      this.formGroup.get('client').updateValueAndValidity();
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('children').updateValueAndValidity();
      this.formGroup.get('databaseStreamUserName').updateValueAndValidity();
      this.formGroup.get('databaseStreamPassword').updateValueAndValidity();
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.getProxyOptions(res);
      const chosenType = find(this.clusterOptions, { uuid: res })?.extendInfo
        ?.installDeployType;
      this.chosenClusterType = chosenType;
      if (chosenType === DataMap.PostgreSqlDeployType.CLup.value) {
        this.tableConfig.table.columns = [this.cols[0], this.extraCols[0]];
        this.formGroup.get('children').clearValidators();
      } else {
        this.tableConfig.table.columns = this.cols;
        this.formGroup
          .get('children')
          .setValidators([this.baseUtilService.VALID.minLength(1)]);
      }
      this.formGroup.get('children').updateValueAndValidity();
      // 没有相应字段时默认为pgpool
      this.isPgpool =
        !chosenType || chosenType === DataMap.PostgreSqlDeployType.Pgpool.value;
      if (this.formGroup.value.type === DataMap.Instance_Type.cluster.value) {
        if (this.isPgpool) {
          this.formGroup
            .get('port')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]);
          this.formGroup.get('port').setValue('9999');
        } else {
          this.formGroup.get('port').clearValidators();
          this.formGroup.get('port').setValue('5432');
        }
      }
      if (this.dataTable) {
        find(this.cols, { key: 'pgpoolClientPath' }).name = this.isPgpool
          ? this.i18n.get('common_pg_pool_path_label')
          : this.i18n.get('common_patroni_path_label');
        this.dataTable.reinit();
      }
      // 防止修改时patchValue引起children置空
      if (!size(this.formGroup.value.children) || this.item) {
        return;
      }
      this.formGroup.get('children').setValue([]);
      this.tableData = {
        data: [],
        total: 0
      };
    });

    this.formGroup.get('userName').valueChanges.subscribe(res => {
      if (!this.formGroup.value.userName) {
        return;
      }
    });
  }

  updateSingle() {
    this.formGroup
      .get('agents')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('business_ip')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]);
    this.formGroup
      .get('client')
      .setValidators([this.baseUtilService.VALID.required(), this.validPath()]);
    this.formGroup.get('cluster').clearValidators();
    this.formGroup.get('children').clearValidators();
    this.formGroup.get('databaseStreamUserName').clearValidators();
    this.formGroup.get('databaseStreamPassword').clearValidators();
  }

  updateCluster() {
    this.formGroup
      .get('cluster')
      .setValidators([this.baseUtilService.VALID.required()]);
    if (!this.item) {
      this.formGroup
        .get('children')
        .setValidators([this.baseUtilService.VALID.minLength(1)]);
    }
    this.formGroup
      .get('databaseStreamUserName')
      .setValidators([
        this.validDataBaseName(),
        this.baseUtilService.VALID.maxLength(63),
        this.baseUtilService.VALID.required()
      ]);
    this.formGroup
      .get('databaseStreamPassword')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup.get('agents').clearValidators();
    this.formGroup.get('business_ip').clearValidators();
    this.formGroup.get('client').clearValidators();
    setTimeout(() => {
      this.dataTable.fetchData();
    }, 0);
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.PostgreSQLInstance.value}Plugin`]
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
        if (isEmpty(this.item)) {
          // 注册场景才过滤主机
          if (MultiCluster.isMulti) {
            resource = getMultiHostOps(resource);
          } else {
            resource = filter(
              resource,
              item =>
                item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          }
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

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.PostgreSQLCluster.value
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
        if (this.item) {
          // 获取选项后更新cluster的valueChanges监听
          this.formGroup.get('cluster').updateValueAndValidity();
        }
        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
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
          return !isEmpty(this.item);
        }
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    this.cols = [
      {
        key: 'hostName',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'client',
        name: this.i18n.get('common_database_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'pgpoolClientPath',
        name: this.isPgpool
          ? this.i18n.get('common_pg_pool_path_label')
          : this.i18n.get('common_patroni_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'business_ip',
        name: this.i18n.get('common_dataplane_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_database_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: 70,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                disableCheck: () => {
                  return !isEmpty(this.item);
                },
                onClick: ([data]) => {
                  this.add(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_remove_label'),
                disableCheck: () => {
                  return !isEmpty(this.item);
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
    this.extraCols = [
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: this.cols,
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

  add(item?) {
    if (!this.formGroup.value.cluster) {
      this.formGroup.get('cluster').markAsTouched();
      return;
    }
    if (!this.formGroup.get('name')?.value) {
      this.formGroup.get('name').markAsTouched();
      return;
    }

    if (!this.formGroup.value.userName) {
      this.formGroup.get('userName').markAsTouched();
      return;
    }

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-postgre-host',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: PostgreAddHostComponent,
        lvComponentParams: {
          parentUuid: this.formGroup.value.cluster,
          children: this.formGroup.value.children,
          osUsername: this.formGroup.value.userName,
          isPgpool: this.isPgpool,
          item
        },
        lvOkDisabled: isEmpty(item),
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as PostgreAddHostComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as PostgreAddHostComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                if (isEmpty(item)) {
                  currentTableData = currentTableData.concat([content.data]);
                } else {
                  const oldItem = currentTableData.find(
                    cur => cur.extendInfo?.hostId === item.extendInfo?.hostId
                  );
                  assign(oldItem, content.data);
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
    const params =
      this.formGroup.value.type === DataMap.Instance_Type.single.value
        ? {
            name: this.formGroup.get('name').value,
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.PostgreSQLInstance.value,
            parentUuid: this.formGroup.value.agents,
            extendInfo: {
              hostId: this.formGroup.value.agents,
              instancePort: this.formGroup.get('port').value,
              clientPath: this.formGroup.get('client').value,
              serviceIp: this.formGroup.get('business_ip').value,
              osUsername: this.formGroup.value.userName,
              isTopInstance: InstanceType.TopInstance
            },
            dependencies: {
              agents: [{ uuid: this.formGroup.value.agents }]
            },
            auth: {
              ...this.extendsAuth,
              extendInfo: {}
            }
          }
        : {
            name: this.formGroup.get('name').value,
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.PostgreSQLClusterInstance.value,
            parentUuid: this.formGroup.value.cluster,
            rootUuid: this.formGroup.value.cluster,
            extendInfo: {
              osUsername: this.formGroup.value.userName,
              instancePort: this.formGroup.get('port').value,
              isTopInstance: InstanceType.TopInstance,
              installDeployType: this.isPgpool
                ? DataMap.PostgreSqlDeployType.Pgpool.value
                : DataMap.PostgreSqlDeployType.Patroni.value
            },
            auth: {
              ...this.extendsAuth,
              extendInfo: {
                dbStreamRepUser: this.formGroup.value.databaseStreamUserName,
                dbStreamRepPwd: this.formGroup.value.databaseStreamPassword
              }
            },
            dependencies: {
              children: this.formGroup.value.children.map(item => {
                const newItem = omit(item, [
                  'port',
                  'hostName',
                  'ip',
                  'client',
                  'business_ip',
                  'parent',
                  'pgpoolClientPath'
                ]);
                newItem['auth'] = {
                  ...this.extendsAuth,
                  extendInfo: {
                    dbStreamRepUser: this.formGroup.value
                      .databaseStreamUserName,
                    dbStreamRepPwd: this.formGroup.value.databaseStreamPassword
                  }
                };
                newItem['extendInfo'][
                  'osUsername'
                ] = this.formGroup.value.userName;
                return newItem;
              })
            }
          };
    if (this.chosenClusterType === DataMap.PostgreSqlDeployType.CLup.value) {
      delete params.extendInfo.instancePort;
      set(
        params,
        'dependencies.children',
        map(this.clusterInfo.dependencies.agents, item => ({
          parentUuid: '',
          name: null,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.PostgreSQLInstance.value,
          extendInfo: {
            hostId: item.uuid,
            osUsername: this.formGroup.value.userName,
            isTopInstance: InstanceType.NotTopinstance
          },
          dependencies: {
            agents: [{ uuid: item.uuid }]
          },
          auth: {
            ...this.extendsAuth,
            extendInfo: {
              dbStreamRepUser: this.formGroup.value.databaseStreamUserName,
              dbStreamRepPwd: this.formGroup.value.databaseStreamPassword
            }
          }
        }))
      );
      set(
        params,
        'dependencies.clupServers',
        map(this.clusterInfo.dependencies.clupServers, item => ({
          uuid: item.uuid
        }))
      );
      set(
        params,
        'extendInfo.installDeployType',
        DataMap.PostgreSqlDeployType.CLup.value
      );
    }
    return params;
  }

  onOK(): Observable<void> {
    this.extendsAuth['authType'] = DataMap.Postgre_Auth_Method.db.value;
    this.extendsAuth['authKey'] = this.formGroup.value.database_username;
    this.extendsAuth['authPwd'] = this.formGroup.value.database_password;

    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.item) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.item.uuid,
            UpdateResourceRequestBody: params as any
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
          CreateResourceRequestBody: params as any
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
