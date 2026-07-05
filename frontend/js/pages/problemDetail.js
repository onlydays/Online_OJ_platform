const ProblemDetailPage = {
  state: { problem: null, editor: null },

  async render(ctx) {
    const id = ctx.params[0];
    const main = document.getElementById('main-content');
    main.innerHTML = '<div style="display:flex;align-items:center;justify-content:center;height:200px"><i class="fa-solid fa-spinner fa-spin"></i> 加载中...</div>';

    try {
      const resp = await API.getProblem(id);
      this.state.problem = resp.data;
      this.renderProblem();
    } catch (e) {
      main.innerHTML = '<div style="padding:20px"><div class="alert-error" style="padding:16px;border-radius:8px">' + this.esc(e.message) + '</div></div>';
    }
  },

  renderProblem() {
    const p = this.state.problem;
    const diffLabels = {easy:'简单',medium:'中等',hard:'困难'};
    const main = document.getElementById('main-content');

    main.innerHTML = `
<div style="height:calc(100vh - 50px);display:flex;flex-direction:column">
  <div style="height:50px;background:#fff;border-bottom:1px solid #e5e7eb;display:flex;align-items:center;justify-content:space-between;padding:0 16px;flex-shrink:0">
    <a href="#/problems" style="color:#2563eb;text-decoration:none;font-size:14px">&larr; 返回列表</a>
    <span style="font-weight:800;font-size:16px;color:#2563eb">OJ</span>
    <span style="font-size:13px;color:#555">${this.esc(Auth.getUser()?.username || '')}</span>
  </div>

  <div style="display:flex;flex:1;overflow:hidden">
    <div class="panel-left" style="width:45%;overflow-y:auto;padding:20px;border-right:1px solid #e5e7eb">
      <div style="margin-bottom:16px">
        <h1 style="font-size:22px;font-weight:700;margin-bottom:4px">${this.esc(p.title)}</h1>
        <div style="color:#94a3b8;font-size:13px;margin-bottom:8px">#${p.id}</div>
        <div style="display:flex;gap:10px;align-items:center;font-size:13px;color:#64748b;flex-wrap:wrap">
          <span class="difficulty ${p.difficulty}">${diffLabels[p.difficulty] || p.difficulty}</span>
          <span>通过率 ${(p.ac_rate||0).toFixed(1)}%</span>
          <span><i class="fa-regular fa-clock"></i> ${p.time_limit_ms}ms</span>
          <span><i class="fa-solid fa-microchip"></i> ${(p.memory_limit_kb/1024).toFixed(0)}MB</span>
        </div>
      </div>

      <div style="background:#fff;border-radius:12px;padding:20px;box-shadow:0 1px 3px rgba(0,0,0,.06);line-height:1.8;font-size:14px;margin-bottom:16px">
        ${this.formatDesc(p.description)}
      </div>

      ${p.sample_testcases && p.sample_testcases.length ? `
      <div style="background:#fff;border-radius:12px;padding:20px;box-shadow:0 1px 3px rgba(0,0,0,.06)">
        <h3 style="font-size:14px;margin-bottom:10px">示例</h3>
        ${p.sample_testcases.map((tc,i) => `
          <pre style="background:#f8fafc;padding:12px;border-radius:8px;font-size:13px;font-family:monospace;border-left:3px solid #2563eb;margin-bottom:10px;white-space:pre-wrap">输入: ${this.esc(tc.input)}\n输出: ${this.esc(tc.expected_output)}</pre>
        `).join('')}
      </div>` : ''}
    </div>

    <div class="panel-right" style="width:55%;display:flex;flex-direction:column;overflow:hidden">
      <div style="flex:1;display:flex;flex-direction:column;overflow:hidden;padding:12px 12px 8px">
        <div style="display:flex;justify-content:space-between;align-items:center;padding:8px 14px;background:#2d2d2d;border-radius:10px 10px 0 0;font-size:13px;color:#ccc">
          <span><i class="fa-solid fa-code"></i> 代码编辑</span>
          <select id="lang-select" onchange="ProblemDetailPage.switchLang()" style="padding:4px 10px;border:1px solid #555;border-radius:6px;font-size:12px;background:#3a3a3a;color:#ccc;outline:none">
            <option value="c_cpp">C++17</option>
            <option value="java">Java</option>
            <option value="python">Python</option>
          </select>
        </div>
        <div id="ace-editor" style="flex:1;border-radius:0 0 10px 10px"></div>
      </div>

      <div style="padding:0 12px 12px;overflow-y:auto;max-height:240px">
        <h3 style="font-size:14px;font-weight:700;margin-bottom:8px">测试用例</h3>
        <div id="tc-cards" style="display:flex;gap:8px;flex-wrap:wrap;margin-bottom:10px">
          ${(p.sample_testcases||[]).map((tc,i) => `
            <div style="background:#fff;border-radius:10px;padding:12px;box-shadow:0 1px 3px rgba(0,0,0,.06);flex:1;min-width:160px;font-size:13px">
              <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:6px">
                <span style="font-weight:600;font-size:12px">用例 ${i+1}</span>
                <span style="font-size:11px;padding:1px 8px;border-radius:10px;background:#f0f0f0;color:#94a3b8" class="tc-status-el">待评测</span>
              </div>
              <div style="font-family:monospace;font-size:12px;color:#555">${this.esc(tc.input).substring(0,30)} &rarr; ${this.esc(tc.expected_output).substring(0,30)}</div>
            </div>
          `).join('')}
        </div>
        <div id="judge-summary" style="display:none;font-size:14px;font-weight:600;padding:8px 0"></div>
      </div>

      <div style="display:flex;align-items:center;gap:8px;padding:0 12px 14px">
        <button class="btn btn-primary" id="btn-run" style="padding:10px 24px;border-radius:10px;font-size:14px;font-weight:600" onclick="ProblemDetailPage.submit()"><i class="fa-solid fa-play"></i> 运行代码</button>
        <button class="btn btn-outline" style="padding:10px 20px;border-radius:10px" onclick="ProblemDetailPage.clearEditor()"><i class="fa-solid fa-eraser"></i> 清空</button>
      </div>
    </div>
  </div>
</div>`;

    this.initEditor();
  },

  initEditor() {
    this.state.editor = ace.edit('ace-editor');
    this.state.editor.setTheme('ace/theme/tomorrow_night_eighties');
    this.state.editor.setFontSize(14);
    this.state.editor.setOptions({ showPrintMargin: false, tabSize: 2, useSoftTabs: true });
    this.state.editor.session.setMode('ace/mode/c_cpp');
    this.state.editor.setValue('#include <iostream>\nusing namespace std;\n\nint main() {\n    \n    return 0;\n}', -1);
  },

  switchLang() {
    const lang = document.getElementById('lang-select').value;
    const codes = {
      c_cpp: '#include <iostream>\nusing namespace std;\n\nint main() {\n    \n    return 0;\n}',
      java: 'import java.util.*;\n\npublic class Main {\n    public static void main(String[] args) {\n        \n    }\n}',
      python: '# Write your solution here\n',
    };
    this.state.editor.session.setMode('ace/mode/' + lang);
    this.state.editor.setValue(codes[lang], -1);
  },

  async submit() {
    const code = this.state.editor.getValue();
    if (!code.trim()) { alert('代码不能为空'); return; }

    const btn = document.getElementById('btn-run');
    btn.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> 判题中...';
    btn.disabled = true;

    try {
      const resp = await API.submit(this.state.problem.id, code);
      Router.navigate('#/submissions/' + resp.data.id);
    } catch (e) {
      document.getElementById('judge-summary').style.display = '';
      document.getElementById('judge-summary').textContent = '提交失败: ' + e.message;
      document.getElementById('judge-summary').style.color = '#dc2626';
      btn.innerHTML = '<i class="fa-solid fa-play"></i> 运行代码';
      btn.disabled = false;
    }
  },

  clearEditor() { this.state.editor.setValue('', -1); },

  formatDesc(desc) {
    if (!desc) return '';
    let html = this.esc(desc);
    html = html.replace(/###?\s*(.+)/g, '<h3 style="font-size:15px;margin:12px 0 6px">$1</h3>');
    html = html.replace(/\n\n/g, '</p><p>');
    html = '<p>' + html + '</p>';
    return html;
  },

  esc(s) {
    if (!s) return '';
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
  },
};
