import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { InstanceDatabaseComponent } from '../gaussdb-dws/instance-database/instance-database.component';

@Component({
  selector: 'aui-gaussdb-for-opengauss',
  templateUrl: './gaussdb-for-opengauss.component.html',
  styleUrls: ['./gaussdb-for-opengauss.component.less']
})
export class GaussdbForOpengaussComponent implements OnInit {
  activeIndex = 0;
  instanceConfig;
  clusterConfig;
  subType = DataMap.Resource_Type.gaussdbForOpengauss.value;

  @ViewChild(InstanceDatabaseComponent, { static: false })
  instanceDatabaseComponent: InstanceDatabaseComponent;

  constructor() {}

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.gaussdbForOpengaussProject.value,
      tableCols: ['uuid', 'name', 'linkStatus', 'authorizedUser', 'operation'],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'ownedProject',
        'region',
        'version',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup'
      ]
    };
  }

  onChange() {
    this.instanceDatabaseComponent.ngOnInit();
  }
}
