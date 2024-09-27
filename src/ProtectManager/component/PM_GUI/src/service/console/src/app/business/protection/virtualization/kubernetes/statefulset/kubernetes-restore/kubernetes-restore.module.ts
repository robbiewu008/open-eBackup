import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { KubernetesRestoreComponent } from './kubernetes-restore.component';

@NgModule({
  declarations: [KubernetesRestoreComponent],
  imports: [CommonModule, BaseModule]
})
export class KubernetesRestoreModule {}
