import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { DashboardService } from './services/dashboard.service';
import { HumanService } from './services/human.service';
import { ListComponent } from './components/list/list.component';
import { HumanComponent } from './components/human/human.component';
import { HumanDetailComponent } from './components/human-detail/human-detail.component';

const routes: Routes = [
  {
    path: 'dashboard',
    component: ListComponent,
    resolve: {
      list: DashboardService,
    },
    children: [
      {
        path: 'human',
        component: HumanComponent,
        resolve: {
          list: HumanService,
        },
        children: [
          {
            path: ":id",
            component: HumanDetailComponent
          }
        ]
      }
    ]
  }
]

@NgModule({
  imports: [
    RouterModule.forChild(routes)
  ],
  exports: [
    RouterModule
  ]
})
export class DashboardRoutingModule { }
