import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { ProButtonModule } from '../../../../../shared/components/pro-button';
import { UserAuthComponent } from './user-auth.component';

@NgModule({
  declarations: [UserAuthComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [UserAuthComponent]
})
export class UserAuthModule {}
