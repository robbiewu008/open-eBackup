import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreInstanceListComponent } from './postgre-instance-list.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [PostgreInstanceListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [PostgreInstanceListComponent]
})
export class PostgreInstanceListModule {}
