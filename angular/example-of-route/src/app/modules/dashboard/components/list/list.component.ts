import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { DashboardService } from '../../services/dashboard.service';

@Component({
  selector: 'app-list',
  templateUrl: './list.component.html',
  styleUrls: ['./list.component.sass']
})
export class ListComponent implements OnInit {
  constructor(private route: ActivatedRoute,
    private service: DashboardService) { }

  list$ = this.service.list();

  ngOnInit() {
    let sp = this.route.snapshot;
    console.log("[List] init");
    console.log("[List] route:", sp.toString())
    console.log("[Human List] url:", sp.url.join(','));
    console.log("[List]\tRoot:", sp.root.toString());
    console.log("[List]\tParent:", sp.parent.toString());
    for (let ch of sp.children) {
      console.log("[List]\tChild:", ch.toString())
    }
  }

}
