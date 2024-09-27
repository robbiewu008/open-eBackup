import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { DragDropModule } from '@angular/cdk/drag-drop';
import { FormsModule } from '@angular/forms';
import {
  AlertModule,
  ButtonModule,
  DatatableModule,
  GroupModule,
  PaginatorModule,
  RadioModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { SetStoragePolicyComponent } from './set-storage-policy.component';

@NgModule({
  declarations: [SetStoragePolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    RadioModule,
    FormsModule,
    GroupModule,
    ButtonModule,
    DatatableModule,
    DragDropModule,
    PaginatorModule,
    AlertModule
  ],
  exports: [SetStoragePolicyComponent]
})
export class SetStoragePolicyModule {}
