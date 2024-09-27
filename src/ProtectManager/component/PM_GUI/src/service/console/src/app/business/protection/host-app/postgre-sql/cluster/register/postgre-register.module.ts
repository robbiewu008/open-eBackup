import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { PostgreRegisterComponent } from './postgre-register.component';

@NgModule({
  declarations: [PostgreRegisterComponent],
  imports: [CommonModule, BaseModule]
})
export class PostgreRegisterModule {}
