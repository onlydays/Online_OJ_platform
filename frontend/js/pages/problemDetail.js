const ProblemDetailPage = {
  state: { problem: null, editor: null },

  async render(ctx) {
    const id = ctx.params[0];
    const main = document.getElementById('main-content');
    main.innerHTML = '<div class="page-container"><p>加载中...</p></div>';

    try {
      const resp = await API.getProblem(id);
      this.state.problem = resp.data;
      this.renderProblem();
    } catch (e) {
      main.innerHTML = '<div class="page-container"><div class="alert alert-error">' + this.esc(e.message) + '</div></div>';
    }
  },

  renderProblem() {
    const p = this.state.problem;
    const main = document.getElementById('main-content');

    main.innerHTML = `
<div class="page-container">
  <div style="margin-bottom:10px">
    <a href="#/problems" style="color:#2563eb;text-decoration:none;font-size:13px">&larr; 返回列表</a>
  </div>

  <div class="split-layout">
    <div class="split-left" style="width:45%">
      <div class="card">
        <h1 style="font-size:18px;margin-bottom:8px">${this.esc(p.id)}. ${this.esc(p.title)}</h1>
        <div class="flex-row mb-16" style="gap:10px">
          <span class="badge badge-${p.difficulty}">${p.difficulty==='easy'?'简单':p.difficulty==='medium'?'中等':'困难'}</span>
          <span class="text-sm">时间: ${p.time_limit_ms}ms</span>
          <span class="text-sm">内存: ${(p.memory_limit_kb/1024).toFixed(0)}MB</span>
        </div>
        <div style="white-space:pre-wrap;line-height:1.8;font-size:14px">${this.esc(p.description)}</div>
      </div>

      ${p.sample_testcases && p.sample_testcases.length ? `
      <div class="card">
        <h3 style="font-size:14px;margin-bottom:10px">样例</h3>
        ${p.sample_testcases.map((tc, i) => `
          <div style="margin-bottom:10px;font-size:13px">
            <div style="font-weight:600">输入 #${i+1}</div>
            <pre style="background:#f8fafc;padding:8px;border-radius:4px;margin:4px 0">${this.esc(tc.input)}</pre>
            <div style="font-weight:600">输出 #${i+1}</div>
            <pre style="background:#f8fafc;padding:8px;border-radius:4px;margin:4px 0">${this.esc(tc.expected_output)}</pre>
          </div>
        `).join('')}
      </div>` : ''}
    </div>

    <div class="split-right" style="width:55%">
      <div class="editor-section">
        <div class="editor-toolbar">
          <span>代码编辑</span>
          <select id="lang-select" onchange="ProblemDetailPage.switchLang()">
            <option value="c_cpp">C++17</option>
            <option value="java">Java</option>
            <option value="python">Python</option>
          </select>
        </div>
        <div id="ace-editor"></div>
      </div>

      <div class="card">
        <div class="flex-between mb-16">
          <span style="font-weight:600;font-size:14px">测试用例</span>
          <span class="text-sm">${p.sample_testcases ? p.sample_testcases.length : 0} 个用例</span>
        </div>
        <div id="tc-results">
          ${(p.sample_testcases||[]).map((tc, i) => `
            <div class="tc-item">
              <span class="tc-status" style="color:#d1d5db"><i class="fa-solid fa-circle"></i></span>
              <span class="tc-io">输入: ${this.esc(tc.input || '').substring(0,40)} &rarr; ${this.esc(tc.expected_output || '').substring(0,40)}</span>
              <span class="tc-time">-</span>
            </div>
          `).join('')}
        </div>

        <div id="judge-summary" style="display:none" class="alert alert-success mt-16"></div>

        <div class="flex-row mt-16" style="gap:8px">
          <button class="btn btn-primary" onclick="ProblemDetailPage.submit()">运行代码</button>
          <button class="btn btn-outline" onclick="ProblemDetailPage.clearEditor()">清空</button>
        </div>
      </div>
    </div>
  </div>
</div>`;

    this.initEditor();
  },

  initEditor() {
    this.state.editor = ace.edit('ace-editor');
    this.state.editor.setTheme('ace/theme/tomorrow');
    this.state.editor.setFontSize(14);
    this.state.editor.setOptions({ showPrintMargin: false, tabSize: 2, useSoftTabs: true });
    this.state.editor.session.setMode('ace/mode/c_cpp');
    this.state.editor.setValue(this.defaultCode_cpp(), -1);
  },

  defaultCode_cpp() {
    return '#include <iostream>\nusing namespace std;\n\nint main() {\n    \n    return 0;\n}';
  },

  switchLang() {
    const lang = document.getElementById('lang-select').value;
    const codes = {
      c_cpp: this.defaultCode_cpp(),
      java: 'import java.util.*;\n\npublic class Main {\n    public static void main(String[] args) {\n        \n    }\n}',
      python: '# Write your solution here\n',
    };
    this.state.editor.session.setMode('ace/mode/' + lang);
    this.state.editor.setValue(codes[lang], -1);
  },

  async submit() {
    const code = this.state.editor.getValue();
    if (!code.trim()) { alert('代码不能为空'); return; }

    const summary = document.getElementById('judge-summary');
    summary.style.display = '';
    summary.textContent = '判题中...';
    summary.className = 'alert alert-info mt-16';

    const items = document.querySelectorAll('#tc-results .tc-item');
    items.forEach(el => {
      el.querySelector('.tc-status').innerHTML = '<i class="fa-solid fa-circle" style="color:#d1d5db"></i>';
      el.querySelector('.tc-time').textContent = '...';
    });

    try {
      const resp = await API.submit(this.state.problem.id, code);
      Router.navigate('#/submissions/' + resp.data.id);
    } catch (e) {
      summary.textContent = '提交失败: ' + e.message;
      summary.className = 'alert alert-error mt-16';
    }
  },

  clearEditor() { this.state.editor.setValue('', -1); },

  esc(s) {
    if (!s) return '';
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
  },
};
