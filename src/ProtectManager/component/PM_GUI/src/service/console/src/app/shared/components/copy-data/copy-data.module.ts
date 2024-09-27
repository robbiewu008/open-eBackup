import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { TodayModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/today/today.module';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataListModule } from '../copy-data-list/copy-data-list.module';
import { CopyDataScnModule } from '../copy-data-scn/copy-data-scn.module';
import { CopyDataSearchModule } from '../copy-data-search/copy-data-search.module';
import { CyberSnapshotDataModule } from '../cyber-snapshot-data/cyber-snapshot-data.module';
import { CopyDataComponent } from './copy-data.component';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [
    CommonModule,
    BaseModule,
    CopyDataListModule,
    CopyDataScnModule,
    TodayModule,
    CopyDataSearchModule,
    CyberSnapshotDataModule
  ],
  exports: [CopyDataComponent]
})
export class CopyDataModule {}
