import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormGroup, ValidatorFn } from '@angular/forms';
import { TransferState } from '@angular/platform-browser';
import {
  ModalRef,
  PageConfig,
  PaginatorComponent,
  TransferColumnItem,
  TransferComponent
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CopyControllerService,
  DataMap,
  I18NService,
  RestoreV2LocationType,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, defer, find, get, isString, map, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-restore',
  templateUrl: './user-level-restore.component.html',
  styleUrls: ['./user-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class UserLevelRestoreComponent implements OnInit {
  @Input() rowCopy: any;
  @Input() childResType: string;
  @Input() restoreType: string;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  dataMap = DataMap;
  hostAndDAGOptions = [];
  sourceData = [];
  sourceSelection = [];
  commonConsts = CommonConsts;
  targetData = [];
  selectionData = [];
  totalPage: number = 0;
  sourceColumns: TransferColumnItem[] = [];
  targetColumns: TransferColumnItem[] = [];
  rowCopyResourceProperties: any;
  rowCopyProperties: any;
  showDatabaseTable = true;
  scriptPlaceholder = this.i18n.get(
    'protection_fileset_advance_script_windows_label'
  );
  scriptToolTip = this.i18n.get('common_script_agent_windows_position_label');
  disableOriginLocation = false;
  disableNewLocation = false;
  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };
  pathErrorTip = {
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  @ViewChild('lvTransfer', { static: false })
  lvTransfer: TransferComponent;
  @ViewChild('pageS', { static: false }) pageS: PaginatorComponent;
  @ViewChild('pageT', { static: false }) pageT: PaginatorComponent;
  constructor(
    public baseUtilService: BaseUtilService,
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private copyControllerService: CopyControllerService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initColumns();
    this.initForm();
    this.getTargetHosts();
  }

  initConfig() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status ===
        DataMap.Resource_Status.notExist.value ||
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value;
    this.scriptPlaceholder = this.i18n.get(
      'protection_fileset_advance_script_windows_label'
    );
    this.rowCopyResourceProperties = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.rowCopyProperties = isString(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties)
      : {};
    // exchange不支持2010版本及以前的版本的新位置恢复，微软官方定义的2010版本号为14开头
    this.disableNewLocation =
      get(this.rowCopyProperties, 'version', '').split('.')[0] <= '14';
    this.showDatabaseTable =
      this.rowCopy.backup_type !== DataMap.CopyData_Backup_Type.log.value;
    if (this.showDatabaseTable) {
      this.getDatabases();
    }
  }

  initColumns() {
    this.sourceColumns = [
      {
        key: 'name',
        label: this.i18n.get('common_username_label')
      },
      {
        key: 'environment_name',
        label: this.i18n.get('protection_host_database_name_label')
      }
    ];
    this.targetColumns = [
      {
        key: 'name',
        label: this.i18n.get('common_username_label')
      }
    ];
  }

  initForm() {
    const scriptValidator: ValidatorFn[] = [
      this.baseUtilService.VALID.maxLength(8192),
      this.baseUtilService.VALID.name(CommonConsts.REGEX.windowsScript, false)
    ];
    this.formGroup = this.fb.group({
      restoreLocation: [RestoreV2LocationType.ORIGIN],
      originLocation: [
        {
          value: '',
          disabled: true
        }
      ],
      host: [
        { value: '', disabled: true },
        [this.baseUtilService.VALID.required()]
      ],
      preScript: ['', scriptValidator],
      postScript: ['', scriptValidator],
      executeScript: ['', scriptValidator]
    });
    this.listenForm();
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    });
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('');

      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
      } else {
        this.formGroup.get('host').enable();
      }
    });
  }

  change(data) {
    defer(() => {
      this.targetData = data.sourceSelection;
      this.cdr.detectChanges();
    });
  }

  selectionChange(data) {
    this.selectionData = data.selection;
    this.cdr.detectChanges();
  }

  stateChange(e: TransferState) {
    // 表头筛选条件发生改变
    const belong = get(e, 'belong');
    const filters = get(e, 'params.filters');
    let filteredData = [];
    // 后续实现key的搜索
    this.cdr.detectChanges();
  }

  sourcePageChange(page) {
    this.getDatabases(page);
  }

  getDatabases(page?: PageConfig) {
    const params = {
      pageNo: page ? page.pageIndex : CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      copyId: this.rowCopy.uuid,
      parentPath: `/${this.rowCopyResourceProperties?.name}`
    };
    this.copyControllerService.ListCopyCatalogs(params).subscribe(res => {
      this.totalPage = res.totalCount;
      this.sourceData = map(res.records, (item, index) => {
        assign(item, {
          environment_name: this.rowCopyResourceProperties.name,
          name: get(item, 'path', ''),
          key: get(item, 'extendInfo', index)
        });
        return item;
      });
      this.cdr.detectChanges();
    });
  }

  getTargetHosts() {
    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify({
        subType: [
          `${DataMap.Resource_Type.ExchangeSingle.value}`,
          `${DataMap.Resource_Type.ExchangeGroup.value}`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.hostAndDAGOptions = map(resource, item => ({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item?.path})`,
          isLeaf: true
        })).filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        if (!this.disableOriginLocation) {
          const originalHost = find(this.hostAndDAGOptions, {
            uuid: this.rowCopyResourceProperties?.environment_uuid
          });
          this.formGroup
            .get('originLocation')
            .setValue(
              originalHost?.label ||
                this.rowCopyResourceProperties?.environment_name
            );
        }
      }
    );
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.formGroup.getRawValue().originLocation
      : find(this.hostAndDAGOptions, { uuid: this.formGroup.value.host })[
          'label'
        ];
  }

  getParams() {
    const { restoreLocation, host } = this.formGroup.value;
    const params = {
      copyId: this.rowCopy.uuid,
      restore_location: restoreLocation,
      targetEnv:
        restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.rowCopyResourceProperties?.environment_uuid
          : host,
      restoreType: this.restoreType,
      targetLocation: restoreLocation,
      agents: [],
      extendInfo: {
        restore_location: restoreLocation
      },
      scripts: {
        preScript: trim(this.formGroup.value.preScript),
        postScript: trim(this.formGroup.value.postScript),
        failPostScript: trim(this.formGroup.value.executeScript)
      },
      subObjects: map(this.selectionData, item =>
        JSON.stringify({
          uuid: item.key,
          name: item.name
        })
      )
    };
    return params;
  }

  restore(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
    });
  }
}
