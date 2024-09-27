import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterOpenstackComponent } from './register-openstack.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RegisterOpenstackComponent],
  imports: [CommonModule, BaseModule],
  exports: [RegisterOpenstackComponent]
})
export class RegisterOpenstackModule {}
