import { Component, OnInit, ViewChild } from '@angular/core';
import { DatabaseTemplateComponent } from '../../database-template/database-template.component';
import { RegisterGroupComponent } from './register-group/register-group.component';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-availabilty-group',
  templateUrl: './availabilty-group.component.html',
  styleUrls: ['./availabilty-group.component.less']
})
export class AvailabiltyGroupComponent implements OnInit {
  exchangeGroupOptions;

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit(): void {
    this.exchangeGroupOptions = {
      activeIndex: DataMap.Resource_Type.Exchange.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'subType',
        'sla_name',
        'sla_compliance',
        'sla_status',
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
        'rescan',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterGroupComponent
    };
  }
}
