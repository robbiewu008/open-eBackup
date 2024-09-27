import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { TargetClusterComponent } from './target-cluster.component';

@NgModule({
  declarations: [TargetClusterComponent],
  imports: [CommonModule, BaseModule]
})
export class TargetClusterModule {}
