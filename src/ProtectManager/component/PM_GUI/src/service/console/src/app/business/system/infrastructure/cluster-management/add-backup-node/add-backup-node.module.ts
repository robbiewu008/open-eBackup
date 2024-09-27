import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddBackupNodeComponent } from './add-backup-node.component';

@NgModule({
  declarations: [AddBackupNodeComponent],
  imports: [CommonModule, BaseModule]
})
export class AddBackupNodeModule {}
