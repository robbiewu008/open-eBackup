import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { RegisterComponent } from './register.component';

@NgModule({
  declarations: [RegisterComponent],
  imports: [CommonModule, BaseModule, ProButtonModule, ProTableModule]
})
export class RegisterModule {}
