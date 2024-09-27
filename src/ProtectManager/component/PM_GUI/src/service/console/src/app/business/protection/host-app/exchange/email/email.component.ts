import { Component, OnInit, ViewChild } from '@angular/core';
import { DatabaseTemplateComponent } from '../../database-template/database-template.component';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-email',
  templateUrl: './email.component.html',
  styleUrls: ['./email.component.less']
})
export class EmailComponent implements OnInit {
  instanceConfig;

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit(): void {
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.ExchangeEmail.value,
      tableCols: [
        'uuid',
        'name',
        'address',
        'environmentName',
        'parentName',
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
