import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BackupClusterComponent } from './backup-cluster.component';

@NgModule({
  declarations: [BackupClusterComponent],
  imports: [CommonModule, BaseModule]
})
export class BackupClusterModule {}
