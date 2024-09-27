import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ClusterRestoreComponent } from './restore-cluster.component';

@NgModule({
  declarations: [ClusterRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [ClusterRestoreComponent]
})
export class ClusterRestoreModule {}
