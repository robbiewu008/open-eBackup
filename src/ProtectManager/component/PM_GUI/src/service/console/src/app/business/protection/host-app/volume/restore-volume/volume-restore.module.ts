import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { VolumeRestoreComponent } from './volume-restore.component';

@NgModule({
  declarations: [VolumeRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AlertModule]
})
export class VolumeRestoreModule {}
