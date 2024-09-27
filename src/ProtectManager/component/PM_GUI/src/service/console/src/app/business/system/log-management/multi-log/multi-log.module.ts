import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { MultiLogComponent } from './multi-log.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [MultiLogComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AlertModule
  ],
  exports: [MultiLogComponent]
})
export class MultiLogModule {}
