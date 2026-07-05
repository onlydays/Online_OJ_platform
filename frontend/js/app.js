const App = {
  init() {
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

    Auth.updateNav();
    Router.start();
  },
};

document.addEventListener('DOMContentLoaded', () => App.init());
