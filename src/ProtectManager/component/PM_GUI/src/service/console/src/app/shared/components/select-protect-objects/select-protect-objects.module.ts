import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {
  SelectionPipe,
  SelectProtectObjectsComponent
} from './select-protect-objects.component';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';
import { ProStatusModule } from '../pro-status';

@NgModule({
  declarations: [SelectProtectObjectsComponent, SelectionPipe],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [SelectProtectObjectsComponent]
})
export class SelectProtectObjectsModule {}
