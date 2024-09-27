import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { KingBaseRestoreComponent } from './king-base-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [KingBaseRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class KingBaseRestoreModule {}
