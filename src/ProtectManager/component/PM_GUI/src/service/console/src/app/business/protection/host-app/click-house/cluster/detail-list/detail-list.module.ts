import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DetailListComponent } from './detail-list.component';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  declarations: [DetailListComponent]
})
export class DetailListModule {}
