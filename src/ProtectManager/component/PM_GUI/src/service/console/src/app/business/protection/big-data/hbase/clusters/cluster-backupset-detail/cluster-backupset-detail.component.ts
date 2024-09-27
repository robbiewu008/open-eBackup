import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-cluster-backupset-detail-hbase',
  templateUrl: './cluster-backupset-detail.component.html',
  styleUrls: ['./cluster-backupset-detail.component.less']
})
export class ClusterBackupsetDetailComponent implements OnInit {
  data = {} as any;
  formItems = [];
  resSubType;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.data = data;

    this.data.subType === DataMap.Resource_Type.Hive.value
      ? (this.resSubType = DataMap.Resource_Type.HiveBackupSet.value)
      : this.data.subType === DataMap.Resource_Type.Elasticsearch.value
      ? (this.resSubType = DataMap.Resource_Type.ElasticsearchBackupSet.value)
      : (this.resSubType = DataMap.Resource_Type.HBaseBackupSet.value);
  }
}
