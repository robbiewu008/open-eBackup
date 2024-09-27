import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddTargetClusterComponent } from './add-target-cluster.component';

@NgModule({
  declarations: [AddTargetClusterComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddTargetClusterComponent]
})
export class AddTargetClusterModule {}
