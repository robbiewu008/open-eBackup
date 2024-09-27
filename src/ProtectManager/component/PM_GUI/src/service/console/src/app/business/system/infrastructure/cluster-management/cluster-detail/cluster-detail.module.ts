import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ClusterDetailComponent } from './cluster-detail.component';

@NgModule({
  declarations: [ClusterDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [ClusterDetailComponent]
})
export class ClusterDetailModule {}
