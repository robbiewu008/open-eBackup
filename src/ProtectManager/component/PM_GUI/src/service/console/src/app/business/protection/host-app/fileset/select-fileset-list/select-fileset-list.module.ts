import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {
  SelectFilesetListComponent,
  SelectionPipe
} from './select-fileset-list.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SelectFilesetListComponent, SelectionPipe],
  imports: [CommonModule, BaseModule],
  exports: [SelectFilesetListComponent]
})
export class SelectFilesetListModule {}
