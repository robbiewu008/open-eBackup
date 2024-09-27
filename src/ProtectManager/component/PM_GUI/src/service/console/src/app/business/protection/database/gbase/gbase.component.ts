import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { InstanceDatabaseComponent } from '../../host-app/gaussdb-dws/instance-database/instance-database.component';

@Component({
  selector: 'aui-gbase',
  templateUrl: './gbase.component.html',
  styleUrls: ['./gbase.component.less']
})
export class GbaseComponent implements OnInit {
  activeIndex = 0;
  clusterConfig;
  instanceConfig;
  subType = DataMap.Resource_Type.GBase.value;

  constructor() {}

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.gbaseCluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'logBackup',
        'authorizedUser',
        'operation'
      ],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.gbaseInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'clusterOrHostName',
        'version',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup',
        'deleteResource'
      ]
    };
  }
}
