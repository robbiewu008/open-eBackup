import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  quaDrantTable,
  quaDrantTableOther,
  SystemApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  BaseUtilService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import {
  assign,
  cloneDeep,
  countBy,
  defer,
  each,
  filter,
  find,
  get,
  includes,
  intersection,
  map,
  nth,
  size,
  some,
  trim,
  uniq
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-creat-logic-port',
  templateUrl: './creat-logic-port.component.html',
  styleUrls: ['./creat-logic-port.component.less']
})
export class CreatLogicPortComponent implements OnInit {
  modifyData;
  data;
  portData;
  componentData;
  rowData;
  controlType;
  memberEsn;
  usedPortIdList = [];
  usedVlanList = [];
  formGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  logicTableConfig: TableConfig;
  logicTableData: TableData;
  logicSelectionData = [];
  dataMap = DataMap;
  ethPortOptions = [];
  validTable = new Subject<boolean>();
  bondPortList = [];
  nameList = [];
  ipList = [];
  ableEthPortDtoList = [];
  vlan = false;
  portLimit = false;
  mtuLimit = 9000;
  private readonly MAX_LOGIC_NGNAME_LENGTH = 255;
  isX9000 = includes(
    [DataMap.Deploy_Type.x9000.value],
    this.i18n.get('deploy_type')
  );
  disableRoleRadio = {
    backup: false,
    replicate: false,
    archive: false
  };
  quaDrantTable = quaDrantTable;
  quaDrantTableOther = quaDrantTableOther;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('logicDataTable', { static: false })
  logicDataTable: ProTableComponent;

  roleOptions = this.dataMapService.toArray('initRole').filter(item => {
    return (item.isLeaf = true);
  });
  ipTypeOptions = this.dataMapService.toArray('IP_Type').filter(item => {
    return (item.isLeaf = true);
  });
  homePortTypeOptions = this.dataMapService
    .toArray('initHomePortType')
    .filter(item => {
      return (item.isLeaf = true && item.value !== '8');
    });

  nameErrorTip = {
    invalidName: this.i18n.get('common_bongding_port_name_tips_label'),
    required: this.i18n.get('common_required_label'),
    invalidSameName: this.i18n.get(
      'common_create_logical_port_same_name_label'
    ),
    invalidLengthName: this.i18n.get(
      'common_bongding_port_name_length_tips_label',
      [this.MAX_LOGIC_NGNAME_LENGTH]
    )
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip,
    invalidSameName: this.i18n.get('common_create_logical_port_same_ip_label')
  };

