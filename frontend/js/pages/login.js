const LoginPage = {
  async render() {
    if (Auth.isLoggedIn()) {
      Router.navigate('#/problems');
      return;
    }

    const main = document.getElementById('main-content');
    const isLogin = !(new URLSearchParams(window.location.hash.split('?')[1] || '')).get('register');

    main.innerHTML = `
      <div style="max-width:380px;margin:80px auto 0">
        <div style="text-align:center;margin-bottom:28px">
          <div style="font-size:28px;font-weight:800;color:#262626;letter-spacing:-1px">Online OJ</div>
          <div style="font-size:13px;color:#8c8c8c;margin-top:4px">${isLogin ? '欢迎回来' : '创建账号'}</div>
        </div>
        <div class="card">
          <div id="login-error" class="alert alert-error" style="display:none"></div>
          <div class="form-group">
            <label>用户名</label>
            <input type="text" id="login-username" placeholder="请输入用户名" autocomplete="username">
          </div>
          <div class="form-group">
            <label>密码</label>
            <input type="password" id="login-password" placeholder="请输入密码" autocomplete="${isLogin ? 'current-password' : 'new-password'}">
          </div>
          <button class="btn btn-primary" style="width:100%;padding:10px" onclick="LoginPage.handleSubmit(${isLogin})">
            ${isLogin ? '登录' : '注册'}
          </button>
          <p style="text-align:center;margin-top:16px;font-size:12px;color:#8c8c8c">
            ${isLogin
              ? '没有账号？<a href="#/login?register=1" style="color:#007aff;text-decoration:none">立即注册</a>'
              : '已有账号？<a href="#/login" style="color:#007aff;text-decoration:none">立即登录</a>'}
          </p>
        </div>
      </div>
    `;
  },

  async handleSubmit(isLogin) {
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;
    const errEl = document.getElementById('login-error');

    if (!username || !password) {
      errEl.textContent = '请填写用户名和密码';
      errEl.style.display = '';
      return;
    }
    if (password.length < 3) {
      errEl.textContent = '密码至少 3 位';
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
      window.location.hash = '#/problems';
    } catch (e) {
      errEl.textContent = e.message || '操作失败';
      errEl.style.display = '';
    }
  },
};
