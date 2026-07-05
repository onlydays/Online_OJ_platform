const HistoryPage = {
  state: { page: 1, submissions: [] },

  async render(ctx) {
    this.state.page = parseInt(ctx.query.page) || 1;
    const main = document.getElementById('main-content');
    main.innerHTML = '<h1 class="page-title">提交记录</h1><div id="history-content">加载中...</div>';
    await this.load();
  },

  async load() {
    try {
      const resp = await API.getSubmissions({ page: this.state.page, size: 20 });
      const subs = resp.submissions || [];

      const statusMap = {
        accepted: ['AC', 'badge-ac'],
        wrong_answer: ['WA', 'badge-wa'],
        compilation_error: ['CE', 'badge-ce'],
        time_limit_exceeded: ['TLE', 'badge-wa'],
        memory_limit_exceeded: ['MLE', 'badge-wa'],
        runtime_error: ['RE', 'badge-wa'],
        presentation_error: ['PE', 'badge-wa'],
        pending: ['PEND', 'badge-pending'],
      };

      let html = '<div class="card" style="padding:0"><table>';
      html += '<tr><th>ID</th><th>题目</th><th>状态</th><th>耗时</th><th>内存</th><th>时间</th></tr>';
      for (const s of subs) {
        const info = statusMap[s.status] || [s.status, ''];
        html += `<tr style="cursor:pointer" onclick="Router.navigate('#/submissions/${s.id}')">
          <td>#${s.id}</td>
          <td>${s.problem_id}</td>
          <td><span class="badge ${info[1]}">${info[0]}</span></td>
          <td>${s.time_used_ms}ms</td>
          <td>${(s.memory_used_kb / 1024).toFixed(1)}MB</td>
          <td style="font-size:12px;color:#888">${new Date(s.created_at).toLocaleString()}</td>
        </tr>`;
      }
      html += '</table></div>';

      const total = resp.total || 0;
      const pages = Math.ceil(total / 20);
      html += '<div class="pagination">';
      for (let i = 1; i <= Math.min(pages, 10); i++) {
        html += `<button class="${i === this.state.page ? 'active' : ''}"
          onclick="HistoryPage.goPage(${i})">${i}</button>`;
      }
      html += '</div>';

      document.getElementById('history-content').innerHTML = html;
    } catch (e) {
      document.getElementById('history-content').innerHTML =
        '<div class="alert alert-error">' + e.message + '</div>';
    }
  },

  goPage(p) {
    Router.navigate('#/history?page=' + p);
  },
};
