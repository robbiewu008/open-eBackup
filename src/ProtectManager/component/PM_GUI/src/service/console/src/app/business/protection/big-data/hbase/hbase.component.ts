import { Component, OnInit, ViewChild } from '@angular/core';
import { ClustersComponent } from './clusters/clusters.component';
import { BackupSetComponent } from './backup-set/backup-set.component';

@Component({
  selector: 'aui-hbase',
  templateUrl: './hbase.component.html',
  styleUrls: ['./hbase.component.less']
})
export class HbaseComponent implements OnInit {
  activeIndex;

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
