import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NamespaceRestoreComponent } from './namespace-restore.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [NamespaceRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [NamespaceRestoreComponent]
})
export class NamespaceRestoreModule {}
