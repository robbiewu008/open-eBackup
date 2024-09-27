import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  MessageService,
  ModalRef,
  UploadFile,
  UploadFileStatusEnum
} from '@iux/live';
import { StorResourceNodeComponent } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node.component';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  Features,
  getMultiHostOps,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationMap,
  Scene
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  filter,
  first,
  get,
  isEmpty,
  isEqual,
  map,
  remove,
  size,
  trim,
  toString as _toString,
  isArray
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-fusion-compute',
  templateUrl: './register-fusion-compute.component.html',
  styleUrls: ['./register-fusion-compute.component.less']
})
export class RegisterFusionComputeComponent implements OnInit {
  resType;
  isModifyExist;
  treeSelection;
  DataMap = DataMap;
  item;

  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };
  optItems = [];
  centralizedLabel = this.i18n.get('protection_hcs_centralized_label');
  scaleoutLabel = this.i18n.get('protection_hcs_scale_out_label');

  formGroup: FormGroup;
  originNameList = [];
  originIpList = [];
  fcCertFilters = [];
  validFcSite$ = new Subject<boolean>();
  selectFcSiteFile = '';

  revocationListFilters = [];
  selectRevocationList = '';

  certFiles = [];
  crlFiles = [];

  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';
  isSupport = false;
  isDisplay = false; // 修改场景首次回显状态值

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip,
    invalidSameIp: this.i18n.get('protection_vm_duplicate_ip_label')
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  roleOperationMap = RoleOperationMap;

  proxyOptions = []; // 代理主机

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private message: MessageService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.isDisplay = !!this.item;
    this.isModifyExist =
      !isEmpty(this.item) &&
      this.treeSelection[0].rootNodeType ===
        DataMap.Resource_Type.fusionComputeCNA.value;
    this.initForm();
    this.initConfig();
    this.initFilters();
    this.getProxyOptions();
  }

  // 判断当前版本是否支持添加存储资源
  isSupportFunc(agent) {
    const params = {
      hostUuidsAndIps: agent,
      applicationType: 'FusionCompute',
      scene: Scene.Register,
      buttonNames: [Features.StorageResources]
    };

    if (!this.isDisplay) {
      this.tableData = {
        data: [],
        total: 0
      };
      this.formGroup.get('children').setValue([]);
    }

    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        this.isSupport = res?.StorageResources;
      });
  }

  addStorage(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-cluster-node',
        lvWidth: MODAL_COMMON.normalWidth + 150,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: StorResourceNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data,
          tableData: this.tableData,
          children: this.formGroup.value.children,
          subType: DataMap.Resource_Type.FusionCompute.value
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as StorResourceNodeComponent;
            content.onOK().subscribe({
              next: res => {
                if (!res) {
                  resolve(false);
                  return;
                }
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                if (isEmpty(data)) {
                  currentTableData = currentTableData.concat([content.data]);
                } else {
                  const oldItem = currentTableData.find(
                    item =>
                      _toString(item.ip) === _toString(data.ip) &&
                      item.port === data.port
                  );
                  assign(oldItem, content.data, {
                    storageType:
                      content.data?.storageType === 0
                        ? this.centralizedLabel
                        : this.scaleoutLabel
                  });
                }
                this.formGroup.get('children').setValue(currentTableData);
                this.formGroup.get('children').updateValueAndValidity();
                this.tableData = {
                  data: currentTableData,
                  total: size(currentTableData)
                };
                if (isEmpty(data)) {
                  this.tableData.data[
                    this.tableData.data.length - 1
                  ].storageType =
                    this.tableData.data[this.tableData.data.length - 1]
                      .storageType === 0
                      ? this.centralizedLabel
                      : this.scaleoutLabel;
                }
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

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(!isEmpty(this.item) ? this.item.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.validIsSameName()
        ]
      }),
      ip: new FormControl(!isEmpty(this.item) ? this.item.endpoint : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip(),
          this.validIsSameIp()
        ]
      }),
      userName: new FormControl(
        !isEmpty(this.item) ? this.item.auth.authKey : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),

      port: new FormControl(!isEmpty(this.item) ? this.item.port : '7443', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      agents: new FormControl(
        !isEmpty(this.item) ? map(this.item.dependencies?.agents, 'uuid') : [],
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      cert: new FormControl(
        !isEmpty(this.item) ? this.item?.extendInfo?.enableCert === '1' : true
      ),
      children: new FormControl([])
    });

    this.formGroup.statusChanges.subscribe(formGroupStatus => {
      const modalIns = this.modal.getInstance();
      modalIns.lvOkDisabled = this.formGroup.value.cert
        ? !(formGroupStatus === 'VALID' && this.certName)
        : formGroupStatus !== 'VALID';
    });
    this.formGroup.get('cert')?.valueChanges.subscribe(() => {
      this.selectFcSiteFile = ''; // 清空
      this.selectRevocationList = '';
    });

    //切换主机是需要重新查询当前主机是否支持添加存储资源
    this.formGroup.get('agents').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        this.isSupport = false;
      } else {
        this.isSupportFunc(res);
      }
    });

    if (isEmpty(this.item)) {
      return;
    }

    const showData = JSON.parse(this.item.extendInfo?.storages || '[]').map(
      item => {
        return {
          ip: item.ipList || item.ip,
          port: item.port,
          username: item.username,
          enableCert: item.enableCert,
          certName: item.certName,
          certSize: item.certSize,
          storageType:
            item.storageType === 1 ? this.centralizedLabel : this.scaleoutLabel
        };
      }
    );
    this.tableData = {
      data: showData,
      total: size(showData)
    };
    this.formGroup.get('children').setValue(this.tableData.data);

    const { extendInfo } = this.item;
    defer(() => {
      if (extendInfo?.certName) {
        this.certFiles = [
          {
            key: '-1',
            name: extendInfo.certName,
            fileSize: extendInfo.certSize,
            status: UploadFileStatusEnum.SUCCESS
          }
        ];
        this.certName = extendInfo.certName;
        this.certSize = extendInfo.certSize;
      }
      if (extendInfo?.crlName) {
        this.crlFiles = [
          {
            key: '-1',
            name: extendInfo.crlName,
            fileSize: extendInfo.crlSize,
            status: UploadFileStatusEnum.SUCCESS
          }
        ];
        this.crlName = extendInfo.crlName;
        this.crlSize = extendInfo.crlSize;
      }
      this.formGroup.get('cert').setValue(extendInfo?.enableCert == '1');
    });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        disableCheck: () => {
          return this.tableData?.total === 32;
        },
        disabledTips: this.i18n.get('protection_number_limited_label'),
        onClick: () => this.addStorage()
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'ip',
        name: this.i18n.get('common_management_ip_label'),
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
        key: 'username',
        name: this.i18n.get('common_username_label'),
        hidden: true,
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
                onClick: ([data]) => this.addStorage(data)
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                onClick: data => this.delete(data)
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
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10,
        pageSizeOptions: [10]
      }
    };
  }

  initFilters() {
    this.fcCertFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectFcSiteFile = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              !this.selectFcSiteFile || this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.revocationListFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crl'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['crl']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 5 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['5KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );

            this.selectRevocationList = ''; // 清空
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              (!this.selectFcSiteFile && !this.certName) ||
              this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  private _certClear() {
    this.certFiles = [];
    each(['selectFcSiteFile', 'certName', 'certSize'], key => {
      this[key] = void 0;
    });
    return this;
  }

  private _crlClear() {
    this.crlFiles = [];
    each(['selectRevocationList', 'crlName', 'crlSize'], key => {
      this[key] = void 0;
    });
    return this;
  }

  cartChange(e) {
    e?.action === 'remove' && this._certClear();
  }

  crlChange(e) {
    e?.action === 'remove' && this._crlClear();
  }

  certFilesChange(files) {
    if (size(files) === 0) {
      each(['certName', 'certSize'], key => (this[key] = ''));
    } else {
      this.certName = get(first(files), 'name');
      this.certSize = get(first(files), 'fileSize');
    }
  }

  crlFilesChange(files) {
    if (size(files) === 0) {
      each(['crlName', 'crlSize'], key => (this[key] = ''));
    } else {
      this.crlName = get(first(files), 'name');
      this.crlSize = get(first(files), 'fileSize');
    }
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${this.resType ||
            DataMap.Application_Type.FusionCompute.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            !isEmpty(item.environment) &&
            item.environment.extendInfo.scenario ===
              DataMap.proxyHostType.external.value
        );
        if (isEmpty(this.item)) {
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
          // 注册的场景
          if (isEmpty(this.item)) {
            const tmp = item.environment;
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          } else {
            // 修改的场景
            const tmp = item.environment;
            if (
              (map(this.item.dependencies?.agents, 'uuid')?.includes(
                tmp.uuid
              ) &&
                tmp.extendInfo.scenario ===
                  DataMap.proxyHostType.external.value) ||
              tmp.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            ) {
              hostArray.push({
                ...tmp,
                key: tmp.uuid,
                value: tmp.uuid,
                label: `${tmp.name}(${tmp.endpoint})`,
                isLeaf: true
              });
            }
          }
        });
        this.proxyOptions = hostArray;

        defer(() => (this.isDisplay = false));
      }
    );
  }

  validIsSameName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originNameList.includes(trim(control.value))
        ? { invalidSameName: { value: control.value } }
        : null;
    };
  }

  validIsSameIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originIpList.includes(trim(control.value))
        ? { invalidSameIp: { value: control.value } }
        : null;
    };
  }

  getParams() {
    let reduceAgents = [];
    if (this.item) {
      reduceAgents = differenceBy(
        this.item?.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    const storages = cloneDeep(this.formGroup.value.children);
    if (this.formGroup.value.children.length !== 0) {
      storages.forEach(item => {
        item.storageType =
          item.storageType === this.centralizedLabel ? '0' : '1';
      });
    }
    return {
      name: this.formGroup.value.name,
      type: ResourceType.PLATFORM,
      endpoint: this.formGroup.value.ip,
      port: +this.formGroup.value.port,
      subType: this.resType || ResourceType.FUSION_COMPUTE,
      auth: {
        authType: 2,
        authKey: this.formGroup.value.userName,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          enableCert: String(+this.formGroup.value.cert),
          cerBase64: isEmpty(this.selectFcSiteFile)
            ? void 0
            : this.selectFcSiteFile,
          certification: isEmpty(this.selectFcSiteFile)
            ? void 0
            : this.selectFcSiteFile,
          revocationlist: isEmpty(this.selectRevocationList)
            ? void 0
            : this.selectRevocationList,
          storages: JSON.stringify(
            map(storages, item => {
              assign(item, {
                ip: isArray(item.ip) ? first(item.ip) : item.ip?.split(',')[0],
                ipList: isArray(item.ip) ? item.ip?.join(',') : item.ip
              });
              return item;
            })
          )
        }
      },
      extendInfo: {
        enableCert: String(+this.formGroup.value.cert),
        certName: this.formGroup.value.cert ? this.certName : void 0,
        certSize: this.formGroup.value.cert ? this.certSize : void 0,
        crlName: this.formGroup.value.cert ? this.crlName : void 0,
        crlSize: this.formGroup.value.cert ? this.crlSize : void 0,
        storages: JSON.stringify(
          map(this.formGroup.value.children, item => {
            return {
              username: item.username,
              port: item.port,
              ip: isArray(item.ip) ? first(item.ip) : item.ip?.split(',')[0],
              ipList: isArray(item.ip) ? item.ip?.join(',') : item.ip,
              enableCert: String(+item.enableCert),
              certName: item.certName,
              certSize: item.certSize,
              crlName: item.crlName,
              crlSize: item.crlSize,
              storageType:
                item.storageType === this.centralizedLabel ? '0' : '1'
            };
          })
        )
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return { uuid: item };
        }),
        '-agents': !isEmpty(this.item)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  // 创建fc
  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: this.getParams() as any
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  // 修改fc
  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          UpdateProtectedEnvironmentRequestBody: this.getParams(),
          envId: this.treeSelection[0].uuid
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
