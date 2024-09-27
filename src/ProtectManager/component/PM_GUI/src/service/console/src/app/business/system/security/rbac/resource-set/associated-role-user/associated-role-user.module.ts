import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AssociatedRoleUserComponent } from './associated-role-user.component';

@NgModule({
  declarations: [AssociatedRoleUserComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AssociatedRoleUserComponent]
})
export class AssociatedRoleUserModule {}
