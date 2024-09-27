import { Component, OnInit, ViewChild } from '@angular/core';
import { DatabaseTemplateComponent } from '../../host-app/database-template/database-template.component';
import { DataMap } from 'app/shared';
import { CreateCommonShareComponent } from './create-commonshare/create-commonshare.component';

@Component({
  selector: 'aui-commonshare',
  templateUrl: './commonshare.component.html',
  styleUrls: ['./commonshare.component.less']
})
export class CommonShareComponent implements OnInit {
  databaseConfig;
  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;
  constructor() {}

  ngOnInit() {
    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.commonShare.value,
      tableCols: [
        'uuid',
        'name',
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
        'resourceAuth',
        'resourceReclaiming',
        'manualBackup',
        'modify',
        'deleteResource'
      ],
      registerComponent: CreateCommonShareComponent
    };
  }
}
