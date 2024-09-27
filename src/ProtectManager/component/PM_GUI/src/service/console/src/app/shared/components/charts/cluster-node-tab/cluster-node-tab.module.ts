import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterNodeTabComponent } from './cluster-node-tab.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [ClusterNodeTabComponent],
  imports: [CommonModule, BaseModule],
  exports: [ClusterNodeTabComponent]
})
export class ClusterNodeTabModule {}
