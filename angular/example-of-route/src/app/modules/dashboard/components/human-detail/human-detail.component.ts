import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { switchMap, map } from 'rxjs/operators';

import { HumanService } from '../../services/human.service';

@Component({
  selector: 'app-human-detail',
  templateUrl: './human-detail.component.html',
  styleUrls: ['./human-detail.component.sass']
})
export class HumanDetailComponent implements OnInit {
  constructor(private route: ActivatedRoute,
    private service: HumanService) { }

  // 必须使用订阅, 不然 id 改变时 item 不会更新
  id = this.route.paramMap.pipe(map(params => params.get("id")));
  item$ = this.id.pipe(switchMap(id => this.service.getHuman(+id)));

  ngOnInit() {
    this.route.url.subscribe(urls => {
      let sp = this.route.snapshot;
      console.log("[Human Detail] init");
      console.log("[Human Detail] route:", sp.toString())
      console.log("[Human Detail]\tRoot:", sp.root.toString());
      console.log("[Human Detail]\tParent:", sp.parent.toString());
      for (let ch of sp.children) {
        console.log("[Human Detail]\tChild:", ch.toString())
      }
    })
  }

}
