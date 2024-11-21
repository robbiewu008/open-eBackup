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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  FilterType,
  TransferColumnItem,
  TransferComponent,
  TransferTableComponent
} from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  cloneDeep,
  filter,
  find,
  get,
  isEmpty,
  map,
  remove,
  trim,
  uniqBy,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-pdb',
  templateUrl: './register-pdb.component.html',
  styleUrls: ['./register-pdb.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class RegisterPdbComponent implements OnInit {
  @ViewChild('lvTransfer', { static: true }) lvTransfer: TransferComponent;
  formGroup: FormGroup;
  data: any;
  databaseOptions = [];
  instanceOptions = [];
  source: {
    cols: TransferColumnItem[];
    total: number;
    data: any[];
    selection: any[];
  } = {
    cols: [],
    total: 0,
    data: [],
    selection: []
  };
  cacheSourceData = [];
  selection = [];
  target: { cols: TransferColumnItem[] } = { cols: [] };
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  isCluster = false;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private appService: AppService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}
  ngOnInit() {
    this.initTransfer();
    this.initForm();
    this.getDatabase();
  }

  initTransfer() {
    const sourceColumns = [
      {
        key: 'label',
        label: this.i18n.get('common_name_label'),
        disabled: true,
        isHidden: false,
        filterType: FilterType.SEARCH
      }
    ];

    this.source.cols = sourceColumns;
    this.target.cols = cloneDeep(sourceColumns);
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: [
        this.data?.name || '',
        [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      ],
      database: ['', [this.baseUtilService.VALID.required()]],
      pdb_s: [
        [],
        [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      ],
      host: ['', [this.baseUtilService.VALID.required()]] // 集群数据库需要选下面的主机才能查pdb
    });
    this.listenForm();
  }

  listenForm() {
    this.databaseValueChange();
    this.nodeValueChange();
  }

  private databaseValueChange() {
    this.formGroup.get('database').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('');
      if (!res) {
        this.isCluster = false;
        this.formGroup.get('host').disable({ emitEvent: false });
        this.instanceOptions = [];
        return;
      }
      const target = find(this.databaseOptions, { value: res });
      if (target.subType === DataMap.Resource_Type.oracleCluster.value) {
        this.isCluster = true;
        this.formGroup.get('host').enable({ emitEvent: false });
        this.instanceOptions = map(
          JSON.parse(target.extendInfo.instances),
          inst => ({
            ...inst,
            key: inst.hostId,
            value: inst.hostId,
            label: inst.inst_name,
            isLeaf: true
          })
        );
      } else {
        this.instanceOptions = [];
        this.isCluster = false;
        this.formGroup.get('host').disable();
        this.getPDBs(target);
      }
    });
  }

  private nodeValueChange() {
    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.formGroup.get('pdb_s').setValue([]);
      this.source.data = [];
      this.source.selection = [];
      if (!res) {
        return;
      }
      const targetDatabase = find(this.databaseOptions, {
        value: this.formGroup.get('database').value
      });
      const targetInstance = find(this.instanceOptions, { value: res });
      this.getPDBs(targetDatabase, targetInstance);
    });
  }

  updateData() {
    this.formGroup.get('database').setValue(this.data?.parentUuid);
    this.formGroup.get('host').setValue(this.data?.extendInfo?.hostId);
  }

  selectionChange(event) {
    this.formGroup.get('pdb_s').setValue(event.sourceSelection);
  }

  selectCurrentPage(selected, panel: TransferTableComponent): void {
    const renderData = panel.lvTable.renderData.filter(item => !item.disabled);
    const selection = uniqBy([...selected, ...renderData], 'key');
    panel.selectionChange(selection);
  }

  clearCurrentPage(panel: TransferTableComponent): void {
    const { renderData } = panel.lvTable;
    panel.removeItem(renderData);
  }

  selectAll(data, panel: TransferTableComponent): void {
    const table = panel.lvTable;
    const filters = table.datatableService.filterMap;
    if (filters.size) {
      data = table.filteredData;
    }
    panel.selectionChange(data.filter(item => !item.disabled));
  }

  clearAll(data, panel: TransferTableComponent): void {
    const table = panel.lvTable;
    const filters = table.datatableService.filterMap;
    if (filters.size) {
      data = table.filteredData;
    }
    this.lvTransfer.removeItem(data);
  }

  getDatabase() {
    const extParams = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      queryDependency: true,
      conditions: JSON.stringify({
        subType: [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.protectedResourceApiService.ListResources(param),
      resource => {
        let filterArr = resource;
        if (!this.data) {
          filterArr = resource.filter(
            item =>
              item.extendInfo.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
        }
        this.databaseOptions = map(filterArr, item => ({
          ...item,
          linkStatus: item.extendInfo.linkStatus,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.environment.endpoint})`,
          isLeaf: true
        }));
        if (!!this.data) {
          this.updateData();
        }
      }
    );
  }

  getPDBs(database, node?) {
    // 接口数据结果只有一条，数据的extendInfo里携带了所有的pdb信息
    // 不需要做分页
    this.source.data = [];
    this.source.selection = [];
    const params = {
      pageNo: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_START_EXTRA,
      envId: database.rootUuid,
      agentId: node ? node.hostId : database.rootUuid,
      resourceIds: [database.uuid],
      appType: DataMap.Resource_Type.oracle.value,
      conditions: JSON.stringify({
        queryType: 'pdb',
        hostId: node ? node.hostId : database.rootUuid
      })
    };
    this.appService.ListResourcesDetails(params).subscribe({
      next: res => {
        if (isEmpty(res.records)) {
          return;
        }
        const tableSpaceInfo = JSON.parse(
          get(res.records[0].extendInfo, 'pdb', '{}')
        );
        const pdbs = get(tableSpaceInfo, 'pdb_names', []);
        this.source.data = pdbs.map(item => ({
          key: uniqueId(),
          value: item,
          label: item
        }));
        this.cacheSourceData = this.source.data;
        if (!!this.data) {
          const copyPdb = JSON.parse(this.data?.extendInfo.pdb || '[]');
          this.source.selection = filter(this.source.data, item =>
            copyPdb.includes(item.value)
          );
          this.formGroup.get('pdb_s').setValue(this.source.selection);
        }
        this.cdr.detectChanges();
      },
      error: err => {
        this.source.data = [];
        this.source.selection = [];
        this.formGroup.get('pdb_s').setValue([]);
      }
    });
  }

  searchByName(event) {
    if (isEmpty(trim(event))) {
      this.source.data = [...this.cacheSourceData];
      return;
    }
    const filterData = this.cacheSourceData.filter(item => {
      return item.label.includes(event);
    });
    this.source.data = filterData;
    this.cdr.detectChanges();
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.oraclePDB.value,
      parentUuid: this.formGroup.value.database,
      extendInfo: {
        dbUuid: this.formGroup.value.database,
        isPdb: this.source.selection?.length === this.source.data.length,
        pdb: JSON.stringify(this.source.selection.map(item => item.label)),
        hostId: this.isCluster
          ? this.formGroup.value.host
          : find(this.databaseOptions, { value: this.formGroup.value.database })
              .rootUuid
      }
    };

    return params;
  }

  getRegisterObserver(params) {
    if (!!this.data) {
      return this.protectedResourceApiService.UpdateResource({
        resourceId: this.data?.uuid,
        UpdateResourceRequestBody: params
      });
    } else {
      return this.protectedResourceApiService.CreateResource({
        CreateResourceRequestBody: params
      });
    }
  }

  onOK(): Observable<any> {
    const params = this.getParams();
    return new Observable<any>((observer: Observer<any>) => {
      this.getRegisterObserver(params).subscribe({
        next: res => {
          cacheGuideResource(res);
          observer.next(res);
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
