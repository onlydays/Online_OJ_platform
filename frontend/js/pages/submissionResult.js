const SubmissionResultPage = {
  async render(ctx) {
    const id = ctx.params[0];
    const main = document.getElementById('main-content');
    main.innerHTML = '<p>加载判题结果...</p>';

    const show = async () => {
      try {
        const resp = await API.getSubmission(id);
        const s = resp.data;
        this.renderResult(s);
      } catch (e) {
        main.innerHTML = '<div class="alert alert-error">' + e.message + '</div>';
      }
    };

    await show();

    // Auto-refresh if pending/running
    const interval = setInterval(async () => {
      try {
        const resp = await API.getSubmission(id);
        if (resp.data.status !== 'pending' && resp.data.status !== 'compiling' && resp.data.status !== 'running') {
          clearInterval(interval);
        }
        this.renderResult(resp.data);
      } catch (e) {
        clearInterval(interval);
      }
    }, 2000);
  },

  renderResult(s) {
    const main = document.getElementById('main-content');
    const statusMap = {
      accepted: ['Accepted', 'badge-ac'],
      wrong_answer: ['Wrong Answer', 'badge-wa'],
      compilation_error: ['Compilation Error', 'badge-ce'],
      time_limit_exceeded: ['Time Limit Exceeded', 'badge-wa'],
      memory_limit_exceeded: ['Memory Limit Exceeded', 'badge-wa'],
      runtime_error: ['Runtime Error', 'badge-wa'],
      presentation_error: ['Presentation Error', 'badge-wa'],
      pending: ['Pending', 'badge-pending'],
      compiling: ['Compiling', 'badge-pending'],
      running: ['Running', 'badge-pending'],
    };

    const info = statusMap[s.status] || [s.status, ''];

    main.innerHTML = `
      <h1 class="page-title">提交结果</h1>
      <div class="card">
        <div class="flex-between mb-16">
          <span class="badge ${info[1]}" style="font-size:16px;padding:4px 16px">${info[0]}</span>
          <span style="color:#888;font-size:13px">
            ${s.time_used_ms}ms · ${(s.memory_used_kb / 1024).toFixed(1)}MB
          </span>
        </div>

        ${s.compile_error ? `
        <div class="alert alert-error mb-16">
          <strong>编译错误:</strong>
          <pre style="margin-top:8px;white-space:pre-wrap;font-size:12px">${this.escape(s.compile_error)}</pre>
        </div>` : ''}

        ${s.results && s.results.length ? `
        <h3 style="margin-bottom:8px">测试用例结果 (${s.results.length})</h3>
        <ul class="result-list">
          ${s.results.map((r, i) => `
            <li class="result-item">
              <span>
                <span class="badge ${r.status === 'accepted' ? 'badge-ac' : 'badge-wa'}">${r.status}</span>
                <span style="margin-left:8px;font-size:12px;color:#888">用例 #${i + 1}</span>
              </span>
              <span style="font-size:12px;color:#888">${r.time_used_ms}ms · ${(r.memory_used_kb / 1024).toFixed(1)}MB</span>
            </li>
          `).join('')}
        </ul>` : ''}

        <div class="mt-16">
          <button class="btn" onclick="Router.navigate('#/problems/${s.problem_id}')">返回题目</button>
          <button class="btn btn-primary" style="margin-left:8px" onclick="Router.navigate('#/history')">查看记录</button>
        </div>
      </div>
    `;
  },

  escape(str) {
    if (!str) return '';
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },
};
