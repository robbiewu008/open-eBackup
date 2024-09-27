import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { UserDetailComponent } from './user-detail.component';

@NgModule({
  declarations: [UserDetailComponent],
  imports: [CommonModule, BaseModule]
})
export class UserDetailModule {}
