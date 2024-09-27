import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MultiClusterOverviewComponent } from './multi-cluster-overview.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [MultiClusterOverviewComponent],
  imports: [CommonModule, BaseModule],
  exports: [MultiClusterOverviewComponent]
})
export class MultiClusterOverviewModule {}
