import { Component, OnInit } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';

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

  ngOnInit() {
    let sp = this.route.snapshot;
    console.log("[Human List] init");
    console.log("[Human List] route:", sp.toString())
    console.log("[Human List]\tRoot:", sp.root.toString());
    console.log("[Human List]\tParent:", sp.parent.toString());
    for (let ch of sp.children) {
      console.log("[Human List]\tChild:", ch.toString())
    }
  }

  goto(item: Human) {
    this.router.navigate([item.id], { relativeTo: this.route });
  }
}
