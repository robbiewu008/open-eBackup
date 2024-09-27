import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddUserComponent } from './add-user.component';

@NgModule({
  declarations: [AddUserComponent],
  imports: [CommonModule, BaseModule]
})
export class AddUserModule {}
