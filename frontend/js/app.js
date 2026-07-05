const App = {
  init() {
    Router.register('#/', (ctx) => HomePage.render());
    Router.register('#/login', (ctx) => LoginPage.render(ctx));

    Router.register('#/problems', (ctx) => {
      if (!Auth.requireAuth()) return;
      ProblemListPage.render(ctx);
    });
    Router.register('#/problems/:id', (ctx) => {
      if (!Auth.requireAuth()) return;
      ProblemDetailPage.render(ctx);
    });
    Router.register('#/submissions/:id', (ctx) => {
      if (!Auth.requireAuth()) return;
      SubmissionResultPage.render(ctx);
    });
    Router.register('#/history', (ctx) => {
      if (!Auth.requireAuth()) return;
      HistoryPage.render(ctx);
    });
    Router.register('#/profile', (ctx) => {
      if (!Auth.requireAuth()) return;
      ProfilePage.render(ctx);
    });
    Router.register('#/admin', (ctx) => {
      if (!Auth.requireAuth()) return;
      AdminPage.render(ctx);
    });

    window.addEventListener('hashchange', () => this.highlightNav());
    Auth.updateNav();
    Router.start();
  },

  highlightNav() {
    const hash = window.location.hash || '#/';
    const links = document.querySelectorAll('#nav-links a');
    links.forEach(a => {
      const href = a.getAttribute('href');
      a.classList.toggle('active', hash === href || (hash.startsWith(href) && href !== '#/'));
    });
  },
};

document.addEventListener('DOMContentLoaded', () => App.init());
