import { Component, OnInit, ViewChild } from '@angular/core';
import { ClustersComponent } from './clusters/clusters.component';
import { FilesetsComponent } from './filesets/filesets.component';

@Component({
  selector: 'aui-hdfs',
  templateUrl: './hdfs.component.html',
  styleUrls: ['./hdfs.component.less']
})
export class HdfsComponent implements OnInit {
  activeIndex;

  @ViewChild(FilesetsComponent, { static: false })
  filesetsComponent: FilesetsComponent;

  @ViewChild(ClustersComponent, { static: false })
  clustersComponent: ClustersComponent;

  constructor() {}

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 0) {
      this.clustersComponent.ngOnInit();
    } else {
      this.filesetsComponent.ngOnInit();
    }
  }
}
