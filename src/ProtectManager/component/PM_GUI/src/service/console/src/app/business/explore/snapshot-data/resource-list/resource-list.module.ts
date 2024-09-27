import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ResourceListComponent } from './resource-list.component';
import { BaseModule } from 'app/shared';
import { CyberSnapshotDataModule } from 'app/shared/components/cyber-snapshot-data/cyber-snapshot-data.module';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [ResourceListComponent],
  imports: [
    CommonModule,
    BaseModule,
    CyberSnapshotDataModule,
    JobResourceModule,
    CustomTableSearchModule
  ],
  exports: [ResourceListComponent]
})
export class ResourceListModule {}
