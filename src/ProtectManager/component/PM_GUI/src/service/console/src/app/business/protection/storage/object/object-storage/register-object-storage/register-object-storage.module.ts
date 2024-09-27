import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { RegisterObjectStorageComponent } from './register-object-storage.component';

@NgModule({
  declarations: [RegisterObjectStorageComponent],
  imports: [CommonModule, BaseModule, AlertModule],
  exports: [RegisterObjectStorageComponent]
})
export class RegisterObjectStorageModule {}
