import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PolicyDetailComponent } from './policy-detail.component';
import { BaseModule } from 'app/shared';
import { AssociateResourceModule } from '../associate-resource/associate-resource.module';

@NgModule({
  declarations: [PolicyDetailComponent],
  imports: [CommonModule, BaseModule, AssociateResourceModule],
  exports: [PolicyDetailComponent]
})
export class PolicyDetailModule {}
