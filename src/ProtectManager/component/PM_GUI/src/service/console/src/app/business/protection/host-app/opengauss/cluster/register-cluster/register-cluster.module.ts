import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterClusterComponent } from './register-cluster.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RegisterClusterComponent],
  imports: [CommonModule, BaseModule]
})
export class RegisterClusterModule {}
