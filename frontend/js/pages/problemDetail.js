const ProblemDetailPage = {
  state: { problem: null },

  async render(ctx) {
    const id = ctx.params[0];
    const main = document.getElementById('main-content');
    main.innerHTML = '<p>加载中...</p>';

    try {
      const resp = await API.getProblem(id);
      this.state.problem = resp.data;
      this.renderProblem();
    } catch (e) {
      main.innerHTML = '<div class="alert alert-error">' + e.message + '</div>';
    }
  },

  renderProblem() {
    const p = this.state.problem;
    const main = document.getElementById('main-content');

    main.innerHTML = `
      <div class="flex-between mb-16">
        <h1 class="page-title" style="margin:0">${this.escape(p.id)}. ${this.escape(p.title)}</h1>
        <span class="badge badge-${p.difficulty}">${p.difficulty}</span>
      </div>

      <div class="flex-row mb-16" style="color:#888;font-size:13px">
        <span>时间限制: ${p.time_limit_ms}ms</span>
        <span>内存限制: ${(p.memory_limit_kb / 1024).toFixed(0)}MB</span>
        <span>通过率: ${(p.ac_rate || 0).toFixed(1)}%</span>
      </div>

      <div class="card">
        <div style="white-space:pre-wrap;line-height:1.8">${this.escape(p.description)}</div>
      </div>

      ${p.sample_testcases && p.sample_testcases.length ? `
      <div class="card">
        <h3 style="margin-bottom:12px">样例</h3>
        ${p.sample_testcases.map((tc, i) => `
          <div class="mb-16">
            <div style="font-weight:600;margin-bottom:4px">输入 #${i + 1}</div>
            <pre style="background:#f8f8f8;padding:8px;border-radius:4px;font-size:13px">${this.escape(tc.input)}</pre>
            <div style="font-weight:600;margin:8px 0 4px">输出 #${i + 1}</div>
            <pre style="background:#f8f8f8;padding:8px;border-radius:4px;font-size:13px">${this.escape(tc.expected_output)}</pre>
          </div>
        `).join('')}
      </div>` : ''}

      <div class="card">
        <div class="editor-container">
          <div class="editor-header">
            <span>C++</span>
            <span id="editor-status" style="color:#888"></span>
          </div>
          <textarea id="code-editor" style="width:100%;height:400px;border:none;padding:12px;font-family:monospace;font-size:14px;resize:vertical;outline:none">#include &lt;iostream&gt;
using namespace std;

int main() {

    return 0;
}</textarea>
        </div>
        <div id="submit-error" class="alert alert-error" style="display:none"></div>
        <button class="btn btn-primary" onclick="ProblemDetailPage.submit()" style="width:100%;padding:12px">
          提交代码
        </button>
      </div>
    `;
  },

  async submit() {
    const code = document.getElementById('code-editor').value;
    const errEl = document.getElementById('submit-error');
    errEl.style.display = 'none';

    if (!code.trim()) {
      errEl.textContent = '代码不能为空';
      errEl.style.display = '';
      return;
    }

    try {
      const resp = await API.submit(this.state.problem.id, code);
      Router.navigate('#/submissions/' + resp.data.id);
    } catch (e) {
      errEl.textContent = e.message;
      errEl.style.display = '';
    }
  },

  escape(str) {
    if (!str) return '';
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },
};
