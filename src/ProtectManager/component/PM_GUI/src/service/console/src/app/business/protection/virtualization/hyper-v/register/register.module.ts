import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterComponent } from './register.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RegisterComponent],
  imports: [CommonModule, BaseModule],
  exports: [RegisterComponent]
})
export class RegisterModule {}
