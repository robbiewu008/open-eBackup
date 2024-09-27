import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CreateArchiveStorageComponent } from './create-archive-storage.component';

@NgModule({
  declarations: [CreateArchiveStorageComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateArchiveStorageModule {}
