const Auth = {
  getToken() { return localStorage.getItem('token'); },
  getUser() {
    try { return JSON.parse(localStorage.getItem('user')); }
    catch (e) { return null; }
  },

  setSession(token, user) {
    localStorage.setItem('token', token);
    localStorage.setItem('user', JSON.stringify(user));
  },

  isLoggedIn() { return !!this.getToken(); },
  isAdmin() {
    const u = this.getUser();
    return u && u.role === 'admin';
  },

  logout() {
    localStorage.removeItem('token');
    localStorage.removeItem('user');
    this.updateNav();
    window.location.hash = '#/login';
  },

  async login(username, password) {
    const resp = await API.login(username, password);
    this.setSession(resp.data.token, resp.data.user);
    this.updateNav();
    return resp.data.user;
  },

  async register(username, password) {
    const resp = await API.register(username, password);
    return resp.data;
  },

  updateNav() {
    const loginEl = document.getElementById('nav-login');
    const logoutEl = document.getElementById('nav-logout');
    const adminEl = document.getElementById('nav-admin');
    const userEl = document.getElementById('nav-user');
    const avatarEl = document.getElementById('nav-avatar');
    const usernameEl = document.getElementById('nav-username');

    const user = this.getUser();

    if (this.isLoggedIn() && user) {
      if (loginEl) loginEl.style.display = 'none';
      if (logoutEl) logoutEl.style.display = '';
      if (adminEl) adminEl.style.display = this.isAdmin() ? '' : 'none';
      if (userEl) {
        userEl.style.display = '';
        if (avatarEl) avatarEl.textContent = user.username.charAt(0).toUpperCase();
        if (usernameEl) usernameEl.textContent = user.username;
      }
    } else {
      if (loginEl) loginEl.style.display = '';
      if (logoutEl) logoutEl.style.display = 'none';
      if (adminEl) adminEl.style.display = 'none';
      if (userEl) userEl.style.display = 'none';
    }
  },

  requireAuth() {
    if (!this.isLoggedIn()) {
      window.location.hash = '#/login';
      return false;
    }
    return true;
  },
};
