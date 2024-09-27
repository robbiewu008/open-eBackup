import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AsManagementClusterComponent } from './as-management-cluster.component';

@NgModule({
  declarations: [AsManagementClusterComponent],
  imports: [CommonModule, BaseModule]
})
export class AsManagementClusterModule {}
