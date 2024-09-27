import {
  ChangeDetectorRef,
  Component,
  QueryList,
  TemplateRef,
  ViewChild,
  ViewChildren
} from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { PopoverComponent } from '@iux/live';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  CapacityCalculateLabel,
  CommonConsts,
  CopyControllerService,
  DataMap,
  I18NService,
  isJson,
  RestoreApiV2Service,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  at,
  each,
  eq,
  every,
  get,
  groupBy,
  includes,
  indexOf,
  isEmpty,
  map,
  remove,
  some
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-single-file-restore',
  templateUrl: './oracle-single-file-restore.component.html',
  styleUrls: ['./oracle-single-file-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class OracleSingleFileRestoreComponent {
  formGroup: FormGroup;
  selectTableConfig: TableConfig;
  selectedTableConfig: TableConfig;
  resourceEnvPro;
  resourcePro;
  activeIndex = 'selecting';
  fileTypeArr = ['datafile', 'logfile', 'controlfile', 'parameterfile'];
  selectLogTableConfig: TableConfig;
  selectedLogTableConfig: TableConfig;
  tableDataArr;
  selectionData = {};
  rowCopy;
  valid$ = new Subject<boolean>();
  restoreLocationType = RestoreV2LocationType;
  restoreLocationErrorTips = {
    ...this.baseUtilService.requiredErrorTip
  };
  restorePathToolTips = this.i18n.get(
    'protection_oracle_single_file_restore_restore_path_tips_label'
  );
  @ViewChild('restoreLocationExtraTpl', { static: true })
  restoreLocationExtraTpl: TemplateRef<any>;
  @ViewChild('restoreLocationTHExtraTpl', { static: true })
  restoreLocationTHExtraTpl: TemplateRef<any>;
  @ViewChildren('selectingTable')
  selectingTableList: QueryList<ProTableComponent>;
  @ViewChildren('selectedTable')
  selectedTableList: QueryList<ProTableComponent>;
  @ViewChild('batchRestoreLocationPopover', { static: false })
  batchRestoreLocationPopover: PopoverComponent;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private capacityCalculate: CapacityCalculateLabel,
    private copyControllerService: CopyControllerService,
    private baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initData();
    this.initConfig();
    this.initForm();
    this.getCopyFileData();
  }

  initData() {
    this.resourceEnvPro = JSON.parse(this.rowCopy.resource_properties);
    this.resourcePro = JSON.parse(this.rowCopy.properties);
    if (this.rowCopy?.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      this.restorePathToolTips = this.i18n.get(
        'protection_log_file_oracle_single_file_restore_restore_path_tips_label'
      );
    }
    const tableArr = [
      {
        title: this.i18n.get('common_data_file_label'),
        id: 'data'
      },
      {
        title: this.i18n.get('common_log_file_label'),
        id: 'log'
      },
      {
        title: this.i18n.get('common_control_file_label'),
        id: 'control'
      },
      {
        title: this.i18n.get('common_param_file_label'),
        id: 'param'
      }
    ];
    this.tableDataArr = map(tableArr, item => ({
      ...item,
      data: {
        data: [],
        total: 0
      },
      activeIndex: 'selecting',
      selectedLength: 0
    }));
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        onClick: (data, ...args) => {
          this.deleteItem(data);
        }
      }
    ];
    const cols: { [key: string]: TableCols } = {
      name: {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        sort: true
      },
      size: {
        key: 'sizeLabel',
        width: '100px',
        name: this.i18n.get('common_size_label')
      },
      location: {
        key: 'restoration',
        name: this.i18n.get('protection_recovery_target_pvc_label'),
        thExtra: this.restoreLocationTHExtraTpl,
        cellRender: this.restoreLocationExtraTpl
      },
      startSCN: {
        key: 'startSCN',
        name: this.i18n.get('protection_start_scn_label'),
        width: '100px',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      endSCN: {
        key: 'endSCN',
        name: this.i18n.get('protection_end_scn_label'),
        width: '100px',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      op: {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: '64px',
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: opts
          }
        }
      }
    };

    const baseCols = ['name', 'size'];
    const basicConfig: TableConfig = {
      table: {
        async: false,
        compareWith: 'path',
        columns: this.getTableColsById(cols, [...baseCols, 'location', 'op']),
        showLoading: false,
        colDisplayControl: false,
        showAnimation: true,
        size: 'small',
        rows: {
          showSelector: false
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE
      }
    };
    this.selectedTableConfig = basicConfig;
    this.selectTableConfig = assign({}, basicConfig, {
      table: {
        ...basicConfig.table,
        columns: this.getTableColsById(cols, [...baseCols, 'location']),
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: () => {
          this.getSelectionData();
          this.setValid();
        }
      }
    });
    this.selectLogTableConfig = assign({}, this.selectTableConfig, {
      table: {
        ...this.selectTableConfig.table,
        columns: this.getTableColsById(cols, [
          ...baseCols,
          'startSCN',
          'endSCN',
          'location'
        ])
      }
    });
    this.selectedLogTableConfig = assign({}, this.selectedTableConfig, {
      table: {
        ...this.selectedTableConfig.table,
        columns: this.getTableColsById(cols, [
          ...baseCols,
          'startSCN',
          'endSCN',
          'location',
          'op'
        ])
      }
    });
  }

  getTableColsById(opColsMap, keys: string[]): TableCols[] {
    return at(opColsMap, keys);
  }

  getSelectTableConfig(item): TableConfig {
    return item.id === 'log'
      ? this.selectLogTableConfig
      : this.selectTableConfig;
  }

  getSelectedTableConfig(item): TableConfig {
    return item.id === 'log'
      ? this.selectedLogTableConfig
      : this.selectedTableConfig;
  }

  openPopover(index) {
    // 记录当前是在第几个文件点击的批量操作
    this.formGroup.get('activeIndex').setValue(index);
    this.formGroup.get('batchRestoreLocationName').setValue('');
  }

  hideBatchRestoreLocationPopover() {
    this.batchRestoreLocationPopover?.hide();
    this.cdr.detectChanges();
  }

  batchSetRestoreLocation(isAll = false) {
    // isAll为true时对所有数据做操作
    // 为false时，仅对选中的数据做操作
    const index = this.formGroup.get('activeIndex').value;
    const targetValue = this.formGroup.get('batchRestoreLocationName').value;
    const rowData = isAll
      ? this.tableDataArr[index].data.data
      : this.selectionData[index].data;
    each(rowData, item => {
      item.restoreLocation.setValue(targetValue);
      item.restoreLocation.updateValueAndValidity();
    });
    this.setValid();
    this.hideBatchRestoreLocationPopover();
  }

  deleteItem([data]) {
    const index = data.tableId;
    data.restoreLocation.clearValidators(); // 删除数据时，需要清除对应的校验
    data.restoreLocation.updateValueAndValidity();
    const rowData = this.selectionData[index].data;
    remove(rowData, item => eq(item['path'], data.path));
    this.selectedTableList.get(index).tableData = {
      data: [...rowData],
      total: rowData.length
    };
    this.tableDataArr[index].selectedLength = rowData.length;
    this.selectingTableList.get(index).setSelections([...rowData]);
    this.cdr.detectChanges();
    this.setValid();
  }

  deleteAllItems(index: number) {
    this.selectingTableList.get(index).table.clearSelection(); // 调用pro-table内部的table函数，同时触发selectionChange
  }

  resetAllRestoreLocation(index) {
    this.formGroup.get('activeIndex').setValue(index); // 重置就是针对所有数据调用一次批量操作
    this.formGroup.get('batchRestoreLocationName').setValue('');
    this.batchSetRestoreLocation(true);
  }

  getSelectionData() {
    this.selectingTableList.forEach((component: ProTableComponent, index) => {
      const selection = component.getAllSelections();
      // 只有日志副本需要对所有的文件目标路径做校验
      if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
        this.addControlToLogCopy(index, selection);
      }
      this.selectionData[index] = {
        data: selection,
        total: selection.length
      };
      this.tableDataArr[index].selectedLength = selection.length;
    });
  }

  private addControlToLogCopy(index: number, selection) {
    each(this.tableDataArr[index].data.data, item => {
      if (includes(selection, item)) {
        item.restoreLocation.addValidators([
          this.baseUtilService.VALID.required()
        ]);
      } else {
        item.restoreLocation.clearValidators();
      }
      item.restoreLocation.updateValueAndValidity();
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: [RestoreV2LocationType.ORIGIN],
      dbConfig: this.fb.array([]),
      location: [
        {
          value: `${this.resourceEnvPro.environment_name}/${this.rowCopy.resource_name}`,
          disabled: true
        }
      ],
      activeIndex: new FormControl(0), // 当前所在的文件列表号
      batchRestoreLocationName: new FormControl('')
    });
  }

  get dbConfig() {
    return (this.formGroup.get('dbConfig') as FormArray).controls;
  }

  checkValid(data) {
    data.restoreLocation.updateValueAndValidity();
    this.setValid();
  }

  showErrorTipsLabel(ctrl: FormControl, errorTip) {
    return errorTip[Object.keys(ctrl.errors)[0]];
  }

  getCopyFileData() {
    const targetPath = `/`;
    const extParams = {
      copyId: this.rowCopy.uuid,
      parentPath: targetPath,
      conditions: JSON.stringify({
        tableName: 'ORACLE_COPY_METADATA'
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.copyControllerService.ListCopyCatalogs(param),
      res => {
        const filterArr = map(res, (item, index) => {
          const extStr = get(item, 'extendInfo');
          const extObj = isJson(extStr) ? JSON.parse(extStr) : {};
          if (extObj.type === 'logfile' && !!extObj.scn) {
            const scn = extObj.scn.split('-');
            assign(item, {
              startSCN: scn[0],
              endSCN: scn[1]
            });
          }
          assign(item, {
            name: item.path,
            sizeLabel: this.capacityCalculate.transform(
              item.size,
              '1.0-3',
              CAPACITY_UNIT.BYTE
            ),
            fileType: extObj.type,
            catalogue: extObj.catalogue,
            tableId: indexOf(this.fileTypeArr, extObj.type),
            index,
            restoreLocation: new FormControl('')
          });
          return item;
        });
        const groupedArr = groupBy(filterArr, 'fileType');
        each(this.fileTypeArr, (item, index) => {
          this.tableDataArr[index].data = {
            data: groupedArr[item],
            total: groupedArr[item]?.length || 0
          };
        });
      }
    );
  }

  getParams() {
    const param = {
      copyId: this.rowCopy.uuid,
      targetEnv: this.resourceEnvPro.environment_uuid,
      restoreType: RestoreV2Type.FileRestore,
      targetLocation: RestoreV2LocationType.ORIGIN,
      targetObject: this.resourceEnvPro.uuid
    };
    const restoreTargetHost = {};
    const dbConfigControls = this.dbConfig;
    dbConfigControls.forEach(control => {
      restoreTargetHost[control.value.key] = control.value.newParam
        ? control.value.newParam
        : control.value.originParam;
    });
    const restoreFiles = map(this.selectionData, ({ data }, key) => {
      return {
        filterTpe: this.fileTypeArr[key],
        files: map(data, (item: any) => ({
          name: get(item, 'catalogue', item['path']),
          path: item.restoreLocation.value
        }))
      };
    });
    assign(param, {
      extendInfo: {
        restoreFiles: JSON.stringify(restoreFiles),
        RESTORE_TARGET_HOST: isEmpty(restoreTargetHost)
          ? ''
          : JSON.stringify(restoreTargetHost),
        isModifyDBConfig: this.formGroup.get('isModify').value,
        isSingleFileRestore: true // 为了job-table区分表级恢复和单文件级恢复
      }
    });
    return param;
  }

  setValid() {
    /*
     * 1、选中的数据不能为空
     * 2、每种类型选中的文件都要通过校验
     * */
    this.valid$.next(
      some(this.tableDataArr, item => item.selectedLength > 0) &&
        every(this.selectionData, (item: any) =>
          every(item.data, val => val.restoreLocation.valid)
        )
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
    });
  }
}
