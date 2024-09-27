import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterMongodbComponent } from './register-mongodb.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BatchConfigPathModule } from './batch-config-path/batch-config-path.module';

@NgModule({
  declarations: [RegisterMongodbComponent],
  imports: [CommonModule, BaseModule, ProTableModule, BatchConfigPathModule]
})
export class RegisterMongodbModule {}
