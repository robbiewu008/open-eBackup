import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  FilterType,
  TransferColumnItem,
  TransferComponent,
  TransferTableComponent
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  VirtualResourceService,
  VmRestoreOptionType
} from 'app/shared';
import { assign, each, find, includes, isEmpty, size } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-create-group',
  templateUrl: './create-group.component.html',
  styleUrls: ['./create-group.component.less']
})
export class CreateGroupComponent implements OnInit {
  includes = includes;
  sourceColumns: TransferColumnItem[] = [];
  sourceData = [];
  sourceSelection = [];
  targetColumns: TransferColumnItem[] = [];
  formGroup: FormGroup;
  selectValid$ = new Subject<boolean>();
  dataMap = DataMap;
  total = 0;
  @Input() rowData;
  @Input() treeSelection;
  @Input() subUnitType;
  groupNameLabel = this.i18n.get('protection_vm_group_name_label');
  curTreeSelection;
  vCenterOptions = [];
  treeData = [];
  treeSelection2;

  @ViewChild('transfer') transfer: TransferComponent;
  constructor(
    public baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private virtualResourceService: VirtualResourceService,
    public fb: FormBuilder,
    public i18n: I18NService
  ) {}

  ngOnInit(): void {
    this.curTreeSelection = this.treeSelection;
    this.vCenterOptions = [
      {
        key: this.treeSelection.uuid,
        label: this.treeSelection.name,
        isLeaf: true
      }
    ];
    if (
      includes(
        [
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value
        ],
        this.subUnitType
      )
    ) {
      this.groupNameLabel = this.i18n.get('protection_cloud_group_name_label');
    }
    this.initColumns();
    this.initForm();
    this.initData();
  }

  initColumns() {
    this.sourceColumns = [
      {
        key: 'uuid',
        label: this.i18n.get('protection_resource_id_label'),
        disabled: true,
        isHidden: false,
        filterType: FilterType.SEARCH
      },
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        disabled: true,
        isHidden: false,
        filterType: FilterType.SEARCH
      },
      {
        key: 'protectionStatusLabel',
        label: this.i18n.get('protection_protected_status_label'),
        width: this.i18n.isEn ? 'auto' : '100px',
        disabled: false,
        isHidden: false,
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.dataMapService.toArray('Protection_Status')
      }
    ];

