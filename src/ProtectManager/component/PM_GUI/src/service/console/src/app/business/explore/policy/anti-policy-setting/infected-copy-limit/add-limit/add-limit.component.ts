import { Component, OnInit } from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AntiRansomwareInfectConfigApiService,
  ApplicationType,
  DataMap,
  DataMapService,
  GlobalService,
  I18NService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  every,
  find,
  includes,
  isEmpty,
  values
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-limit',
  templateUrl: './add-limit.component.html',
  styleUrls: ['./add-limit.component.less']
})
export class AddLimitComponent implements OnInit {
  activeIndex = 'Backup';
  data;
  allSelectionMap = {};
  dataLimit = [];
  dataLimitAllSelect = false;
  dataLimitGroup = this.dataMapService
    .toArray('copyDataLimitType')
    .filter(
      item =>
        !includes([DataMap.copyDataLimitType.replication.value], item.value)
    );
  copyTypes: any = [
    {
      label: this.i18n.get('explore_backup_copy_label'),
      key: 'Backup'
    },
    {
      label: this.i18n.get('common_copy_a_copy_label'),
      key: 'Replicated'
    }
  ];
  appList: any = [
    {
      label: this.i18n.get('common_virtualization_label'),
      apps: [
        {
          subType: DataMap.Resource_Type.virtualMachine.value,
          type: 'Vmware',
          label: 'VMware'
        },
        {
          subType: DataMap.Resource_Type.FusionCompute.value,
          type: ApplicationType.FusionCompute,
          label: 'FusionCompute'
        },
        {
          subType: DataMap.Resource_Type.fusionOne.value,
          type: ApplicationType.FusionOne,
          label: this.i18n.get('protection_fusionone_label')
        },
        {
          subType: DataMap.Resource_Type.cNwareVm.value,
          type: ApplicationType.CNware,
          label: 'CNware'
        },
        {
          subType: DataMap.Resource_Type.hyperVVm.value,
          type: 'Hyper-V',
          label: 'Hyper-V'
        }
      ]
    },
    {
      label: this.i18n.get('common_huawei_clouds_label'),
      apps: [
        {
          subType: DataMap.Resource_Type.HCSCloudHost.value,
          type: ApplicationType.HCS_CONTAINER,
          label: this.i18n.get('common_cloud_label')
        },
        {
          subType: DataMap.Resource_Type.openStackCloudServer.value,
          type: ApplicationType.OpenStack,
          label: this.i18n.get('common_open_stack_label')
        }
      ]
    },
    {
      label: this.i18n.get('common_file_system_label'),
      apps: [
        {
          subType: DataMap.Resource_Type.NASFileSystem.value,
          type: ApplicationType.NASFileSystem,
          label: this.i18n.get('common_nas_file_systems_label')
        },
        {
          subType: DataMap.Resource_Type.NASShare.value,
          type: ApplicationType.NASShare,
          label: this.i18n.get('common_nas_shares_label')
        },
        {
          subType: DataMap.Resource_Type.fileset.value,
          type: ApplicationType.Fileset,
          label: this.i18n.get('common_fileset_label')
        }
      ]
    }
  ];

  constructor(
    public modal: ModalRef,
    public globalService: GlobalService,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private antiRansomwareInfectedCopyService: AntiRansomwareInfectConfigApiService
  ) {}

  ngOnInit() {
    each(this.copyTypes, item => {
      assign(item, {
        appList: cloneDeep(this.appList)
      });
    });
    // 在单个资源的限制修改时回显操作
    if (!!this.data && this.data.length === 1) {
      this.updateData();
    }
  }

  updateData() {
    const tmpData = this.data[0].infectedCopyOperations.split(',');
    this.dataLimit = this.dataLimitGroup
      .map(item => item.value)
      .filter(val => tmpData.includes(val));
    this.dataLimitChange();
  }

  dataLimitChange(e?) {
    this.dataLimitAllSelect =
      this.dataLimit.length === this.dataLimitGroup.length;
    this.disableBtn();
  }

  allSelectDataLimit(e) {
    e.stopPropagation();
    const tmpAllSelect = !this.dataLimitAllSelect;
    if (tmpAllSelect) {
      this.dataLimit = this.dataLimitGroup.map(item => item.value);
    } else {
      this.dataLimit = [];
    }
    this.dataLimitAllSelect = tmpAllSelect;
    this.disableBtn();
  }

  beforeExpanded = collapse => {
    // 展开时触发获取数据
    this.globalService.emitStore({
      action: collapse.lvId + 'antiPolicy' + this.activeIndex,
      state: true
    });
  };

  selectChange(e) {
    each(this.copyTypes, item => {
      let typeFlag = false;
      each(item.appList, val => {
        let flag = false;
        each(val.apps, app => {
          app.selected =
            !isEmpty(this.allSelectionMap[app.subType]) &&
            !!find(this.allSelectionMap[app.subType], { copyType: item.key });
          flag = flag || app.selected;
        });
        val.selected = flag;
        typeFlag = typeFlag || val.selected;
      });
      item.selected = typeFlag;
    });
    this.disableBtn();
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled =
      !this.dataLimit.length ||
      (every(values(this.allSelectionMap), isEmpty) && !this.data);
  }

  getSelectedStatus() {
    let flag = true;
    for (const key in this.allSelectionMap) {
      if (!isEmpty(this.allSelectionMap[key])) {
        flag = false;
      }
    }
    return flag;
  }

  getParams() {
    const params: any = {
      operations: [...this.dataLimit]
    };

    if (!!this.data) {
      assign(params, {
        ids: this.data.map(item => item.id)
      });
      return params;
    }

    let resourceList = [];
    for (const key in this.allSelectionMap) {
      each(this.allSelectionMap[key], item => {
        resourceList.push({
          resourceId: item.uuid,
          resourceName: item.name,
          resourceSubType: item.subType,
          resourceLocation: item?.path,
          copyType: item.copyType
        });
      });
    }

    assign(params, {
      selectedResources: resourceList
    });

    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!!this.data) {
        this.antiRansomwareInfectedCopyService
          .antiRansomwareInfectedCopyConfigUpdate({
            configUpdateReq: this.getParams()
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
        this.antiRansomwareInfectedCopyService
          .antiRansomwareInfectedCopyConfigAdd({
            configAddReq: this.getParams()
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
      }
    });
  }
}
