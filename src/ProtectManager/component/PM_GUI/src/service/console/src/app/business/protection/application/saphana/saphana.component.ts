import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { DatabaseTemplateComponent } from '../../host-app/database-template/database-template.component';
import { RegisterInstanceComponent } from './register-instance/register-instance.component';
import { RegisterDatabaseComponent } from './register-database/register-database.component';
@Component({
  selector: 'aui-saphana',
  templateUrl: './saphana.component.html',
  styleUrls: ['./saphana.component.less']
})
export class SaphanaComponent implements OnInit {
  activeIndex = 0;
  instanceConfig;
  databaseConfig;
  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit() {
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.saphanaInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'version',
        'enableLogBackup',
        'authorizedUser',
        'operation'
      ],
      tableOpts: [
        'register',
        'resourceAuth',
        'resourceReclaiming',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterInstanceComponent
    };

    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.saphanaDatabase.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'sapHanaDbType',
        'sapHanaDbDeployType',
        'environmentName',
        'environmentEndpoint',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterDatabaseComponent
    };
  }

  onChange() {
    this.databaseTemplateComponent.ngOnInit();
  }
}
