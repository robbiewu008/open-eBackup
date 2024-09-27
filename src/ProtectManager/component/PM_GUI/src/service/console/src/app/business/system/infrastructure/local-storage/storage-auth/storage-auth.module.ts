import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { StorageAuthComponent } from './storage-auth.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [StorageAuthComponent],
  imports: [CommonModule, BaseModule],
  exports: [StorageAuthComponent]
})
export class StorageAuthModule {}
