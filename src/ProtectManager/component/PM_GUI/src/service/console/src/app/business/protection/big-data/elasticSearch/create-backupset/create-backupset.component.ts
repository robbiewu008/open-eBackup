import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CatalogName,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  has as _has,
  isArray,
  isEmpty,
  isNumber,
  map,
  reject,
  replace,
  size,
  split,
  startsWith
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { SelectTableComponent } from './select-table/select-table.component';

@Component({
  selector: 'aui-create-backupset',
  templateUrl: './create-backupset.component.html',
  styleUrls: ['./create-backupset.component.less']
})
export class CreateBackupsetComponent implements OnInit {
  formGroup: FormGroup;
  fakeCluster;
  modifiedTables;
  clusterOptions = [];
  databaseOptions = [];
  metadataPathData = [];
  clusterData = [];
  clusterUuid = '';
  selectedDatabaseTables = [];
  isAutoProtectDisabled = true;
  protectDatabaseHelp = this.i18n.get(
    'protection_es_auto_protect_table_help_label'
  );

  @Input('data') data;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getCluster();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      autoProtect: new FormControl(true)
    });
    this.listenForm();

    if (this.data && this.data.extendInfo.indexList !== '*') {
      this.formGroup.get('autoProtect').setValue(false);
    }
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (res === '') {
        return;
      }

      this.clusterData = filter(this.clusterOptions, item => {
        return item.value === res;
      });

      const selectCluster = cloneDeep(find(this.clusterOptions, { uuid: res }));
      if (selectCluster) {
        assign(selectCluster, {
          label: selectCluster?.name,
          children: [],
          isLeaf: false
        });
        this.metadataPathData = [selectCluster];
        this.clusterUuid = selectCluster.uuid;

        if (selectCluster.extendInfo['auto-protection'] !== 'true') {
          this.formGroup.get('autoProtect').setValue(false);
        }
      }
    });
  }

  getCluster() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.Elasticsearch.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      this.fakeCluster = first(res.records).uuid;
      this.getClusters();
    });
  }

  getClusters(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.data?.rootUuid || this.fakeCluster,
      parentId: this.data?.rootUuid || '',
      resourceType: 'ElasticsearchCluster'
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
          if (this.data) {
            this.modifiedTables = this.data.extendInfo.indexList;
            this.formGroup.patchValue({
              name: this.data.name,
              cluster: this.data.rootUuid
            });
          }
          return;
        }
        this.getClusters(recordsTemp, startPage);
      });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid ||
      (!this.formGroup.value.autoProtect &&
        !size(this.selectedDatabaseTables) &&
        !this.modifiedTables);
  }

  selectTable(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('protection_select_es_index_tip_label'),
      lvContent: SelectTableComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.largeWidth - 20,
      lvComponentParams: {
        data: assign(
          {
            clusterId: this.formGroup.value.cluster,
            backupSetId: this.data?.uuid
          },
          data
        ),
        selectedTableData: this.selectedDatabaseTables,
        modifiedTables: this.data
          ? split(this.data.extendInfo.indexList, ',')
          : []
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as SelectTableComponent;
        this.selectedDatabaseTables = content.onOK();
        this.disableOkBtn();
      }
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const tempPath: any = first(this.formGroup.value.metadataPath) || {};
      const params = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.cluster,
        subType: DataMap.Resource_Type.ElasticsearchBackupSet.value,
        type: DataMap.Resource_Type.Elasticsearch.value,
        extendInfo: {
          indexList: this.formGroup.value.autoProtect
            ? '*'
            : !!size(this.selectedDatabaseTables)
            ? map(this.selectedDatabaseTables, 'name').join(',')
            : this.modifiedTables
        }
      };
      if (this.data) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.data.uuid,
            UpdateResourceRequestBody: params
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
        this.protectedResourceApiService
          .CreateResource({ CreateResourceRequestBody: params })
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
