const API_BASE = '/api';

const API = {
  async request(method, path, body = null) {
    const headers = { 'Content-Type': 'application/json' };
    const token = localStorage.getItem('token');
    if (token) headers['Authorization'] = 'Bearer ' + token;

    const opts = { method, headers };
    if (body) opts.body = JSON.stringify(body);

    const res = await fetch(API_BASE + path, opts);
    const data = await res.json();
    if (!res.ok) throw new Error(data.message || 'Request failed');
    return data;
  },

  get(path) { return this.request('GET', path); },
  post(path, body) { return this.request('POST', path, body); },
  put(path, body) { return this.request('PUT', path, body); },
  del(path) { return this.request('DELETE', path); },

  // Auth
  login(username, password) { return this.post('/auth/login', { username, password }); },
  register(username, password) { return this.post('/auth/register', { username, password }); },

  // Problems
  getProblems(params = {}) {
    const qs = new URLSearchParams(params).toString();
    return this.get('/problems' + (qs ? '?' + qs : ''));
  },
  getProblem(id) { return this.get('/problems/' + id); },
  createProblem(data) { return this.post('/problems', data); },
  updateProblem(id, data) { return this.put('/problems/' + id, data); },
  deleteProblem(id) { return this.del('/problems/' + id); },

  // Testcases
  getTestcases(problemId) { return this.get('/problems/' + problemId + '/testcases'); },
  addTestcases(problemId, testcases) {
    return this.post('/problems/' + problemId + '/testcases', { testcases });
  },
  deleteTestcase(id) { return this.del('/testcases/' + id); },

  // Submissions
  submit(problemId, code) { return this.post('/submissions', { problem_id: problemId, code }); },
  getSubmissions(params = {}) {
    const qs = new URLSearchParams(params).toString();
    return this.get('/submissions' + (qs ? '?' + qs : ''));
  },
  getSubmission(id) { return this.get('/submissions/' + id); },

  // User
  getProfile() { return this.get('/users/me'); },
  getStats() { return this.get('/users/me/stats'); },
};
