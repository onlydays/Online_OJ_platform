const ProblemListPage = {
  state: { difficulty: '', page: 1 },

  async render(ctx) {
    this.state.difficulty = ctx.query.difficulty || '';
    this.state.page = parseInt(ctx.query.page) || 1;

    const user = Auth.getUser();
    const main = document.getElementById('main-content');

    main.innerHTML = `
<div class="split-layout" style="margin-top:20px">
  <div class="split-left">
    <div class="card">
      ${user ? `
      <div style="text-align:center;margin-bottom:14px">
        <div class="avatar-small" style="width:40px;height:40px;font-size:18px;margin:0 auto 6px">${user.username.charAt(0).toUpperCase()}</div>
        <div style="font-weight:600;font-size:14px">${this.esc(user.username)}</div>
        <div style="font-size:11px;color:#8c8c8c">${user.role}</div>
      </div>` : ''}
      <div class="flex-between mb-16">
        <span style="font-weight:700;font-size:14px">题目列表</span>
        <span style="font-size:12px;background:#eff6ff;color:#2563eb;padding:1px 8px;border-radius:10px" id="prob-count">-</span>
      </div>
      <div class="filter-tags">
        <button class="${this.state.difficulty===''?'active':''}" onclick="ProblemListPage.filter('')">全部</button>
        <button class="${this.state.difficulty==='easy'?'active':''}" onclick="ProblemListPage.filter('easy')">简单</button>
        <button class="${this.state.difficulty==='medium'?'active':''}" onclick="ProblemListPage.filter('medium')">中等</button>
        <button class="${this.state.difficulty==='hard'?'active':''}" onclick="ProblemListPage.filter('hard')">困难</button>
      </div>
    </div>
  </div>
  <div class="split-right" id="prob-list-container">加载中...</div>
</div>`;

    await this.load();
  },

  async load() {
    const container = document.getElementById('prob-list-container');
    try {
      const resp = await API.getProblems({ difficulty: this.state.difficulty, page: this.state.page, size: 50 });
      const problems = resp.problems || [];
      document.getElementById('prob-count').textContent = resp.total || problems.length;

      container.innerHTML = '<div class="card" style="padding:0"><table>' +
        '<tr><th>#</th><th>题目</th><th>难度</th><th>通过率</th></tr>' +
        problems.map(p => `<tr onclick="Router.navigate('#/problems/${p.id}')">
          <td style="width:60px;color:#8c8c8c">${p.id}</td>
          <td>${this.esc(p.title)}</td>
          <td><span class="badge badge-${p.difficulty}">${p.difficulty==='easy'?'简单':p.difficulty==='medium'?'中等':'困难'}</span></td>
          <td style="width:80px">${(p.ac_rate||0).toFixed(1)}%</td>
        </tr>`).join('') +
        '</table></div>';
    } catch (e) {
        container.innerHTML = '<div class="alert alert-error">' + this.esc(e.message) + '</div>';
    }
  },

  filter(d) {
    this.state.difficulty = d;
    this.state.page = 1;
    Router.navigate('#/problems' + (d ? '?difficulty=' + d : ''));
  },

  esc(s) {
    if (!s) return '';
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
  },
};