    this.targetColumns = [
      {
        key: 'uuid',
        label: this.i18n.get('protection_resource_id_label'),
        disabled: true,
        isHidden: false
      },
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        disabled: true,
        isHidden: false
      }
    ];
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(this.rowData ? this.rowData[0].name : '', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      selectedVCenter: new FormControl('')
    });
    if (!!this.rowData) {
      const data = [];
      each(this.rowData[0]?.resourceGroupMembers, (item: any) => {
        delete item.uuid;
        data.push({
          uuid: item.sourceId,
          name: item.sourceName,
          path: item.path,

          disabled:
            item.protectionStatus === DataMap.Protection_Status.protected.value,
          ...item
        });
      });
      this.sourceSelection = data;
    }

    this.formGroup.get('selectedVCenter').valueChanges.subscribe(res => {
      if (!res) return;
      this.treeData = [];
      this.getVcenterChangeData(CommonConsts.PAGE_START, res);
    });
  }

  initData(e?) {
    if (!this.treeSelection) {
      return;
    }

    const params = {
      pageNo: e?.paginator?.pageIndex || CommonConsts.PAGE_START,
      pageSize: e?.paginator?.pageSize || CommonConsts.PAGE_SIZE
    };

    let path = '';
    path = e?.path || this.curTreeSelection?.path;
    const defaultConditions = {
      subType: [this.subUnitType],
      rootUuid: this.treeSelection?.rootUuid,
      path: [['=~'], path]
    };
    if (this.subUnitType === DataMap.Resource_Type.hyperVVm.value) {
      delete defaultConditions?.path;
      if (e?.parentUuid) {
        delete defaultConditions?.rootUuid;
        assign(defaultConditions, {
          parentUuid: e?.uuid
        });
      }
    }

    if (!isEmpty(e?.filters)) {
      if (!isEmpty(e.filters.protectionStatusLabel)) {
        assign(defaultConditions, {
          protectionStatus: e.filters.protectionStatusLabel
        });
      }
      if (!isEmpty(e.filters.name)) {
        assign(defaultConditions, { name: [['~~'], e.filters.name] });
      }
      if (!isEmpty(e.filters.uuid)) {
        assign(defaultConditions, { uuid: [['~~'], e.filters.uuid] });
      }
    }

    if (this.subUnitType === DataMap.Resource_Type.FusionCompute.value) {
      assign(defaultConditions, { type: ResourceType.VM });
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      const data = [];
      each(res.records, (item: any) => {
        data.push({
          id: item.uuid,
          name: item.name,
          path: item.path,
          protectionStatus: item.protectionStatus,
          protectionStatusLabel: this.dataMapService.getLabel(
            'Protection_Status',
            item.protectionStatus
          ),
          disabled:
            item.protectionStatus ===
              DataMap.Protection_Status.protected.value || item.inGroup,
          ...item
        });
      });

      this.sourceData = data;
      this.total = res.totalCount;
    });
  }

  stateChange(e) {
    this.initData(e.params);
  }

  change(e) {
    this.sourceSelection = e.selection;
    if (!this.rowData) {
      this.selectValid$.next(!!size(this.sourceSelection));
    }
  }

  clearAll(data, panel: TransferTableComponent): void {
    this.sourceSelection = [];
    if (!this.rowData) {
      this.selectValid$.next(false);
    }
  }

  changeLocation(event) {
    this.curTreeSelection = event || this.treeSelection;
    this.initData(this.curTreeSelection);
  }

  vCenterChange(event) {
    this.treeData = [];
    this.getVcenterChangeData(CommonConsts.PAGE_START, event);
  }

  getVcenterChangeData(startPage, event) {
    if (
      includes([DataMap.Resource_Type.virtualMachine.value], this.subUnitType)
    ) {
      this.getV1TreeData(startPage, event);
    } else {
      this.getV2TreeData(startPage, event);
    }
  }

  getV2TreeData(startPage, event) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        conditions: JSON.stringify({
          parentUuid: event
        })
      })
      .subscribe(res => {
        res.records.forEach(item => {
          const node = {
            label: item.name,
            name: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            uuid: item.uuid,
            path: item.path,
            subType: item.subType,
            parentUuid: item?.parentUuid,
            children: [],
            isLeaf: false
          };
          this.treeData.push(node);
        });
        startPage++;
        if (
          res.totalCount - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] >
          0
        ) {
          this.getVcenterChangeData(startPage, event);
          return;
        }
        this.treeData = [...this.treeData];
      });
  }

  getV1TreeData(startPage, event) {
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          parent_uuid: event
        })
      })
      .subscribe(res => {
        res.items.forEach(item => {
          const node = {
            label: item.name,
            name: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type as string,
            uuid: item.uuid,
            path: item.path,
            subType: item.sub_type,
            children: [],
            isLeaf: false
          };
          this.treeData.push(node);
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] > 0) {
          this.getVcenterChangeData(startPage, event);
          return;
        }
        this.treeData = [...this.treeData];
      });
  }

  getResourceIcon(node) {
    if (includes([DataMap.Resource_Type.cNwareVm.value], this.subUnitType)) {
      switch (node.subType) {
        case ResourceType.CNWARE:
          return node.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
            ? 'aui-icon-vCenter'
            : 'aui-icon-vCenter-offine';
        case DataMap.Resource_Type.cNwareHostPool.value:
          return 'aui-icon-host-pool';
        case DataMap.Resource_Type.cNwareHostPool:
          return 'aui-icon-dataCenter';
        case DataMap.Resource_Type.cNwareCluster.value:
          return 'aui-icon-cluster';
        case DataMap.Resource_Type.cNwareHost.value:
          return 'aui-icon-host';
        default:
          return 'aui-icon-host';
      }
    } else {
      const nodeResource = find(
        DataMap.Resource_Type,
        item => item.value === (node.sub_type || node.type)
      );
      return nodeResource['icon'] + '';
    }
  }

  expandedChange(event) {
    if (event.children.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(CommonConsts.PAGE_START, event);
  }

  getExpandedChangeData(startPage, event) {
    if (
      includes([DataMap.Resource_Type.virtualMachine.value], this.subUnitType)
    ) {
      this.getV1ExpendData(startPage, event);
    } else {
      this.getV2ExpendData(startPage, event);
    }
  }

  getV2ExpendData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        parentUuid: event.uuid
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.forEach(item => {
        if (item.type !== ResourceType.PLATFORM) {
          if (item.type === 'VM') {
            return;
          }
          const node = {
            label: item.name,
            contentToggleIcon: this.getResourceIcon(item),
            type: item.type,
            rootUuid: item.rootUuid,
            uuid: item.uuid,
            path: item.path,
            children: [],
            isLeaf: this.isLeaf(item)
          };
          event.children.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  getV1ExpendData(startPage, event) {
    this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
        pageNo: startPage,
        conditions: JSON.stringify({
          parent_uuid: event.uuid
        })
      })
      .subscribe(res => {
        res.items.forEach(item => {
          if (item.type !== ResourceType.VM) {
            const node = {
              label: item.name,
              contentToggleIcon: this.getResourceIcon(item),
              type: item.type as string,
              rootUuid: item.root_uuid,
              uuid: item.uuid,
              path: item.path,
              children: [],
              isLeaf: this.isLeaf(item)
            };
            event.children.push(node);
          }
        });
        startPage++;
        if (res.total - startPage * CommonConsts.PAGE_SIZE_OPTIONS[2] > 0) {
          this.getExpandedChangeData(startPage, event);
          return;
        }
        this.treeData = [...this.treeData];
      });
  }

  isLeaf(node) {
    if (includes([DataMap.Resource_Type.cNwareVm.value], this.subUnitType)) {
      return includes([DataMap.Resource_Type.cNwareHost.value], node.subType);
    } else {
      return node.type === ResourceType.VM;
    }
  }

  getRootNode(node) {
    if (!!node.parent) {
      return this.getRootNode(node.parent);
    } else {
      return node;
    }
  }

  nodeCheck(e) {
    this.curTreeSelection = this.treeSelection2[0] || this.treeSelection;
    this.initData(this.curTreeSelection);
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      path: this.treeSelection.path,
      sourceType: this.sourceSelection[0]?.type,
      sourceSubType: this.sourceSelection[0]?.subType,
      resourceIds: this.sourceSelection.map(item => item.uuid),
      scopeResourceId: this.treeSelection.uuid
    };
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      if (this.rowData) {
        // 调修改接口
        this.protectedResourceApiService
          .UpdateResourceGroup({
            UpdateResourceGroupRequestBody: {
              name: this.formGroup.value.name,
              resourceIds: this.sourceSelection.map(item => item.uuid)
            },
            resourceGroupId: this.rowData[0].uuid
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        // 调创建接口
        this.protectedResourceApiService
          .CreateResourceGroup({ CreateResourceGroupRequestBody: params })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
