import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { DatabaseTemplateComponent } from '../database-template/database-template.component';
import { CreateVolumeComponent } from './create-volume/create-volume.component';

@Component({
  selector: 'aui-volume',
  templateUrl: './volume.component.html',
  styleUrls: ['./volume.component.less']
})
export class VolumeComponent implements OnInit {
  volumeConfig;
  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit() {
    this.volumeConfig = {
      activeIndex: DataMap.Resource_Type.volume.value,
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
      registerComponent: CreateVolumeComponent
    };
  }

  onChange() {}
}
