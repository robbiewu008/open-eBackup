import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { HdfsRoutingModule } from './hdfs-routing.module';
import { HdfsComponent } from './hdfs.component';
import { FilesetsModule } from './filesets/filesets.module';
import { ClustersModule } from './clusters/clusters.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [HdfsComponent],
  imports: [
    CommonModule,
    HdfsRoutingModule,
    BaseModule,
    FilesetsModule,
    ClustersModule,
    MultiClusterSwitchModule
  ]
})
export class HdfsModule {}
