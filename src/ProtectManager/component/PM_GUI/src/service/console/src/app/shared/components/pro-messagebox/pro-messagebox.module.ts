import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { I18NModule, CheckboxModule, GroupModule } from '@iux/live';
import {
  ProMessageboxComponent,
  SafeHtmlPipe
} from './pro-messagebox.component';
import { ProTableModule } from '../pro-table/index';

@NgModule({
  declarations: [ProMessageboxComponent, SafeHtmlPipe],
  imports: [
    FormsModule,
    CommonModule,
    I18NModule,
    CheckboxModule,
    GroupModule,
    ProTableModule
  ]
})
export class ProMessageboxModule {}
