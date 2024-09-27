import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MultiClusterSwitchComponent } from './multi-cluster-switch.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [MultiClusterSwitchComponent],
  imports: [CommonModule, BaseModule],
  exports: [MultiClusterSwitchComponent]
})
export class MultiClusterSwitchModule {}
