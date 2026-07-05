const LoginPage = {
  async render() {
    const main = document.getElementById('main-content');

    const isLogin = !(new URLSearchParams(window.location.hash.split('?')[1] || '')).get('register');

    main.innerHTML = `
      <div style="max-width:400px;margin:60px auto">
        <div class="card">
          <h2 class="page-title">${isLogin ? '登录' : '注册'}</h2>
          <div id="login-error" class="alert alert-error" style="display:none"></div>
          <div class="form-group">
            <label>用户名</label>
            <input type="text" id="login-username" placeholder="请输入用户名">
          </div>
          <div class="form-group">
            <label>密码</label>
            <input type="password" id="login-password" placeholder="请输入密码">
          </div>
          <button class="btn btn-primary" style="width:100%" onclick="LoginPage.submit(${isLogin})">
            ${isLogin ? '登录' : '注册'}
          </button>
          <p style="text-align:center;margin-top:16px;font-size:13px">
            ${isLogin
              ? '没有账号？<a href="#/login?register=1">立即注册</a>'
              : '已有账号？<a href="#/login">立即登录</a>'}
          </p>
        </div>
      </div>
    `;
  },

  async submit(isLogin) {
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;
    const errEl = document.getElementById('login-error');

    if (!username || !password) {
      errEl.textContent = '请填写用户名和密码';
      errEl.style.display = '';
      return;
    }

    try {
      if (isLogin) {
        await Auth.login(username, password);
      } else {
        await Auth.register(username, password);
        await Auth.login(username, password);
      }
      Auth.updateNav();
      Router.navigate('#/problems');
    } catch (e) {
      errEl.textContent = e.message || '操作失败';
      errEl.style.display = '';
    }
  },
};
