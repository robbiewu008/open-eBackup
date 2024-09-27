import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AssociateResourceComponent } from './associate-resource.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AssociateResourceComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AssociateResourceComponent]
})
export class AssociateResourceModule {}
