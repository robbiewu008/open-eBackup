import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { CustomModalOperateModule } from '../custom-modal-operate';
import { DownloadFlrFilesModule } from '../download-flr-files/download-flr-files.module';
import { FileIndexedModule } from '../file-indexed';
import { CopyDataDetailComponent } from './copy-data-detail.component';
import { FileTreeModule } from '../file-tree/file-tree.module';

@NgModule({
  declarations: [CopyDataDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CustomModalOperateModule,
    FileIndexedModule,
    ManualMountModule,
    DownloadFlrFilesModule,
    FileTreeModule
  ],

  exports: [CopyDataDetailComponent]
})
export class CopyDataDetailModule {}
