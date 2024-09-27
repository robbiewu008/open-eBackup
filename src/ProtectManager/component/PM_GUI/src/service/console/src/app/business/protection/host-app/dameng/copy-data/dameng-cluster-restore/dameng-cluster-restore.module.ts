import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DamengClusterRestoreComponent } from './dameng-cluster-restore.component';
import { BaseModule } from 'app/shared';
import { DamengRestoreModule } from '../dameng-restore/dameng-restore.module';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [DamengClusterRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class DamengClusterRestoreModule {}
