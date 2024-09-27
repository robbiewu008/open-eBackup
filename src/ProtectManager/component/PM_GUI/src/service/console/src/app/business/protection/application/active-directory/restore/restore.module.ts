import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { RestoreComponent } from './restore.component';

@NgModule({
  declarations: [RestoreComponent],
  imports: [CommonModule, BaseModule, AlertModule]
})
export class RestoreModule {}
