import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ClusterNodesModule } from './cluster-nodes/cluster-nodes.module';
import { SummaryComponent } from './summary.component';

@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, ClusterNodesModule],
  exports: [SummaryComponent]
})
export class RedisSummaryModule {}
