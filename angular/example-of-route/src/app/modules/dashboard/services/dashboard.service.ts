import { Injectable } from '@angular/core';
import { Resolve, ActivatedRouteSnapshot } from '@angular/router';
import { Observable, of } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class DashboardService implements Resolve<itemInfo[]> {
  constructor() { }

  async resolve(route: ActivatedRouteSnapshot) {
    return LIST;
  }

  list(): Observable<itemInfo[]> {
    return of(LIST);
  }

  get(name: string): Observable<itemInfo> {
    return of(LIST.find(info => info.name == name));
  }
}

const LIST: itemInfo[] = [
  { name: "Human", path: "/dashboard/human" }
]

interface itemInfo {
  name: string,
  path: string,
}
