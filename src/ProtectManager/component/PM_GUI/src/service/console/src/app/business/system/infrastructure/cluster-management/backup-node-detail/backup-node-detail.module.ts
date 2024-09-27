import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { BackupNodeDetailComponent } from './backup-node-detail.component';

@NgModule({
  declarations: [BackupNodeDetailComponent],
  imports: [CommonModule, BaseModule, AlertModule, CurrentSystemTimeModule]
})
export class BackupNodeDetailModule {}
