import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DashboardRoutingModule } from './dashboard-routing.module';
import { ListComponent } from './components/list/list.component';
import { HumanDetailComponent } from './components/human-detail/human-detail.component';
import { SexTypePipe } from './pipes/sex-type.pipe';
import { HumanComponent } from './components/human/human.component';

@NgModule({
  declarations: [
    ListComponent,
    HumanDetailComponent,
    SexTypePipe,
    HumanComponent
  ],
  imports: [
    CommonModule,
    DashboardRoutingModule
  ]
})
export class DashboardModule { }