  prefixErrorTip = assign(
    {},
    this.baseUtilService.rangeErrorTip,
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
    },
    this.baseUtilService.requiredErrorTip,
    {
      invalidName: this.i18n.get('common_invalid_inputtext_label')
    }
  );

  mtuErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      1280,
      this.mtuLimit
    ]),
    invalidInput: this.i18n.get('common_valid_integer_label')
  };

  vlanErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_config_vlan_id_error_tip_label')
  };

  constructor(
    public modal: ModalRef,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService
  ) {}

  ngOnInit() {
    each(this.portData?.data, item => {
      this.nameList.push(item.name);
    });
    each(this.portData?.data, item => {
      this.ipList.push(item.ip);
    });
    this.ableEthPortDtoList = cloneDeep(this.data?.ethPortDtoList);
    this.initForm();
    this.initRoles();
    this.initBondingportOptions();
    this.initConfig();
    if (this.modifyData) {
      this.updateData();
    }
  }

  nameChange(type) {
    // 根据所选角色自动匹配一个名称
    let backupData = filter(this.rowData, item => item.role === type);
    let backupNum: any = size(backupData) + 1;
    if (backupNum < 10) {
      backupNum = `0${backupNum}`;
    }
    let typeName =
      type === DataMap.initRole.data.value
        ? 'Backup'
        : type === DataMap.initRole.copy.value
        ? 'Replication'
        : 'Archive';
    while (
      find(
        this.rowData,
        item => item.name === `${this.controlType}-${typeName}${backupNum}`
      )
    ) {
      backupNum = Number(backupNum) + 1;
      if (backupNum < 10) {
        backupNum = `0${backupNum}`;
      }
    }
    this.formGroup
      .get('name')
      .setValue(`${this.controlType}-${typeName}${backupNum}`);
  }

  getControlType(item) {
    // x9000根据象限划分控制器
    let Quadrant = nth(item.location.split('.'), -2);
    if (!this.controlType.includes(item.ownIngController)) {
      return false;
    }
    return some(['A', 'B', 'C', 'D'], type => {
      return (
        this.controlType.includes(type) &&
        includes(quaDrantTable[type], Quadrant)
      );
    });
  }

  confirmControlType(item) {
    return this.isX9000
      ? this.getControlType(item)
      : item.ownIngController === this.controlType;
  }

  initBondingportOptions() {
    let usedEtherPorts = [];
    each(this.data.dmLogicPortList, item => {
      if (
        item.HOMEPORTTYPE === DataMap.initHomePortType.ethernet.value &&
        !usedEtherPorts.includes(item.HOMEPORTNAME)
      ) {
        // 用于以太网类型的以太网口无法用于绑定端口
        usedEtherPorts.push(item.HOMEPORTNAME);
      }
    });

    each(this.data.bondPortList, item => {
      // 某些场景只通过逻辑端口来过滤不全，所以也要根据绑定端口来过滤,这个list包含所有的绑定端口
      const portList = item.bondInfo.split(',');
      each(portList, port => {
        if (!this.modifyData) {
          usedEtherPorts.push(port);
        }
      });
    });

    each(this.data.vlanList, item => {
      // 把所有的vlan也过滤掉，不管是底座的还是界面上创的
      // 需要回显所以修改时不能过滤掉
      if (!this.modifyData) {
        usedEtherPorts = [...usedEtherPorts, ...item.portNameList];
      }
    });

    usedEtherPorts = uniq(usedEtherPorts);

    const tablePorts = [];
    each(this.ableEthPortDtoList, item => {
      if (
        this.confirmControlType(item) &&
        !find(usedEtherPorts, port => port === item.location) &&
        item.logicType !== DataMap.initLogicType.podFrontEndPort.value
      ) {
        tablePorts.push(item);
        if (this.modifyData) {
          assign(item, {
            disabled: true
          });
        }
      }
    });
    if (tablePorts.length < 2 && !this.modifyData) {
      this.portLimit = true;
      this.formGroup.get('portChoice').setValue('0');
    }
    this.tableData = {
      data: tablePorts,
      total: tablePorts.length
    };

    this.getOldLogicPortData();
  }

  getOldLogicPortData() {
    // 获取复用的端口/VLAN
    const oldLogicPorts = [];
    if (
      this.formGroup.value.homePortType ===
      DataMap.initHomePortType.bonding.value
    ) {
      each(this.data.bondPortList, item => {
        const ethPort = filter(
          this.data.ethPortDtoList,
          port =>
            item.portIdList.includes(port.id) && this.confirmControlType(port)
        );
        if (!!ethPort.length) {
          let bondVlan = filter(this.data.vlanList, { bondPortId: item.id });
          item.portNameList = ethPort.map(item => {
            return item.location;
          });
          if (!!bondVlan.length && !this.vlan) {
            item.vlanId = bondVlan[0].tags[0];
            item.tagID = bondVlan[0].tags + item.bondInfo;
            oldLogicPorts.push(item);
          } else if (!!bondVlan.length && this.vlan) {
            each(bondVlan, bond => {
              oldLogicPorts.push({
                ...item,
                vlanId: bond.tags[0],
                tagID: bond.tags[0] + item.bondInfo
              });
            });
          } else {
            oldLogicPorts.push(
              assign(item, {
                tagID: item.bondInfo
              })
            );
          }
        }
      });
    } else {
      each(this.data.vlanList, item => {
        if (
          item.portType === DataMap.initHomePortType.ethernet.value &&
          find(
            this.data.ethPortDtoList,
            port =>
              item.portNameList.includes(port.location) &&
              this.confirmControlType(port)
          )
        ) {
          oldLogicPorts.push(
            assign(item, {
              vlanId: item.tags[0],
              name: `${item.portNameList[0]}.${item.tags[0]}`,
              tagID: `${item.portNameList[0]}.${item.tags[0]}`
            })
          );
        }
      });
    }
    this.logicTableData = {
      data: oldLogicPorts,
      total: oldLogicPorts.length
    };
  }

  updateData() {
    this.formGroup.patchValue({
      name: this.modifyData.name,
      role: this.modifyData.role,
      supportProtocol: this.modifyData.supportProtocol,
      ip: this.modifyData.ip,
      ipType: this.modifyData.ipType,
      mask: this.modifyData?.mask,
      gateWay: this.modifyData?.gateWay,
      homePortType:
        this.modifyData.homePortType === DataMap.initHomePortType.vlan.value
          ? this.modifyData.vlan.portType
          : this.modifyData.homePortType,
      ethPort:
        this.modifyData.homePortType === DataMap.initHomePortType.ethernet.value
          ? this.modifyData.homePortName
          : this.modifyData?.vlan?.portNameList[0],
      mtu:
        this.modifyData.homePortType === DataMap.initHomePortType.bonding.value
          ? this.modifyData?.bondPort?.mtu
          : this.modifyData?.vlan?.mtu,
      vlanId: this.modifyData?.vlan?.tags[0]
    });
    this.vlan = !!this.modifyData?.vlan;
    this.modal.getInstance().lvOkDisabled = false;
    if (this.modifyData?.bondPort) {
      defer(() => {
        let tmpData = filter(this.tableData.data, item => {
          return this.modifyData?.bondPort.portNameList.includes(item.location);
        });
        this.dataTable.setSelections(tmpData);
        this.selectionData = [...tmpData];
      });
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(
            CommonConsts.REGEX.bondingPort,
            false
          ),
          this.baseUtilService.VALID.required(),
          this.validSameName(),
          this.validBongdingNameLength()
        ]
      }),
      role: new FormControl(DataMap.initRole.data.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      ip: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip(),
          this.validSameIp()
        ]
      }),
      ipType: new FormControl('IPV4', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      mask: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
        ]
      }),
      gateWay: new FormControl('', {
        validators: this.baseUtilService.VALID.ip()
      }),
      vlanId: new FormControl({ value: '', disabled: !!this.modifyData }),
      homePortType: new FormControl('7', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      portChoice: new FormControl('1'),
      ethPort: new FormControl(),
      mtu: new FormControl(
        { value: 1500, disabled: !!this.modifyData },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
          ]
        }
      )
    });

    if (!this.modifyData) {
      this.nameChange(DataMap.initRole.data.value);
    }
    this.watch();
  }

  initRoles() {
    const currentControllerRoleData = countBy(
      this.portData.data.filter(
        item => item.homeControllerId === this.controlType
      ),
      'role'
    );
    this.disableRoleRadio.backup =
      get(currentControllerRoleData, DataMap.initRole.data.value, 0) >= 8 ||
      !!this.modifyData;
    this.disableRoleRadio.replicate =
      get(currentControllerRoleData, DataMap.initRole.copy.value, 0) >= 8 ||
      !!this.modifyData;
    this.disableRoleRadio.archive =
      get(currentControllerRoleData, DataMap.initRole.dataManage.value, 0) >=
        4 || !!this.modifyData;
    this.formGroup
      .get('role')
      .setValue(
        !this.disableRoleRadio.backup
          ? DataMap.initRole.data.value
          : !this.disableRoleRadio.replicate
          ? DataMap.initRole.copy.value
          : DataMap.initRole.dataManage.value
      );
  }

  validVlan(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value || !this.vlan) {
        return;
      }

      const value = control.value;
      if (
        includes(this.usedVlanList, value) &&
        (this.formGroup.value.portChoice === '1' ||
          (this.formGroup.value.portChoice === '0' &&
            this.formGroup.value.homePortType ===
              DataMap.initHomePortType.bonding.value &&
            !!this.logicSelectionData.length &&
            value !== this.logicSelectionData[0]?.vlanId))
      ) {
        return { invalidName: { value: control.value } };
      }
    };
  }

  watch() {
    this.formGroup.get('role').valueChanges.subscribe(res => {
      // 根据角色默认赋值名字
      if (!this.modifyData) {
        this.nameChange(res);
      }
    });
    this.formGroup.get('homePortType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      defer(() => this.getOldLogicPortData());
      this.logicSelectionData = [];
      this.logicDataTable?.setSelections([]);
      this.usedVlanList = [];
      if (res === DataMap.initHomePortType.ethernet.value) {
        this.formGroup.get('mtu').clearValidators();
        if (!(this.vlan && this.formGroup.value.portChoice === '0')) {
          this.validTable.next(true);
          this.formGroup
            .get('ethPort')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        if (this.vlan && this.formGroup.value.portChoice === '0') {
          this.validTable.next(size(this.logicSelectionData) > 0);
        }
        if (this.vlan && this.formGroup.value.portChoice === '1') {
          this.formGroup
            .get('mtu')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
            ]);
        }
      } else {
        if (this.portLimit) {
          this.formGroup.get('portChoice').setValue('0');
        }
        this.selectionData = [];
        this.validTable.next(false);
        this.formGroup.get('ethPort').clearValidators();
        if (this.formGroup.value.portChoice === '1') {
          this.formGroup
            .get('mtu')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
            ]);
        } else {
          if (
            this.vlan &&
            !(
              !!this.logicSelectionData.length &&
              this.logicSelectionData[0].tags[0] ===
                this.formGroup.get('vlanId').value
            )
          ) {
            this.formGroup
              .get('mtu')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
              ]);
          }
        }
      }
      this.formGroup.get('ethPort').updateValueAndValidity();
      this.formGroup.get('mtu').updateValueAndValidity();
    });

    this.formGroup.get('portChoice').valueChanges.subscribe(res => {
      if (res === '0') {
        defer(() => this.logicDataTable.setSelections(this.logicSelectionData));
        this.validTable.next(size(this.logicSelectionData) > 0);
        this.formGroup.get('mtu').clearValidators();
        if (
          this.vlan &&
          this.formGroup.value.homePortType ===
            DataMap.initHomePortType.ethernet.value
        ) {
          this.formGroup.get('ethPort').clearValidators();
        }
        if (this.vlan && !!this.logicSelectionData.length) {
          this.formGroup
            .get('vlanId')
            .setValue(this.logicSelectionData[0].vlanId);
        }
        this.getOldLogicPortData();
      } else {
        if (
          this.formGroup.value.homePortType ===
          DataMap.initHomePortType.bonding.value
        ) {
          defer(() => this.dataTable.setSelections(this.selectionData));
          this.validTable.next(size(this.selectionData) > 1);
        }

        this.formGroup
          .get('mtu')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
          ]);
        if (
          this.formGroup.value.homePortType ===
          DataMap.initHomePortType.ethernet.value
        ) {
          this.validTable.next(true);
          this.formGroup
            .get('ethPort')
            .setValidators([this.baseUtilService.VALID.required()]);
          if (this.vlan) {
            this.formGroup.get('vlanId').setValue('');
          }
        }
      }
      this.formGroup.get('mtu').updateValueAndValidity();
      this.formGroup.get('ethPort').updateValueAndValidity();
    });

    this.formGroup.get('ethPort').valueChanges.subscribe(res => {
      if (!res) {
        this.usedVlanList = [];
        return;
      }
      if (this.vlan) {
        const tmpList = filter(this.data.vlanList, item => {
          return item.portNameList[0] === res;
        });
        this.usedVlanList = uniq(map(tmpList, val => val.tags[0]));
        this.formGroup.get('vlanId').updateValueAndValidity();
      }
    });

    this.formGroup.get('vlanId').valueChanges.subscribe(res => {
      if (
        this.vlan &&
        !!this.logicSelectionData.length &&
        this.logicSelectionData[0].vlanId === res &&
        this.formGroup.value.portChoice === '0'
      ) {
        this.formGroup.get('mtu').clearValidators();
      } else if (
        this.vlan &&
        (this.formGroup.value.portChoice === '1' ||
          (this.formGroup.value.portChoice === '0' &&
            !!this.logicSelectionData.length &&
            this.logicSelectionData[0].vlanId !== res &&
            this.formGroup.value.portChoice === '0'))
      ) {
        this.formGroup
          .get('mtu')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
          ]);
      }

      this.formGroup.get('mtu').updateValueAndValidity();
    });

    this.formGroup.get('ipType').valueChanges.subscribe(res => {
      if (res === DataMap.IP_Type.ipv6.value) {
        this.formGroup
          .get('ip')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.ipv6)
          ]);

        this.formGroup
          .get('mask')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 128)
          ]);
      } else {
        this.formGroup
          .get('ip')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ipv4(),
            this.validSameIp()
          ]);

        this.formGroup
          .get('mask')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
          ]);
      }
      this.formGroup.get('ip').markAsDirty();
      this.formGroup.get('ip').updateValueAndValidity();
      this.formGroup.get('mask').updateValueAndValidity();
      this.formGroup.get('mask').markAsDirty();
    });
  }

  validSameIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      for (const item of this.ipList) {
        if (control.value === item && control.value !== this.modifyData.ip) {
          return { invalidSameName: { value: control.value } };
        }
      }

      return null;
    };
  }

  validBongdingNameLength(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const str = control.value;
      if (!this.formGroup) {
        return null;
      }
      const strLength = size(str);
      const cnLength = size(str.match(/[\u4e00-\u9fa5]/g));
      const length = strLength - cnLength + cnLength * 3;
      if (length > this.MAX_LOGIC_NGNAME_LENGTH) {
        return { invalidLengthName: { value: control.value } };
      }
    };
  }

  validSameName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      for (const item of this.nameList) {
        if (control.value === item && control.value !== this.modifyData.name) {
          return { invalidSameName: { value: control.value } };
        }
      }

      return null;
    };
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'location',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        width: 135
      },
      {
        key: 'macAddress',
        name: this.i18n.get('common_mac_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'mtu',
        name: this.i18n.get('common_maximum_transmission_unit_label')
      },
      {
        key: 'maxSpeed',
        name: this.i18n.get('common_maximum_operating_speed_label')
      },
      {
        key: 'healthStatus',
        name: this.i18n.get('common_health_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          options: this.dataMapService.toArray('initHeathStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('initHeathStatus')
        }
      },
      {
        key: 'runningStatus',
        name: this.i18n.get('protection_running_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          options: this.dataMapService.toArray('initRuningStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('initRuningStatus')
        }
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'location',
        columns: cols,
        colDisplayControl: false,
        size: 'small',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          this.selectionData = selection;
          this.validTable.next(size(this.selectionData) > 1);
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showTotal: true
      }
    };

    const logicCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_business_port_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'mtu',
        name: this.i18n.get('common_maximum_transmission_unit_label')
      },
      {
        key: 'portNameList',
        name: this.i18n.get('common_ethernet_label'),
        width: 250
      }
    ];
    this.logicTableConfig = {
      table: {
        async: false,
        compareWith: 'tagID',
        columns: logicCols,
        colDisplayControl: false,
        size: 'small',
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          this.logicSelectionData = selection;
          this.validTable.next(size(this.logicSelectionData) > 0);
          if (this.vlan && !!selection.length && selection[0]?.vlanId) {
            this.formGroup.get('vlanId').setValue(selection[0].vlanId);
            const selectedName = selection[0].portNameList;
            this.formGroup.get('mtu').clearValidators();
            this.formGroup.get('mtu').updateValueAndValidity();
            const vlanPortList = filter(this.data.vlanList, item => {
              return size(intersection(item?.portNameList, selectedName)) > 0;
            });
            this.usedVlanList = uniq(map(vlanPortList, val => val?.tags[0]));
          }

          if (!selection.length || !selection[0]?.vlanId) {
            this.usedVlanList = [];
            this.formGroup.get('vlanId').setValue('');
          }
        }
      },
      pagination: null
    };

    this.getSelectOptions();
  }

  getSelectOptions() {
    each(this.data.bondPortList, item => {
      each(item.portIdList, item => {
        this.usedPortIdList.push(item);
      });
    });
    each(this.usedPortIdList, item => {
      const idx = this.ableEthPortDtoList.findIndex(port => port.id === item);
      this.ableEthPortDtoList.splice(idx, 1);
      this.ableEthPortDtoList = [...this.ableEthPortDtoList];
    });
    each(this.ableEthPortDtoList, item => {
      // 非X9000场景需要按控制器类型过滤以太网口, X9000则没有控制器概念
      if (this.confirmControlType(item)) {
        this.ethPortOptions.push({
          ...item,
          value: item.location,
          label: `${item.location}(${this.dataMapService.getLabel(
            'initRuningStatus',
            item.runningStatus
          )})`,
          isLeaf: true
        });
      }
    });
    this.ethPortOptions = [...this.ethPortOptions];
  }

  vlanChange(e) {
    if (
      this.vlan &&
      !!this.logicSelectionData.length &&
      this.logicSelectionData[0]?.vlanId
    ) {
      // 复用VLAN端口时直接匹配VLANID
      this.formGroup.get('vlanId').setValue(this.logicSelectionData[0]?.vlanId);
    }

    if (!this.vlan) {
      this.usedVlanList = [];
      if (this.logicTableConfig.table.columns.length === 4) {
        this.logicTableConfig.table.columns.splice(1, 1);
        this.logicDataTable?.reinit();
      }
      if (
        this.formGroup.value.homePortType ===
        DataMap.initHomePortType.ethernet.value
      ) {
        this.formGroup.get('mtu').clearValidators();
        this.formGroup.get('vlanId').clearValidators();
        this.validTable.next(true);
      } else {
        if (this.formGroup.value.portChoice === '0') {
          this.formGroup.get('mtu').clearValidators();
        }
        this.formGroup.get('vlanId').clearValidators();
      }
    }
    if (this.vlan) {
      if (this.logicTableConfig.table.columns.length === 3) {
        this.logicTableConfig.table.columns.splice(1, 0, {
          key: 'vlanId',
          name: 'VLAN ID',
          filter: {
            type: 'search',
            filterMode: 'contains'
          }
        });
        this.logicDataTable?.reinit();
      }
      this.formGroup
        .get('vlanId')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validVlan()
        ]);
      if (
        this.formGroup.value.homePortType ===
        DataMap.initHomePortType.ethernet.value
      ) {
        if (this.formGroup.value.portChoice === '1') {
          this.formGroup
            .get('mtu')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
            ]);
        } else {
          this.validTable.next(this.logicSelectionData.length > 0);
        }
      } else {
        if (
          !(
            this.formGroup.value.portChoice === '0' &&
            !!this.logicSelectionData.length &&
            !!this.logicSelectionData[0]?.vlanId &&
            this.logicSelectionData[0].vlanId ===
              this.formGroup.get('vlanId').value
          )
        ) {
          this.formGroup
            .get('mtu')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1280, this.mtuLimit)
            ]);
        }
      }
    }

    if (
      this.vlan &&
      this.formGroup.value.homePortType ===
        DataMap.initHomePortType.ethernet.value &&
      !!this.formGroup.value.ethPort
    ) {
      const tmpList = filter(this.data.vlanList, item => {
        return item.portNameList[0] === this.formGroup.value.ethPort;
      });
      this.usedVlanList = uniq(map(tmpList, val => val.tags[0]));
    }

    if (
      this.vlan &&
      this.formGroup.value.homePortType !==
        DataMap.initHomePortType.ethernet.value &&
      !!this.logicSelectionData.length
    ) {
      let selectedName = this.logicSelectionData[0].portNameList;
      const vlanPortList = filter(this.data.vlanList, item => {
        return size(intersection(item?.portNameList, selectedName)) > 0;
      });
      this.usedVlanList = uniq(map(vlanPortList, val => val?.tags[0]));
    }
    this.formGroup.get('mtu').updateValueAndValidity();
    this.formGroup.get('vlanId').updateValueAndValidity();
    this.getOldLogicPortData();
  }

  getParams() {
    let params: any = {};
    if (!!this.modifyData) {
      assign(params, {
        name: this.formGroup.value.name,
        id: this.modifyData.id,
        ip: this.formGroup.value.ip,
        mask: this.formGroup.value.mask,
        gateWay: this.formGroup.value.gateWay || '',
        addressFamily: this.formGroup.value.ipType === 'IPV4' ? '0' : '1'
      });
    } else {
      assign(params, {
        name: this.formGroup.value.name,
        ip: this.formGroup.value.ip,
        mask: this.formGroup.value.mask,
        gateWay: this.formGroup.value.gateWay || '',
        ipType: this.formGroup.value.ipType,
        homePortType: this.formGroup.value.homePortType,
        homePortId: '',
        homeControllerId: this.controlType,
        currentControllerId: this.controlType,
        role: String(this.formGroup.value.role)
      });
      if (
        this.formGroup.get('homePortType').value ===
        DataMap.initHomePortType.ethernet.value
      ) {
        params.homePortName = this.formGroup.value.ethPort;
        if (this.vlan) {
          params.homePortType = '8';
          if (this.formGroup.value.portChoice === '1') {
            params.vlan = {
              tags: [this.formGroup.get('vlanId').value],
              portType: this.formGroup.value.homePortType,
              portNameList: [this.formGroup.value.ethPort],
              mtu: this.formGroup.get('mtu').value
            };
          } else {
            // 以太网复用时一定复用整个VLAN
            const vlan = cloneDeep(this.logicSelectionData[0]);
            delete vlan.vlanId;
            delete vlan.tagID;
            delete vlan.name;
            delete vlan.parent;
            params.homePortId = vlan.id;
            params.vlan = vlan;
          }
        }
      } else {
        if (this.formGroup.value.portChoice === '0') {
          // 复用绑定端口
          let bondPort;
          bondPort = this.logicSelectionData[0];
          params.homePortId = bondPort.id;
          params.bondPort = {
            id: params.homePortId,
            portNameList: bondPort.portNameList,
            mtu: bondPort.mtu
          };
          if (this.vlan) {
            params.homePortType = '8';
            if (
              this.logicSelectionData[0]?.vlanId &&
              this.logicSelectionData[0]?.vlanId ===
                this.formGroup.get('vlanId').value
            ) {
              // 复用场景下复用整个VLAN
              const vlan = find(
                this.data.vlanList,
                vlan =>
                  size(
                    intersection(
                      vlan.portNameList,
                      this.logicSelectionData[0].portNameList
                    )
                  ) > 0
              );
              params.homePortId = vlan.id;
              params.vlan = vlan;
            } else {
              // 复用绑定端口但创vlan
              params.homePortId = '';
              params.vlan = {
                tags: [this.formGroup.get('vlanId').value],
                portType: this.formGroup.value.homePortType,
                bondPortId: bondPort.id,
                portNameList: bondPort.portNameList,
                mtu: this.formGroup.get('mtu').value
              };
            }
          }
        } else {
          params.bondPort = {
            id: '',
            portNameList: map(this.selectionData, item => item.location),
            mtu: this.formGroup.get('mtu').value
          };
          if (this.vlan) {
            params.homePortType = '8';
            params.vlan = {
              tags: [this.formGroup.get('vlanId').value],
              portType: this.formGroup.value.homePortType,
              portNameList: map(this.selectionData, item => {
                return item.location;
              }),
              mtu: this.formGroup.get('mtu').value
            };
          }
        }
      }
    }
    return params;
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (!!this.modifyData) {
        this.systemApiService
          .modifyLogcPort({
            name: this.modifyData.name,
            modifyLogicPortRequest: params,
            memberEsn: this.memberEsn || ''
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.systemApiService
          .addLogicPorts({
            model: params,
            memberEsn: this.memberEsn || ''
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
