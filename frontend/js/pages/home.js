const HomePage = {
  render() {
    const main = document.getElementById('main-content');
    main.innerHTML = `
<div class="hero">
  <div class="hero-badge">在线C++编程训练平台</div>
  <h1>提升编程技能</h1>
  <p>从这里开始，丰富的编程题目，实时的代码执行，详细的执行结果反馈</p>
  <div class="hero-btns">
    <button class="btn btn-primary" onclick="Router.navigate('#/problems')">立即开始</button>
    <button class="btn btn-light" onclick="Router.navigate('#/problems')">浏览题目</button>
  </div>
</div>

<div class="features">
  <div class="feature-card">
    <div class="ficon">📚</div>
    <div class="fval">100+</div>
    <div class="flbl">在线题目</div>
  </div>
  <div class="feature-card">
    <div class="ficon">💻</div>
    <div class="fval">C++/Java/Python</div>
    <div class="flbl">支持语言</div>
  </div>
  <div class="feature-card">
    <div class="ficon">⚡</div>
    <div class="fval">< 5s</div>
    <div class="flbl">响应时间</div>
  </div>
</div>

<div class="steps">
  <h2>简单四步，快速上手</h2>
  <div class="steps-grid">
    <div class="step-card">
      <div class="snum">1</div>
      <div class="stitle">注册账号</div>
      <div class="sdesc">快速创建个人账户</div>
    </div>
    <div class="step-card">
      <div class="snum">2</div>
      <div class="stitle">选择题目</div>
      <div class="sdesc">浏览题库选择练习</div>
    </div>
    <div class="step-card">
      <div class="snum">3</div>
      <div class="stitle">编写代码</div>
      <div class="sdesc">在线编辑器中编写</div>
    </div>
    <div class="step-card">
      <div class="snum">4</div>
      <div class="stitle">提交评测</div>
      <div class="sdesc">获取即时判题结果</div>
    </div>
  </div>
  <div class="steps-footer">无需复杂配置，注册后即可开始练习编程</div>
</div>
    `;
  },
};
