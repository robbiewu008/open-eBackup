import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { ManualCopyModule } from './manual-copy/manual-copy.module';
import { ResourceReplicaListComponent } from './resource-replica-list.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [ResourceReplicaListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProtectModule,
    ManualCopyModule,
    CustomTableSearchModule
  ],
  exports: [ResourceReplicaListComponent]
})
export class ResourceReplicaListModule {}
