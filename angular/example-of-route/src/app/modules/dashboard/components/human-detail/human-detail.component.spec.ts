import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { HumanDetailComponent } from './human-detail.component';

describe('HumanDetailComponent', () => {
  let component: HumanDetailComponent;
  let fixture: ComponentFixture<HumanDetailComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ HumanDetailComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(HumanDetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
