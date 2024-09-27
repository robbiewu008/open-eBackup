import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { FileIndexedModule } from 'app/shared/components';
import { RestoreModule } from 'app/shared/services/restore.service';
import { CopyListComponent, FilterPipe } from './copy-list.component';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { ManualIndexModule } from 'app/shared/components/manual-index/manual-index.module';
import { CopyActionModule } from 'app/shared/services/copy-action.service';
import { CopyVerifyModule } from 'app/shared/components/copy-verify-proxy/copy-verify.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [CopyListComponent, FilterPipe],
  imports: [
    CommonModule,
    BaseModule,
    FileIndexedModule,
    RestoreModule,
    ManualMountModule,
    ManualIndexModule,
    CopyActionModule,
    CopyVerifyModule,
    CustomTableSearchModule
  ],
  exports: [CopyListComponent]
})
export class CopyListModule {}
