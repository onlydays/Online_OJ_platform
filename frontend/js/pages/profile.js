const ProfilePage = {
  async render() {
    const main = document.getElementById('main-content');

    if (!Auth.isLoggedIn()) {
      main.innerHTML = '<div class="alert alert-error">请先登录</div>';
      setTimeout(() => Router.navigate('#/login'), 1000);
      return;
    }

    main.innerHTML = '<h1 class="page-title">个人主页</h1><div id="profile-content">加载中...</div>';

    try {
      const [profileResp, statsResp] = await Promise.all([
        API.getProfile(),
        API.getStats(),
      ]);

      const user = profileResp.data;
      const stats = statsResp.data;

      document.getElementById('profile-content').innerHTML = `
        <div class="card">
          <h2 style="margin-bottom:8px">${this.escape(user.username)}</h2>
          <span class="badge badge-${user.role === 'admin' ? 'hard' : 'easy'}">${user.role}</span>
          <span style="color:#888;font-size:13px;margin-left:8px">
            加入于 ${new Date(user.created_at).toLocaleDateString()}
          </span>
        </div>

        <div class="stats-grid">
          <div class="stat-card">
            <div class="value">${stats.total_submissions || 0}</div>
            <div class="label">总提交</div>
          </div>
          <div class="stat-card">
            <div class="value">${stats.accepted || 0}</div>
            <div class="label">通过</div>
          </div>
          <div class="stat-card">
            <div class="value">${(stats.ac_rate || 0).toFixed(1)}%</div>
            <div class="label">通过率</div>
          </div>
          <div class="stat-card">
            <div class="value">${stats.problems_solved || 0}</div>
            <div class="label">已解决</div>
          </div>
        </div>
      `;
    } catch (e) {
      document.getElementById('profile-content').innerHTML =
        '<div class="alert alert-error">' + e.message + '</div>';
    }
  },

  escape(str) {
    if (!str) return '';
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },
};
