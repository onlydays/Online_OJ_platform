const AdminPage = {
  state: { testcases: [] },

  async render() {
    if (!Auth.isAdmin()) {
      document.getElementById('main-content').innerHTML =
        '<div class="alert alert-error" style="margin:20px">需要管理员权限</div>';
      return;
    }
    document.getElementById('main-content').innerHTML =
      '<div class="page-container"><div id="admin-content">加载中...</div></div>';
    await this.renderEditor();
  },

  async renderEditor() {
    const resp = await API.getProblems({ page: 1, size: 50 }).catch(() => ({ problems: [] }));
    const problems = resp.problems || [];

    document.getElementById('admin-content').innerHTML = `
<div class="admin-layout">
  <div class="admin-form">
    <div class="card">
      <h2 style="font-size:16px;margin-bottom:16px">新增题目</h2>
      <div class="form-group"><label>题目标题</label><input type="text" id="adm-title"></div>
      <div class="form-group"><label>难度</label><select id="adm-difficulty">
        <option value="easy">简单</option><option value="medium">中等</option><option value="hard">困难</option>
      </select></div>
      <div class="form-group"><label>题目描述 <span class="text-sm">(支持文本)</span></label>
        <textarea id="adm-desc" rows="6"></textarea></div>
      <div class="flex-row mb-16">
        <div class="form-group flex-1"><label>时间限制(ms)</label><input type="number" id="adm-time" value="1000"></div>
        <div class="form-group flex-1"><label>内存限制(KB)</label><input type="number" id="adm-memory" value="262144"></div>
      </div>

      <h3 style="font-size:14px;margin-bottom:8px">测试用例</h3>
      <div id="tc-container">
        ${this.state.testcases.map((tc,i) => `
          <div class="tc-row">
            <input type="text" class="tc-input" value="${this.esc(tc.input||'')}" placeholder="输入">
            <input type="text" class="tc-output" value="${this.esc(tc.expected||'')}" placeholder="期望输出">
            <label class="tc-sample-lbl" style="font-size:11px;white-space:nowrap"><input type="checkbox" class="tc-sample" ${tc.isSample?'checked':''}> 样例</label>
            <button class="btn btn-danger btn-sm" onclick="AdminPage.removeTc(${i})">×</button>
          </div>
        `).join('')}
      </div>
      <button class="btn btn-outline btn-sm mt-16" onclick="AdminPage.addTc()">+ 添加用例</button>
      <button class="btn btn-primary mt-16" style="width:100%;padding:10px" onclick="AdminPage.createProblem()">创建题目</button>
    </div>
  </div>

  <div class="admin-list">
    <div class="card" style="padding:0">
      <div style="padding:14px;border-bottom:1px solid #e5e5e5;font-weight:700;font-size:14px">已有题目 (${problems.length})</div>
      <table>
        <tr><th>#</th><th>标题</th><th>难度</th><th>操作</th></tr>
        ${problems.map(p => `<tr>
          <td style="width:50px;color:#8c8c8c">${p.id}</td>
          <td>${this.esc(p.title)}</td>
          <td><span class="badge badge-${p.difficulty}">${p.difficulty==='easy'?'简单':p.difficulty==='medium'?'中等':'困难'}</span></td>
          <td style="width:100px">
            <button class="btn btn-outline btn-sm" onclick="Router.navigate('#/problems/${p.id}')">查看</button>
            <button class="btn btn-danger btn-sm" onclick="AdminPage.deleteProblem(${p.id})">删除</button>
          </td>
        </tr>`).join('')}
      </table>
    </div>
  </div>
</div>`;

    if (!this.state.testcases.length) this.addTc();
  },

  addTc() {
    this.state.testcases.push({ input: '', expected: '', isSample: false });
    this.renderEditor();
  },

  removeTc(i) {
    this.state.testcases.splice(i, 1);
    this.renderEditor();
  },

  async createProblem() {
    const title = document.getElementById('adm-title').value.trim();
    const desc = document.getElementById('adm-desc').value.trim();
    if (!title || !desc) { alert('请填写标题和描述'); return; }

    try {
      const probResp = await API.createProblem({
        title,
        description: desc,
        difficulty: document.getElementById('adm-difficulty').value,
        time_limit_ms: parseInt(document.getElementById('adm-time').value),
        memory_limit_kb: parseInt(document.getElementById('adm-memory').value),
      });
      const pid = probResp.data.id;

      const inputs = document.querySelectorAll('.tc-input');
      const outputs = document.querySelectorAll('.tc-output');
      const samples = document.querySelectorAll('.tc-sample');
      const tcs = [];
      for (let i = 0; i < inputs.length; i++) {
        const inp = inputs[i].value.trim();
        const out = outputs[i].value.trim();
        if (inp && out) tcs.push({ input: inp, expected_output: out, is_sample: samples[i].checked });
      }
      if (tcs.length) await API.addTestcases(pid, tcs);

      alert('题目已创建');
      this.state.testcases = [];
      this.renderEditor();
    } catch (e) { alert('错误: ' + e.message); }
  },

  async deleteProblem(id) {
    if (!confirm('确认删除？')) return;
    try { await API.deleteProblem(id); this.renderEditor(); }
    catch (e) { alert(e.message); }
  },

  esc(s) {
    if (!s) return '';
    const d = document.createElement('div');
    d.textContent = s;
    return d.innerHTML;
  },
};
