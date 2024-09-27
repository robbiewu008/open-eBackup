import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { BackupSetComponent } from '../hbase/backup-set/backup-set.component';
import { ClustersComponent } from '../hbase/clusters/clusters.component';

@Component({
  selector: 'aui-hive',
  templateUrl: './hive.component.html',
  styleUrls: ['./hive.component.less']
})
export class HiveComponent implements OnInit {
  activeIndex;
  resSubType = DataMap.Resource_Type.Hive.value;
  backupsetResSubType = DataMap.Resource_Type.HiveBackupSet.value;

  @ViewChild(BackupSetComponent, { static: false })
  backupSetComponent: BackupSetComponent;

  @ViewChild(ClustersComponent, { static: false })
  clustersComponent: ClustersComponent;

  constructor() {}

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 0) {
      this.clustersComponent.ngOnInit();
    } else {
      this.backupSetComponent.ngOnInit();
    }
  }
}
