import { Component, Input, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { isString } from 'lodash';

@Component({
  selector: 'aui-restore-parameter-detail',
  templateUrl: './restore-parameter-detail.component.html',
  styleUrls: ['./restore-parameter-detail.component.less']
})
export class RestoreParameterDetailComponent implements OnInit {
  @Input() job;
  data;
  agentName;
  boolean = Boolean;
  dataMap = DataMap;
  isString = isString;
  _parse = JSON.parse;
  _string = String;
  _boolean = Boolean;
  isProxy = false;
  isVirtualCloud = false; // 是否为部分虚拟化云平台应用
  isScript = false; // 是否有脚本
  machineData; // 用于tdsql分布式实例
  hideAutoPowerOn = false; // 隐藏恢复后自动开机
  hideRestoreBeforeVerify = false; // 隐藏恢复前执行副本校验

  constructor() {}

  ngOnInit(): void {
    this.data = JSON.parse(this.job?.extendStr || '{}')?.jobConfig;
    this.agentName = this.data.agents;
    this.isProxy = [
      DataMap.Resource_Type.APSResourceSet.value,
      DataMap.Resource_Type.APSCloudServer.value,
      DataMap.Resource_Type.APSZone.value,
      DataMap.Resource_Type.HCSCloudHost.value,
      DataMap.Resource_Type.virtualMachine.value,
      DataMap.Resource_Type.cNwareVm.value,
      DataMap.Resource_Type.FusionCompute.value,
      DataMap.Resource_Type.KubernetesStatefulset.value,
      DataMap.Resource_Type.openStackCloudServer.value
    ].includes(this.job.sourceSubType);
    this.isVirtualCloud = [
      DataMap.Resource_Type.cNwareVm.value,
      DataMap.Resource_Type.FusionCompute.value,
      DataMap.Resource_Type.KubernetesStatefulset.value,
      DataMap.Resource_Type.HCSCloudHost.value,
      DataMap.Resource_Type.hyperV.value,
      DataMap.Resource_Type.virtualMachine.value,
      DataMap.Resource_Type.openStackCloudServer.value
    ].includes(this.job.sourceSubType);
    this.hideAutoPowerOn =
      this.job.sourceSubType ===
        DataMap.Resource_Type.KubernetesStatefulset.value ||
      (this.job.sourceSubType ===
        DataMap.Resource_Type.openStackCloudServer.value &&
        this.data?.restoreLevel === '0');
    this.hideRestoreBeforeVerify = [
      DataMap.Resource_Type.hyperV.value,
      DataMap.Resource_Type.virtualMachine.value
    ].includes(this.job.sourceSubType);
    this.isScript = [
      DataMap.Resource_Type.oracle.value,
      DataMap.Resource_Type.oracleCluster.value,
      DataMap.Resource_Type.MySQL.value,
      DataMap.Resource_Type.MySQLInstance.value,
      DataMap.Resource_Type.MySQLClusterInstance.value,
      DataMap.Resource_Type.MySQLDatabase.value,
      DataMap.Resource_Type.fileset.value,
      DataMap.Resource_Type.volume.value,
      DataMap.Resource_Type.dbTwoDatabase.value,
      DataMap.Resource_Type.dbTwoTableSet.value,
      DataMap.Resource_Type.SQLServerDatabase.value,
      DataMap.Resource_Type.PostgreSQLInstance.value,
      DataMap.Resource_Type.PostgreSQLClusterInstance.value,
      DataMap.Resource_Type.KingBaseInstance.value,
      DataMap.Resource_Type.KingBaseClusterInstance.value,
      DataMap.Resource_Type.KubernetesStatefulset.value,
      DataMap.Resource_Type.ExchangeSingle.value,
      DataMap.Resource_Type.ExchangeGroup.value,
      DataMap.Resource_Type.ExchangeDataBase.value
    ].includes(this.job.sourceSubType);
    if (
      this.job.sourceSubType ===
      DataMap.Resource_Type.tdsqlDistributedInstance.value
    ) {
      this.machineData = JSON.parse(this.data?.extParameter);
    }
  }
}
