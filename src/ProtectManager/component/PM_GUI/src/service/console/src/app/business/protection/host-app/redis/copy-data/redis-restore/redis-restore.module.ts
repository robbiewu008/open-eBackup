import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RedisRestoreComponent } from './redis-restore.component';

@NgModule({
  declarations: [RedisRestoreComponent],
  imports: [CommonModule, BaseModule]
})
export class RedisRestoreModule {}
