import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { BackupSetComponent } from '../hbase/backup-set/backup-set.component';
import { ClustersComponent } from '../hbase/clusters/clusters.component';

@Component({
  selector: 'aui-elastic-search',
  templateUrl: './elastic-search.component.html',
  styleUrls: ['./elastic-search.component.less']
})
export class ElasticSearchComponent implements OnInit {
  activeIndex;
  resSubType = DataMap.Resource_Type.Elasticsearch.value;
  backupsetResSubType = DataMap.Resource_Type.ElasticsearchBackupSet.value;

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
