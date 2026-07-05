const AdminPage = {
  state: { problems: [], editing: null, page: 1 },

  async render(ctx) {
    if (!Auth.isAdmin()) {
      document.getElementById('main-content').innerHTML =
        '<div class="alert alert-error">需要管理员权限</div>';
      return;
    }

    document.getElementById('main-content').innerHTML =
      '<h1 class="page-title">题目管理</h1><div id="admin-content">加载中...</div>';
    await this.loadList();
  },

  async loadList() {
    try {
      const resp = await API.getProblems({ page: this.state.page, size: 50 });
      const problems = resp.problems || [];

      let html = `<button class="btn btn-primary mb-16" onclick="AdminPage.showCreate()">+ 新增题目</button>`;

      html += '<div class="card" style="padding:0"><table>';
      html += '<tr><th>ID</th><th>标题</th><th>难度</th><th>操作</th></tr>';
      for (const p of problems) {
        html += `<tr>
          <td>${p.id}</td>
          <td>${this.escape(p.title)}</td>
          <td><span class="badge badge-${p.difficulty}">${p.difficulty}</span></td>
          <td>
            <button class="btn" onclick="AdminPage.showEdit(${p.id})" style="padding:4px 12px;font-size:12px">编辑</button>
            <button class="btn btn-danger" onclick="AdminPage.deleteProblem(${p.id})" style="padding:4px 12px;font-size:12px">删除</button>
          </td>
        </tr>`;
      }
      html += '</table></div>';
      html += '<div id="admin-editor"></div>';

      document.getElementById('admin-content').innerHTML = html;
    } catch (e) {
      document.getElementById('admin-content').innerHTML =
        '<div class="alert alert-error">' + e.message + '</div>';
    }
  },

  async showCreate() {
    document.getElementById('admin-editor').innerHTML = `
      <div class="card mt-16">
        <h3>新增题目</h3>
        ${this.editorForm({})}
        <button class="btn btn-primary" onclick="AdminPage.submitCreate()">创建</button>
        <button class="btn" onclick="document.getElementById('admin-editor').innerHTML=''">取消</button>
      </div>`;
  },

  async showEdit(id) {
    try {
      const resp = await API.getProblem(id);
      const p = resp.data;

      // Load all testcases
      let testcases = [];
      try {
        const tcResp = await API.getTestcases(id);
        testcases = tcResp.testcases || [];
      } catch (e) {}

      document.getElementById('admin-editor').innerHTML = `
        <div class="card mt-16">
          <h3>编辑题目</h3>
          ${this.editorForm(p)}
          <button class="btn btn-primary" onclick="AdminPage.submitUpdate(${id})">保存</button>
          <button class="btn" onclick="document.getElementById('admin-editor').innerHTML=''">取消</button>
        </div>
        <div class="card mt-16">
          <h3>测试用例 (${testcases.length})</h3>
          <div id="testcase-list">
            ${testcases.map((tc, i) => `
              <div style="background:#f8f8f8;padding:8px;margin-bottom:8px;border-radius:4px">
                <strong>#${i + 1}</strong> ${tc.is_sample ? '(样例)' : '(隐藏)'}
                <button class="btn btn-danger" style="float:right;padding:2px 8px;font-size:11px"
                  onclick="AdminPage.deleteTestcase(${tc.id})">删除</button>
                <pre style="font-size:12px;margin-top:4px">输入: ${this.escape(tc.input)}</pre>
                <pre style="font-size:12px">输出: ${this.escape(tc.expected_output)}</pre>
              </div>
            `).join('')}
          </div>
          <h4 style="margin-top:12px">添加测试用例</h4>
          <div class="form-group">
            <label>输入</label>
            <textarea id="tc-input" rows="3"></textarea>
          </div>
          <div class="form-group">
            <label>预期输出</label>
            <textarea id="tc-expected" rows="3"></textarea>
          </div>
          <div class="form-group">
            <label><input type="checkbox" id="tc-is-sample"> 设为样例</label>
          </div>
          <button class="btn btn-success" onclick="AdminPage.addTestcase(${id})">添加用例</button>
        </div>`;
    } catch (e) {
      alert('加载失败: ' + e.message);
    }
  },

  editorForm(p) {
    return `
      <div class="form-group">
        <label>标题</label>
        <input type="text" id="prob-title" value="${this.escape(p.title || '')}">
      </div>
      <div class="form-group">
        <label>描述</label>
        <textarea id="prob-desc" rows="8">${this.escape(p.description || '')}</textarea>
      </div>
      <div class="flex-row">
        <div class="form-group" style="flex:1">
          <label>难度</label>
          <select id="prob-difficulty">
            <option value="easy" ${p.difficulty === 'easy' ? 'selected' : ''}>Easy</option>
            <option value="medium" ${p.difficulty === 'medium' ? 'selected' : ''}>Medium</option>
            <option value="hard" ${p.difficulty === 'hard' ? 'selected' : ''}>Hard</option>
          </select>
        </div>
        <div class="form-group" style="flex:1">
          <label>时间限制 (ms)</label>
          <input type="number" id="prob-time" value="${p.time_limit_ms || 1000}">
        </div>
        <div class="form-group" style="flex:1">
          <label>内存限制 (KB)</label>
          <input type="number" id="prob-memory" value="${p.memory_limit_kb || 262144}">
        </div>
      </div>`;
  },

  async submitCreate() {
    try {
      await API.createProblem({
        title: document.getElementById('prob-title').value,
        description: document.getElementById('prob-desc').value,
        difficulty: document.getElementById('prob-difficulty').value,
        time_limit_ms: parseInt(document.getElementById('prob-time').value),
        memory_limit_kb: parseInt(document.getElementById('prob-memory').value),
      });
      document.getElementById('admin-editor').innerHTML = '';
      await this.loadList();
    } catch (e) {
      alert(e.message);
    }
  },

  async submitUpdate(id) {
    try {
      await API.updateProblem(id, {
        title: document.getElementById('prob-title').value,
        description: document.getElementById('prob-desc').value,
        difficulty: document.getElementById('prob-difficulty').value,
        time_limit_ms: parseInt(document.getElementById('prob-time').value),
        memory_limit_kb: parseInt(document.getElementById('prob-memory').value),
      });
      alert('保存成功');
    } catch (e) {
      alert(e.message);
    }
  },

  async deleteProblem(id) {
    if (!confirm('确认删除此题及所有测试用例？')) return;
    try {
      await API.deleteProblem(id);
      await this.loadList();
    } catch (e) {
      alert(e.message);
    }
  },

  async addTestcase(problemId) {
    const input = document.getElementById('tc-input').value;
    const expected = document.getElementById('tc-expected').value;
    const isSample = document.getElementById('tc-is-sample').checked;

    if (!input || !expected) { alert('请填写输入和预期输出'); return; }

    try {
      await API.addTestcases(problemId, [{ input, expected_output: expected, is_sample: isSample }]);
      document.getElementById('admin-editor').innerHTML = '';
      await this.showEdit(problemId);
    } catch (e) {
      alert(e.message);
    }
  },

  async deleteTestcase(id) {
    if (!confirm('确认删除此测试用例？')) return;
    try {
      await API.deleteTestcase(id);
      alert('已删除');
      await this.loadList();
      document.getElementById('admin-editor').innerHTML = '';
    } catch (e) {
      alert(e.message);
    }
  },

  escape(str) {
    if (!str) return '';
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  },
};
