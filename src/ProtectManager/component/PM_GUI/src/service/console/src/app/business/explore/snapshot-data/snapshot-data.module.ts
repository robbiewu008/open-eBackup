import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { SnapshotDataRoutingModule } from './snapshot-data-routing.module';
import { SnapshotDataComponent } from './snapshot-data.component';
import { BaseModule } from 'app/shared';
import { ResourceListModule } from './resource-list/resource-list.module';
import { SnapshotListModule } from './snapshot-list/snapshot-list.module';

@NgModule({
  declarations: [SnapshotDataComponent],
  imports: [
    CommonModule,
    SnapshotDataRoutingModule,
    BaseModule,
    ResourceListModule,
    SnapshotListModule
  ]
})
export class SnapshotDataModule {}
