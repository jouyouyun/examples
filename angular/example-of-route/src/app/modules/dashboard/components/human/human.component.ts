import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { map } from 'rxjs/operators';

import { HumanService, Human } from '../../services/human.service';

@Component({
  selector: 'app-human',
  templateUrl: './human.component.html',
  styleUrls: ['./human.component.sass']
})
export class HumanComponent implements OnInit {
  constructor(private route: ActivatedRoute,
    private router: Router,
    private service: HumanService) { }

  list$ = this.service.listHuman();
  activeID: number;

  ngOnInit() {
    this.list$.subscribe(list => {
      this.activeID = list[0].id
      console.log("==========Active ID:", this.activeID);
    });

    let sp = this.route.snapshot;
    console.log("[Human List] init");
    console.log("[Human List] route:", sp.toString())
    console.log("[Human List] url:", sp.url.join(','));
    console.log("[Human List]\tRoot:", sp.root.toString());
    console.log("[Human List]\tParent:", sp.parent.toString());
    for (let ch of sp.children) {
      console.log("[Human List]\tChild:", ch.toString())
    }
    console.log("[Human List] TEST:", sp.paramMap.get('id'));
  }

  goto(item: Human) {
    this.activeID = item.id;
    this.router.navigate([item.id], { relativeTo: this.route });
  }
}
