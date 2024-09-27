import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AssociateVstoreComponent } from './associate-vstore.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AssociateVstoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [AssociateVstoreComponent]
})
export class AssociateVstoreModule {}
