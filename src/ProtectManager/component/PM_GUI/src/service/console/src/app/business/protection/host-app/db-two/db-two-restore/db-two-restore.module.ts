import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DbTwoRestoreComponent } from './db-two-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [DbTwoRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class DbTwoRestoreModule {}
