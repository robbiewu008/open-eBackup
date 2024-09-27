import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';
import { HCSUserListComponent } from './hcs-user-list.component';

@NgModule({
  declarations: [HCSUserListComponent],
  imports: [CommonModule, BaseModule, ProTableModule],

  exports: [HCSUserListComponent]
})
export class HCSUserListModule {}
