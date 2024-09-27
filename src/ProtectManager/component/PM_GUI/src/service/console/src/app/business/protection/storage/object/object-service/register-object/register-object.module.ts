import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RegisterObjectComponent } from './register-object.component';

@NgModule({
  declarations: [RegisterObjectComponent],
  imports: [CommonModule, BaseModule],
  exports: [RegisterObjectComponent]
})
export class RegisterObjectModule {}
