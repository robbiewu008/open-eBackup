import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { UserListComponent } from './user-list.component';
import { CustomTableSearchModule } from '../custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [UserListComponent],
  imports: [CommonModule, BaseModule, CustomTableSearchModule],

  exports: [UserListComponent]
})
export class UserListModule {}
