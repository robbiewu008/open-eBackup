import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { RegisterRedisComponent } from './register-redis.component';
import { NasSharedModule } from 'app/business/protection/storage/nas-shared/nas-shared.module';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [RegisterRedisComponent],
  imports: [CommonModule, BaseModule, NasSharedModule, ProTableModule]
})
export class RegisterRedisModule {}
