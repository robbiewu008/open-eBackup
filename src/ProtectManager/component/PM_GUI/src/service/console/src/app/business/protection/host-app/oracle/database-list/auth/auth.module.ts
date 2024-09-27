import { NgModule } from '@angular/core';
import { AuthComponent } from './auth.component';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [AuthComponent],
  exports: [AuthComponent]
})
export class AuthModule {}
