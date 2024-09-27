import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { HdfsFilesetRestoreComponent } from './hdfs-fileset-restore.component';

@NgModule({
  declarations: [HdfsFilesetRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [HdfsFilesetRestoreComponent]
})
export class HdfsFilesetRestoreModule {}
