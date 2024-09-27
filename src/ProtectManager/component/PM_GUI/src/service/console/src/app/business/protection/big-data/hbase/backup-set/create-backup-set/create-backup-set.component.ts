import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  CatalogName
} from 'app/shared';
import {
  each,
  isNumber,
  filter,
  size,
  includes,
  map,
  assign,
  isEmpty,
  find,
  isUndefined,
  indexOf,
  union,
  cloneDeep,
  reject
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { TablesComponent } from '../tables/tables.component';

@Component({
  selector: 'aui-create-backup-set',
  templateUrl: './create-backup-set.component.html',
  styleUrls: ['./create-backup-set.component.less']
})
export class CreateBackupSetComponent implements OnInit {
  data;
  formGroup: FormGroup;
  clusterOptions = [];
  namespaceOptions = [];
  selectedTableData = [];
  readonly HBASE_TYPE = 'HBASE';
  namespaceErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_required_label')
  };

  @ViewChild(TablesComponent, { static: false })
  tablesComponent: TablesComponent;
  @ViewChild('page', { static: false }) page;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.getClusters();
    this.initForm();
    this.updateData();
  }

  updateData() {
    if (!this.data) {
      return;
    }
    assign(this.data, {
      cluster: this.data.parentUuid
    });
    this.formGroup.patchValue(this.data);
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
      namespace: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      table: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      })
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getNameSpace(res);
    });

    this.formGroup.get('namespace').valueChanges.subscribe(res => {
      if (!res || !size(res)) {
        return;
      }
      const selectedNameSpace = filter(this.namespaceOptions, item => {
        return includes(res, item.key);
      });
      this.tablesComponent.getNameSpace(cloneDeep(selectedNameSpace));
    });
  }

  getNameSpace(clusterId, recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE,
      envId: clusterId,
      parentId: clusterId,
      resourceType: DataMap.Resource_Type.HBaseNameSpace.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) + 1 ||
          res.totalCount === 0
        ) {
          const clusterArray = [];
          each(recordsTemp, item => {
            clusterArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.extendInfo.nameSpace,
              isLeaf: true
            });
          });
          this.namespaceOptions = clusterArray;
          if (
            !this.data ||
            !this.data.extendInfo ||
            isEmpty(this.data.extendInfo.table)
          ) {
            return;
          }
          const paths = this.data.extendInfo.table.split(',');
          const nameSpaces = [];
          each(paths, path => {
            const namespace = find(this.namespaceOptions, {
              label: path.startsWith('/')
                ? path.split('/')[1]
                : path.split('/')[0]
            });
            if (isUndefined(namespace)) {
              return;
            }
            if (!includes(nameSpaces, namespace)) {
              nameSpaces.push(namespace);
              this.formGroup
                .get('namespace')
                .setValue(map(nameSpaces, 'value'));
            }
            const node = find(this.tablesComponent.tableData, {
              name: namespace.name
            });
            if (
              !node.expanded &&
              !size(node.children) &&
              indexOf(paths, node.name) === -1
            ) {
              this.tablesComponent.tableData.filter(item => {
                if (node.name === item.name) {
                  item.expanded = true;
                }
              });
              this.tablesComponent.getResource(node);
            }
            this.setTableValue(node, path);
          });
          return;
        }
        this.getNameSpace(clusterId, recordsTemp, startPage);
      });
  }

  setTableValue(node, path) {
    if (node.name === path) {
      this.tablesComponent.tableSelection = this.tablesComponent.tableSelection.concat(
        [node]
      );
      this.tablesComponent.selectionChange();
      return;
    }
    this.tablesComponent.children$.subscribe(res => {
      const selectedNode = find(res.children, { name: path });
      if (isUndefined(selectedNode)) {
        return;
      }
      this.tablesComponent.tableSelection = union(
        this.tablesComponent.tableSelection,
        [selectedNode]
      );
      this.tablesComponent.selectionChange();
    });
  }

  getClusters(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.HBase.value
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
      this.getClusters(recordsTemp, startPage);
    });
  }

  selectionChange(data) {
    this.formGroup.get('table').setValue(data);
    this.selectedTableData = cloneDeep(data);
  }

  clearSelected() {
    this.tablesComponent.tableSelection = [];
    this.tablesComponent.selectionChange();
  }

  remove(item) {
    this.tablesComponent.tableSelection = reject(
      this.tablesComponent.tableSelection,
      val => {
        if (includes(this.formGroup.value.namespace, item.key)) {
          return (
            val.name?.indexOf(`${item.name}/`) !== -1 || item.name === val.name
          );
        } else {
          if (
            item.name.indexOf(`${val.name}/`) !== -1 &&
            !val.expanded &&
            !size(val.children)
          ) {
            val.expanded = true;
            this.tablesComponent.getResource(val);
          }
          return (
            item.name === val.name || item.name.indexOf(`${val.name}/`) !== -1
          );
        }
      }
    );
    this.tablesComponent.selectionChange();
  }

  onOK(): Observable<void> {
    return this.data ? this.modify() : this.create();
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.cluster,
        subType: DataMap.Resource_Type.HBaseBackupSet.value,
        type: this.HBASE_TYPE,
        extendInfo: {
          table: map(this.formGroup.value.table, 'name').toString()
        }
      };
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
    });
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.cluster,
        subType: DataMap.Resource_Type.HBaseBackupSet.value,
        type: this.HBASE_TYPE,
        path: find(this.clusterOptions, {
          key: this.formGroup.value.cluster
        }).name,
        extendInfo: {
          table: map(this.formGroup.value.table, 'name').toString()
        }
      };
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
    });
  }
}
