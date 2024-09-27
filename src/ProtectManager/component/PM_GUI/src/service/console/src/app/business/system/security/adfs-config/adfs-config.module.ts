import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AdfsConfigComponent } from './adfs-config.component';
import { FormsModule } from '@angular/forms';
import { AdfsConfigRoutingModule } from './adfs-config-routing.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AdfsConfigComponent],
  imports: [CommonModule, BaseModule, FormsModule, AdfsConfigRoutingModule]
})
export class AdfsConfigModule {}
