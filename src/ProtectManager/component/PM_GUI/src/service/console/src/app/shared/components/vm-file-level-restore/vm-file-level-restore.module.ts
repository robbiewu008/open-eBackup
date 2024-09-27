import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FileRestoreModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/file-restore/file-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { VmFileLevelRestoreComponent } from './vm-file-level-restore.component';

@NgModule({
  declarations: [VmFileLevelRestoreComponent],
  imports: [CommonModule, BaseModule, FileRestoreModule],
  exports: [VmFileLevelRestoreComponent]
})
export class VmFileLevelRestoreModule {}
