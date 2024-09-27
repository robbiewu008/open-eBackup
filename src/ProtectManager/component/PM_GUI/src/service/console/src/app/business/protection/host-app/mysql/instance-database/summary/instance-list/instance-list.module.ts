import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceListComponent } from './instance-list.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [InstanceListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [InstanceListComponent]
})
export class InstanceListModule {}
