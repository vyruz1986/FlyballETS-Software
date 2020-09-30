import { FlyballETSWebPage } from './app.po';

describe('flyball-etsweb App', () => {
  let page: FlyballETSWebPage;

  beforeEach(() => {
    page = new FlyballETSWebPage();
  });

  it('should display welcome message', () => {
    page.navigateTo();
    expect(page.getParagraphText()).toEqual('Welcome to app!');
  });
});
