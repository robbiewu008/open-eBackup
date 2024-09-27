import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddScriptComponent } from './add-script.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddScriptComponent],
  imports: [CommonModule, BaseModule]
})
export class AddScriptModule {}
