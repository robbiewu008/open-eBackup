import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DamengRestoreComponent } from './dameng-restore.component';
import { BaseModule } from 'app/shared';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [DamengRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class DamengRestoreModule {}
