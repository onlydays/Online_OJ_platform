const Router = {
  routes: {},
  currentPage: null,

  register(hash, handler) {
    this.routes[hash] = handler;
  },

  navigate(hash) {
    window.location.hash = hash;
  },

  async handleRoute() {
    const hash = window.location.hash || '#/problems';
    const base = hash.split('?')[0];

    let matched = null;
    let params = [];

    for (const [pattern, handler] of Object.entries(this.routes)) {
      const regex = new RegExp('^' + pattern.replace(/:\w+/g, '(\\w+)') + '$');
      const match = base.match(regex);
      if (match) {
        matched = handler;
        params = match.slice(1);
        break;
      }
    }

    if (!matched) {
      this.routes['#/problems']({});
      return;
    }

    // Extract query params
    const query = {};
    const qs = hash.split('?')[1];
    if (qs) {
      new URLSearchParams(qs).forEach((v, k) => query[k] = v);
    }

    await matched({ params, query });
  },

  start() {
    window.addEventListener('hashchange', () => this.handleRoute());
    this.handleRoute();
  },
};
