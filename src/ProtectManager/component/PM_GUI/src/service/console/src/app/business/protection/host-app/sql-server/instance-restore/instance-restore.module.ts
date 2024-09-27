import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceRestoreComponent } from './instance-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [InstanceRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class InstanceRestoreModule {}
