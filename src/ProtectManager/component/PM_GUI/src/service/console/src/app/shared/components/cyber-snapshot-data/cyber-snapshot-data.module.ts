import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CyberSnapshotDataComponent } from './cyber-snapshot-data.component';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';

@NgModule({
  declarations: [CyberSnapshotDataComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [CyberSnapshotDataComponent]
})
export class CyberSnapshotDataModule {}
