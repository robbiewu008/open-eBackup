import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { RegisterComponent } from './register/register.component';

@Component({
  selector: 'aui-active-directory',
  templateUrl: './active-directory.component.html',
  styleUrls: ['./active-directory.component.less']
})
export class ActiveDirectoryComponent implements OnInit {
  config;
  constructor() {}

  ngOnInit(): void {
    this.config = {
      activeIndex: DataMap.Resource_Type.ActiveDirectory.value,
      tableCols: [
        'uuid',
        'name',
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
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterComponent
    };
  }
}
