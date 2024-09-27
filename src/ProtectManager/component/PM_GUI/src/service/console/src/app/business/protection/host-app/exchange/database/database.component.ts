import { Component, OnInit, ViewChild } from '@angular/core';
import { DatabaseTemplateComponent } from '../../database-template/database-template.component';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-database',
  templateUrl: './database.component.html',
  styleUrls: ['./database.component.less']
})
export class DatabaseComponent implements OnInit {
  instanceConfig;

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit(): void {
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.ExchangeDataBase.value,
      tableCols: [
        'uuid',
        'name',
        'environmentName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup'
      ]
    };
  }
}
