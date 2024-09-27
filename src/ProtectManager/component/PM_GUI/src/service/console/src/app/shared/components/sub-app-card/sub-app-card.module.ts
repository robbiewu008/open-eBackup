import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SubAppCardComponent } from './sub-app-card.component';
import { MultiClusterSwitchModule } from '../multi-cluster-switch/multi-cluster-switch.module';
import { LinkModule } from '@iux/live';

@NgModule({
  declarations: [SubAppCardComponent],
  imports: [CommonModule, BaseModule, MultiClusterSwitchModule, LinkModule],
  exports: [SubAppCardComponent]
})
export class SubAppCardModule {}
