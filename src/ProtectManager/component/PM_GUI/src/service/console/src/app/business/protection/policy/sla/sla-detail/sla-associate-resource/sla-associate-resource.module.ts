import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SlaAssociateResourceComponent } from './sla-associate-resource.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SlaAssociateResourceComponent],
  imports: [CommonModule, BaseModule],
  exports: [SlaAssociateResourceComponent]
})
export class SlaAssociateResourceModule {}
