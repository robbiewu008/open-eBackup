import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RestoreComponent } from './restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [RestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [RestoreComponent]
})
export class RestoreModule {}
