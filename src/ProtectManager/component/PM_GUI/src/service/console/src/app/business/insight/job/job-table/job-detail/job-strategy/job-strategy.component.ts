import { Component, Input, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  ApplicationType,
  DataMap,
  GlobalService,
  LiveMountAction,
  ResourceType
} from 'app/shared';
import {
  LiveMountPolicyApiService,
  SlaApiService
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, findKey, isArray } from 'lodash';

@Component({
  selector: 'aui-job-strategy',
  templateUrl: './job-strategy.component.html',
  styleUrls: ['./job-strategy.component.less']
})
export class JobStrategyComponent implements OnInit {
  @Input() job;
  sla;
  isExist = true;
  slaName;
  slaUpdated = false;
  backupDetailJobType = [
    DataMap.Job_type.backup_job.value,
    DataMap.Job_type.copy_data_job.value,
    DataMap.Job_type.archive_job.value
  ];
  dataMap = DataMap;
  applicationType = ApplicationType;
  extendStr;
  liveMountAction = LiveMountAction;
  liveMountData: any = {};
  isLiveMountDatabase = false; // 判断是否为即时挂载数据库类型
  currentRoute: string;
  resourceSubTypeSLAMap = this.appUtilsService.findResourceTypeByKey('slaId');
  constructor(
    public router: Router,
    public slaApiService: SlaApiService,
    public globalService: GlobalService,
    private appUtilsService: AppUtilsService,
    private liveMountPolicyApiService: LiveMountPolicyApiService
  ) {
    this.currentRoute = this.router.url;
  }

  ngOnInit(): void {
    this.extendStr = JSON.parse(this.job?.extendStr);
    if (this.backupDetailJobType.includes(this.job.type)) {
      this.sla = assign(this.extendStr, {
        application: this.getTypeKey(
          this.job.sourceSubType,
          this.resourceSubTypeSLAMap
        ),
        policy_list: this.extendStr?.triggerPolicy?.policy_list
      });
      this.slaName = this.sla.slaName;
    } else if (DataMap.Job_type.live_mount_job.value === this.job.type) {
      this.getLiveMountData();
    }
  }

  /**
   * 根据subType取出对应的slaType
   * @param type
   * @param typeMap
   */
  getTypeKey(type: string, typeMap) {
    const key = findKey(typeMap, item => {
      if (isArray(item)) {
        return item.includes(type);
      } else {
        return item === type;
      }
    });
    return key || type;
  }

  getSlaDetail() {
    if (!this.isExist) {
      return;
    }
    this.appUtilsService.setCacheValue('jobToSla', this.slaName);
    if (this.currentRoute === '/protection/policy/sla') {
      this.router.navigateByUrl('/protection/policy/sla');
      this.globalService.emitStore({
        action: 'jobToSla',
        state: true
      });
    } else {
      this.router.navigateByUrl('/protection/policy/sla');
    }
  }

  getLiveMountData() {
    if (!this.extendStr) {
      return;
    }
    let resourceType;
    let childResourceType;
    this.isLiveMountDatabase = [
      ApplicationType.Oracle,
      ApplicationType.TDSQL,
      ApplicationType.MySQL,
      ApplicationType.Exchange
    ].includes(this.job.sourceSubType);
    switch (this.job.sourceSubType) {
      case ApplicationType.Fileset:
        resourceType = ResourceType.HOST;
        childResourceType = [DataMap.Resource_Type.fileset.value];
        break;
      case ApplicationType.Volume:
        (resourceType = ResourceType.HOST),
          (childResourceType = [DataMap.Resource_Type.volume.value]);
        break;
      case ApplicationType.Vmware:
        resourceType = ResourceType.VM;
        childResourceType = [DataMap.Resource_Type.virtualMachine.value];
        break;
      case ApplicationType.CNware:
        resourceType = ResourceType.VM;
        childResourceType = [DataMap.Resource_Type.cNwareVm.value];
        break;
      case ApplicationType.Oracle:
        resourceType = ResourceType.DATABASE;
        childResourceType = [DataMap.Resource_Type.oracle.value];
        break;
      case ApplicationType.TDSQL:
        resourceType = ResourceType.DATABASE;
        childResourceType = [DataMap.Resource_Type.tdsqlInstance.value];
        break;
      case ApplicationType.MySQL:
        resourceType = ResourceType.DATABASE;
        childResourceType = [DataMap.Resource_Type.MySQLInstance.value];
        break;
      case ApplicationType.Exchange:
        resourceType = ResourceType.DATABASE;
        childResourceType = [
          DataMap.Resource_Type.ExchangeSingle.value,
          DataMap.Resource_Type.ExchangeGroup.value,
          DataMap.Resource_Type.ExchangeDataBase.value
        ];
        break;
      case ApplicationType.NASShare:
        resourceType = ResourceType.Storage;
        childResourceType = [DataMap.Resource_Type.NASShare.value];
        break;
      case ApplicationType.NASFileSystem:
        resourceType = ResourceType.Storage;
        childResourceType = [DataMap.Resource_Type.NASFileSystem.value];
        break;
    }
    this.extendStr.jobConfig.targetLocation = this.job.targetLocation;
    this.liveMountData = {
      liveMountData: { parameters: JSON.stringify(this.extendStr?.jobConfig) },
      action: LiveMountAction.View,
      resourceType,
      childResourceType
    };
  }
}
