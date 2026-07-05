const ProblemListPage = {
  state: { page: 1, difficulty: '', problems: [], total: 0 },

  async render(ctx) {
    this.state.page = parseInt(ctx.query.page) || 1;
    this.state.difficulty = ctx.query.difficulty || '';

    const main = document.getElementById('main-content');
    main.innerHTML = `<h1 class="page-title">题库</h1><div id="problem-list-content">加载中...</div>`;
    await this.load();
  },

  async load() {
    try {
      const resp = await API.getProblems({
        page: this.state.page,
        size: 20,
        difficulty: this.state.difficulty,
      });

      const el = document.getElementById('problem-list-content');
      let html = '<div class="flex-row mb-16" style="gap:8px">';
      for (const d of ['', 'easy', 'medium', 'hard']) {
        const label = d || '全部';
        const cls = this.state.difficulty === d ? 'btn-primary' : '';
        html += `<button class="btn ${cls}" onclick="ProblemListPage.filter('${d}')">${label}</button>`;
      }
      html += '</div>';

      html += '<div class="card" style="padding:0"><table>';
      html += '<tr><th>#</th><th>题目</th><th>难度</th><th>通过率</th></tr>';

      const problems = resp.problems || [];
      for (const p of problems) {
        html += `<tr style="cursor:pointer" onclick="Router.navigate('#/problems/${p.id}')">
          <td style="width:60px">${p.id}</td>
          <td>${this.escape(p.title)}</td>
          <td><span class="badge badge-${p.difficulty}">${p.difficulty}</span></td>
          <td>${(p.ac_rate || 0).toFixed(1)}%</td>
        </tr>`;
      }
      html += '</table></div>';

      const total = resp.total || 0;
      const pages = Math.ceil(total / 20);
      html += '<div class="pagination">';
      for (let i = 1; i <= Math.min(pages, 10); i++) {
        html += `<button class="${i === this.state.page ? 'active' : ''}"
          onclick="ProblemListPage.goPage(${i})">${i}</button>`;
      }
      html += '</div>';

      el.innerHTML = html;
    } catch (e) {
      document.getElementById('problem-list-content').innerHTML =
        '<div class="alert alert-error">加载失败: ' + e.message + '</div>';
    }
  },

  filter(d) {
    this.state.difficulty = d;
    this.state.page = 1;
    Router.navigate('#/problems?difficulty=' + d + '&page=1');
  },

  goPage(p) {
    this.state.page = p;
    Router.navigate('#/problems?difficulty=' + this.state.difficulty + '&page=' + p);
  },

  escape(str) {
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },
};
