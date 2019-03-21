import { TestBed } from '@angular/core/testing';

import { HumanService } from './human.service';

describe('HumanService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: HumanService = TestBed.get(HumanService);
    expect(service).toBeTruthy();
  });
});
