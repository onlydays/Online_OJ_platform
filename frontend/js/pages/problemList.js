const ProblemListPage = {
  state: { difficulty: '', page: 1, problems: [], total: 0 },

  async render(ctx) {
    this.state.difficulty = ctx.query.difficulty || '';
    this.state.page = parseInt(ctx.query.page) || 1;

    const user = Auth.getUser();
    const main = document.getElementById('main-content');

    main.innerHTML = `
<div class="split-layout" style="margin-top:12px;min-height:calc(100vh - 86px)">
  <aside class="split-left" style="background:#fff;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,.06);padding:0;overflow:hidden;display:flex;flex-direction:column">
    ${user ? `
    <div style="padding:18px 16px;border-bottom:1px solid #f0f0f0;display:flex;align-items:center;gap:10px">
      <div style="width:40px;height:40px;border-radius:50%;background:#3b82f6;color:#fff;display:flex;align-items:center;justify-content:center;font-weight:700;font-size:16px;flex-shrink:0">${user.username.charAt(0).toUpperCase()}</div>
      <div><div style="font-size:14px;font-weight:600">${this.esc(user.username)}</div><div style="font-size:11px;color:#8c8c8c">${user.role}</div></div>
    </div>` : ''}
    <div style="padding:12px 8px;border-bottom:1px solid #f0f0f0">
      <a href="#/" style="text-decoration:none;color:#555;font-size:14px;padding:8px 12px;border-radius:8px;display:flex;align-items:center;gap:8px;transition:all .15s"><i class="fa-solid fa-house"></i> 首页</a>
      <a href="#/problems" style="text-decoration:none;color:#3b82f6;font-size:14px;padding:8px 12px;border-radius:8px;display:flex;align-items:center;gap:8px;font-weight:600;background:#eff6ff"><i class="fa-solid fa-list"></i> 题库</a>
      <a href="#/history" style="text-decoration:none;color:#555;font-size:14px;padding:8px 12px;border-radius:8px;display:flex;align-items:center;gap:8px;transition:all .15s"><i class="fa-solid fa-clock-rotate-left"></i> 提交记录</a>
      ${user && user.role==='admin' ? `<a href="#/admin" style="text-decoration:none;color:#555;font-size:14px;padding:8px 12px;border-radius:8px;display:flex;align-items:center;gap:8px;transition:all .15s"><i class="fa-solid fa-gear"></i> 管理后台</a>` : ''}
    </div>
    <div style="padding:16px;flex:1">
      <h3 style="font-size:14px;font-weight:700;margin-bottom:10px;display:flex;justify-content:space-between">题目列表 <span style="background:#eff6ff;color:#3b82f6;padding:0 8px;border-radius:10px;font-size:11px" id="prob-count">-</span></h3>
      <div class="filter-tags">
        <button class="${this.state.difficulty===''?'active':''}" onclick="ProblemListPage.filter('')">全部</button>
        <button class="${this.state.difficulty==='easy'?'active':''}" onclick="ProblemListPage.filter('easy')">简单</button>
        <button class="${this.state.difficulty==='medium'?'active':''}" onclick="ProblemListPage.filter('medium')">中等</button>
        <button class="${this.state.difficulty==='hard'?'active':''}" onclick="ProblemListPage.filter('hard')">困难</button>
      </div>
    </div>
    <div style="padding:16px;border-top:1px solid #f0f0f0"><a href="#" onclick="Auth.logout()" style="color:#999;text-decoration:none;font-size:13px"><i class="fa-solid fa-right-from-bracket"></i> 退出</a></div>
  </aside>

  <div class="split-right" id="prob-list-container" style="overflow-y:auto">
    <h2 style="font-size:20px;font-weight:700;margin-bottom:16px" id="section-title">全部题目</h2>
    <div class="problem-grid" id="problem-grid">加载中...</div>
  </div>
</div>`;
    await this.load();
  },

  async load() {
    try {
      const resp = await API.getProblems({ difficulty: this.state.difficulty, page: 1, size: 100 });
      this.state.problems = resp.problems || [];
      this.state.total = resp.total || this.state.problems.length;

      const labels = {'':'全部题目','easy':'简单题目','medium':'中等题目','hard':'困难题目'};
      document.getElementById('section-title').textContent = labels[this.state.difficulty] || '全部题目';
      document.getElementById('prob-count').textContent = this.state.total;

      const diffLabels = {easy:'简单',medium:'中等',hard:'困难'};
      document.getElementById('problem-grid').innerHTML = this.state.problems.map(p => `
        <div class="problem-card" onclick="Router.navigate('#/problems/${p.id}')">
          <span class="pnum">#${p.id}</span>
          <span class="ptitle">${this.esc(p.title)}</span>
          <span class="pmeta">
            <span class="difficulty ${p.difficulty}">${diffLabels[p.difficulty] || p.difficulty}</span>
            <span class="ac-rate">${(p.ac_rate||0).toFixed(1)}%</span>
          </span>
        </div>
      `).join('');
    } catch (e) {
      document.getElementById('problem-grid').innerHTML = '<div class="alert-error" style="padding:16px;border-radius:8px">' + this.esc(e.message) + '</div>';
    }
  },

  filter(d) {
    this.state.difficulty = d;
    Router.navigate('#/problems' + (d ? '?difficulty=' + d : ''));
  },

  esc(s) {
    if (!s) return '';
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
  },
};
