const App = {
  init() {
    Router.register('#/login', (ctx) => LoginPage.render(ctx));
    Router.register('#/problems', (ctx) => { ProblemListPage.render(ctx); });
    Router.register('#/problems/:id', (ctx) => { ProblemDetailPage.render(ctx); });
    Router.register('#/submissions/:id', (ctx) => { SubmissionResultPage.render(ctx); });
    Router.register('#/history', (ctx) => { HistoryPage.render(ctx); });
    Router.register('#/profile', (ctx) => { ProfilePage.render(ctx); });
    Router.register('#/admin', (ctx) => { AdminPage.render(ctx); });

    Auth.updateNav();
    Router.start();
  },
};

document.addEventListener('DOMContentLoaded', () => App.init());
