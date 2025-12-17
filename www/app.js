const API_BASE = '';
let authToken = localStorage.getItem('authToken');
let currentUser = null;

const showLoading = () => document.getElementById('loading').classList.add('show');
const hideLoading = () => document.getElementById('loading').classList.remove('show');

function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;
    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}

async function apiCall(endpoint, options = {}) {
    showLoading();
    try {
        const headers = {
            'Content-Type': 'application/json',
            ...options.headers
        };
        
        if (authToken && !options.noAuth) {
            headers['Authorization'] = `Bearer ${authToken}`;
        }
        
        const response = await fetch(API_BASE + endpoint, {
            ...options,
            headers
        });
        
        const data = await response.json();
        hideLoading();
        
        if (!data.success) {
            showToast(data.message, 'error');
            if (data.message.includes('未登录') || data.message.includes('会话已过期')) {
                logout();
            }
            return null;
        }
        
        return data;
    } catch (error) {
        hideLoading();
        showToast('网络错误，请重试', 'error');
        console.error(error);
        return null;
    }
}

function switchPage(pageName) {
    document.querySelectorAll('.page').forEach(page => {
        page.classList.remove('active');
    });
    document.getElementById(pageName + 'Page').classList.add('active');
}

function switchView(viewName) {
    document.querySelectorAll('.view').forEach(view => {
        view.classList.remove('active');
    });
    document.getElementById(viewName + 'View').classList.add('active');
    
    document.querySelectorAll('.menu-item').forEach(item => {
        item.classList.remove('active');
    });
    document.querySelector(`[data-view="${viewName}"]`).classList.add('active');
}

document.querySelectorAll('.tab').forEach(tab => {
    tab.addEventListener('click', () => {
        const tabName = tab.dataset.tab;
        document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
        document.querySelectorAll('.auth-form').forEach(f => f.classList.remove('active'));
        tab.classList.add('active');
        document.getElementById(tabName + 'Form').classList.add('active');
    });
});

document.getElementById('loginForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const username = document.getElementById('loginUsername').value;
    const password = document.getElementById('loginPassword').value;
    
    const result = await apiCall('/api/login', {
        method: 'POST',
        body: JSON.stringify({ username, password }),
        noAuth: true
    });
    
    if (result && result.data) {
        authToken = result.data.token;
        localStorage.setItem('authToken', authToken);
        currentUser = result.data.user;
        showToast('登录成功！');
        loadMainPage();
    }
});

document.getElementById('registerForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const userData = {
        username: document.getElementById('regUsername').value,
        password: document.getElementById('regPassword').value,
        age: parseInt(document.getElementById('regAge').value),
        weight: parseFloat(document.getElementById('regWeight').value),
        height: parseFloat(document.getElementById('regHeight').value),
        gender: document.getElementById('regGender').value,
        activityLevel: document.getElementById('regActivity').value
    };
    
    const result = await apiCall('/api/register', {
        method: 'POST',
        body: JSON.stringify(userData),
        noAuth: true
    });
    
    if (result && result.data) {
        authToken = result.data.token;
        localStorage.setItem('authToken', authToken);
        currentUser = result.data.user;
        showToast('注册成功！');
        loadMainPage();
    }
});

document.getElementById('logoutBtn').addEventListener('click', logout);

function logout() {
    authToken = null;
    currentUser = null;
    localStorage.removeItem('authToken');
    switchPage('auth');
}

document.querySelectorAll('.menu-item').forEach(item => {
    item.addEventListener('click', () => {
        const viewName = item.dataset.view;
        switchView(viewName);
        loadViewData(viewName);
    });
});

document.querySelectorAll('.action-btn').forEach(btn => {
    btn.addEventListener('click', () => {
        const action = btn.dataset.action;
        switchView(action);
        loadViewData(action);
    });
});

async function loadMainPage() {
    switchPage('main');
    
    const result = await apiCall('/api/user/profile');
    if (result && result.data) {
        currentUser = result.data;
        document.getElementById('navUsername').textContent = currentUser.username;
        updateDashboard();
    }
}

function updateDashboard() {
    document.getElementById('statCalories').textContent = Math.round(currentUser.dailyCalorieGoal) + ' kcal';
    document.getElementById('statProtein').textContent = Math.round(currentUser.dailyProteinGoal) + 'g';
    document.getElementById('statCarbs').textContent = Math.round(currentUser.dailyCarbsGoal) + 'g';
    document.getElementById('statFat').textContent = Math.round(currentUser.dailyFatGoal) + 'g';
}

async function loadViewData(viewName) {
    switch (viewName) {
        case 'profile':
            loadProfile();
            break;
        case 'foods':
            loadFoods();
            break;
        case 'history':
            loadHistory();
            break;
        case 'recommend':
            setupRecommend();
            break;
    }
}

function loadProfile() {
    document.getElementById('profileAge').value = currentUser.age;
    document.getElementById('profileWeight').value = currentUser.weight;
    document.getElementById('profileHeight').value = currentUser.height;
    document.getElementById('profileGender').value = currentUser.gender;
    document.getElementById('profileActivity').value = currentUser.activityLevel;
}

document.getElementById('profileForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const profileData = {
        age: parseInt(document.getElementById('profileAge').value),
        weight: parseFloat(document.getElementById('profileWeight').value),
        height: parseFloat(document.getElementById('profileHeight').value),
        gender: document.getElementById('profileGender').value,
        activityLevel: document.getElementById('profileActivity').value
    };
    
    const result = await apiCall('/api/user/profile', {
        method: 'PUT',
        body: JSON.stringify(profileData)
    });
    
    if (result && result.data) {
        currentUser = result.data;
        showToast('个人信息更新成功！');
        updateDashboard();
    }
});

async function loadFoods() {
    const result = await apiCall('/api/foods');
    if (result && result.data) {
        displayFoods(result.data);
        setupFoodFilters(result.data);
    }
}

function displayFoods(foods) {
    const grid = document.getElementById('foodsGrid');
    grid.innerHTML = foods.map(food => `
        <div class="food-card">
            <div class="food-name">${food.name}</div>
            <div class="food-category">${food.category}</div>
            <div class="food-nutrition">
                <div class="food-nutrition-item">
                    <strong>${Math.round(food.calories)}</strong> kcal
                </div>
                <div class="food-nutrition-item">
                    <strong>${food.protein.toFixed(1)}g</strong> 蛋白质
                </div>
                <div class="food-nutrition-item">
                    <strong>${food.carbs.toFixed(1)}g</strong> 碳水
                </div>
                <div class="food-nutrition-item">
                    <strong>${food.fat.toFixed(1)}g</strong> 脂肪
                </div>
            </div>
            ${food.tags.length > 0 ? `
                <div class="food-tags" style="margin-top: 10px;">
                    ${food.tags.map(tag => `<span class="food-tag" style="font-size: 11px; padding: 3px 8px;">${tag}</span>`).join('')}
                </div>
            ` : ''}
        </div>
    `).join('');
}

function setupFoodFilters(foods) {
    const categories = [...new Set(foods.map(f => f.category))];
    const categoryFilter = document.getElementById('categoryFilter');
    categoryFilter.innerHTML = '<option value="">所有类别</option>' +
        categories.map(cat => `<option value="${cat}">${cat}</option>`).join('');
    
    const searchInput = document.getElementById('foodSearch');
    
    const filterFoods = () => {
        const searchTerm = searchInput.value.toLowerCase();
        const selectedCategory = categoryFilter.value;
        
        const filtered = foods.filter(food => {
            const matchSearch = food.name.toLowerCase().includes(searchTerm) ||
                               food.tags.some(tag => tag.toLowerCase().includes(searchTerm));
            const matchCategory = !selectedCategory || food.category === selectedCategory;
            return matchSearch && matchCategory;
        });
        
        displayFoods(filtered);
    };
    
    searchInput.addEventListener('input', filterFoods);
    categoryFilter.addEventListener('change', filterFoods);
}

async function loadHistory() {
    const result = await apiCall('/api/meals/history');
    if (result && result.data) {
        displayHistory(result.data);
    }
}

function displayHistory(meals) {
    const grid = document.getElementById('historyResults');
    
    if (meals.length === 0) {
        grid.innerHTML = '<div style="padding: 40px; text-align: center; color: #666;">暂无历史记录</div>';
        return;
    }
    
    const groupedByDate = meals.reduce((acc, meal) => {
        if (!acc[meal.date]) acc[meal.date] = [];
        acc[meal.date].push(meal);
        return acc;
    }, {});
    
    grid.innerHTML = Object.entries(groupedByDate)
        .sort((a, b) => b[0].localeCompare(a[0]))
        .map(([date, dateMeals]) => {
            const dayCalories = dateMeals.reduce((sum, m) => sum + m.totalCalories, 0);
            const dayProtein = dateMeals.reduce((sum, m) => sum + m.totalProtein, 0);
            const dayCarbs = dateMeals.reduce((sum, m) => sum + m.totalCarbs, 0);
            const dayFat = dateMeals.reduce((sum, m) => sum + m.totalFat, 0);
            
            return `
                <div style="margin-bottom: 30px;">
                    <div style="display: flex; align-items: center; justify-content: space-between; gap: 15px; margin-bottom: 15px;">
                        <h3 style="color: #333; margin-bottom: 0; font-size: 20px;">📅 ${date}</h3>
                        <button class="btn-danger btn-danger-small delete-day-btn" data-date="${date}">删除当日餐单</button>
                    </div>
                    <div style="display: grid; gap: 15px;">
                        ${dateMeals.map(meal => createMealCard(meal, true)).join('')}
                    </div>
                    <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 20px; border-radius: 15px; color: white; margin-top: 15px;">
                        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; text-align: center;">
                            <div>
                                <div style="font-size: 12px; opacity: 0.9;">总热量</div>
                                <div style="font-size: 24px; font-weight: 700;">${Math.round(dayCalories)}</div>
                                <div style="font-size: 11px; opacity: 0.8;">${Math.round(dayCalories / currentUser.dailyCalorieGoal * 100)}% 目标</div>
                            </div>
                            <div>
                                <div style="font-size: 12px; opacity: 0.9;">蛋白质</div>
                                <div style="font-size: 24px; font-weight: 700;">${Math.round(dayProtein)}g</div>
                                <div style="font-size: 11px; opacity: 0.8;">${Math.round(dayProtein / currentUser.dailyProteinGoal * 100)}% 目标</div>
                            </div>
                            <div>
                                <div style="font-size: 12px; opacity: 0.9;">碳水化合物</div>
                                <div style="font-size: 24px; font-weight: 700;">${Math.round(dayCarbs)}g</div>
                            </div>
                            <div>
                                <div style="font-size: 12px; opacity: 0.9;">脂肪</div>
                                <div style="font-size: 24px; font-weight: 700;">${Math.round(dayFat)}g</div>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        }).join('');

    grid.querySelectorAll('.delete-meal-btn').forEach(btn => {
        btn.addEventListener('click', async (e) => {
            e.preventDefault();
            e.stopPropagation();

            const mealId = btn.dataset.mealId;
            if (!mealId) return;

            if (!confirm('确定要删除这条餐单记录吗？')) return;

            const result = await apiCall(`/api/meals/${mealId}`, { method: 'DELETE' });
            if (result) {
                showToast(result.message || '删除成功！');
                loadHistory();
            }
        });
    });

    grid.querySelectorAll('.delete-day-btn').forEach(btn => {
        btn.addEventListener('click', async (e) => {
            e.preventDefault();
            e.stopPropagation();

            const date = btn.dataset.date;
            if (!date) return;

            if (!confirm(`确定要删除 ${date} 的全部餐单吗？`)) return;

            const result = await apiCall(`/api/meals/date/${encodeURIComponent(date)}`, { method: 'DELETE' });
            if (result) {
                showToast(result.message || '删除成功！');
                loadHistory();
            }
        });
    });
}

function createMealCard(meal, allowDelete = false) {
    const mealTypeNames = {
        'breakfast': '早餐 🌅',
        'lunch': '午餐 ☀️',
        'dinner': '晚餐 🌙'
    };

    return `
        <div class="meal-card">
            <div class="meal-header">
                <div class="meal-type">${mealTypeNames[meal.mealType] || meal.mealType}</div>
                ${allowDelete ? `<button class="btn-danger btn-danger-small delete-meal-btn" data-meal-id="${meal.id}">删除</button>` : ''}
            </div>
            <div class="meal-nutrition">
                <div class="nutrition-item">
                    <div class="nutrition-label">热量</div>
                    <div class="nutrition-value">${Math.round(meal.totalCalories)}</div>
                </div>
                <div class="nutrition-item">
                    <div class="nutrition-label">蛋白质</div>
                    <div class="nutrition-value">${Math.round(meal.totalProtein)}g</div>
                </div>
                <div class="nutrition-item">
                    <div class="nutrition-label">碳水</div>
                    <div class="nutrition-value">${Math.round(meal.totalCarbs)}g</div>
                </div>
                <div class="nutrition-item">
                    <div class="nutrition-label">脂肪</div>
                    <div class="nutrition-value">${Math.round(meal.totalFat)}g</div>
                </div>
            </div>
            <div class="meal-foods">
                <div class="meal-foods-title">包含食物：</div>
                <div class="food-tags">
                    ${meal.foods.map(food => `<span class="food-tag">${food.name}</span>`).join('')}
                </div>
            </div>
        </div>
    `;
}

function setupRecommend() {
    const today = new Date().toISOString().split('T')[0];
    document.getElementById('recommendDate').value = today;
}

document.getElementById('generateBtn').addEventListener('click', async () => {
    const date = document.getElementById('recommendDate').value;
    if (!date) {
        showToast('请选择日期', 'error');
        return;
    }
    
    const result = await apiCall('/api/meals/recommend', {
        method: 'POST',
        body: JSON.stringify({ date })
    });
    
    if (result && result.data) {
        displayRecommendation(result.data, date);
    }
});

function displayRecommendation(meals, date) {
    const grid = document.getElementById('recommendResults');
    
    const totalCalories = meals.reduce((sum, m) => sum + m.totalCalories, 0);
    const totalProtein = meals.reduce((sum, m) => sum + m.totalProtein, 0);
    const totalCarbs = meals.reduce((sum, m) => sum + m.totalCarbs, 0);
    const totalFat = meals.reduce((sum, m) => sum + m.totalFat, 0);
    
    grid.innerHTML = `
        <div style="background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%); padding: 30px; border-radius: 20px; color: white; margin-bottom: 25px; text-align: center;">
            <h3 style="font-size: 24px; margin-bottom: 10px;">✨ 为您精心推荐</h3>
            <p style="opacity: 0.9; margin-bottom: 20px;">${date} 的营养配餐方案</p>
            <button id="saveRecommendBtn" class="btn-primary" style="background: white; color: #43e97b; max-width: 220px; margin: 0 auto;">
                💾 保存/替换此推荐
            </button>
        </div>
        
        <div style="display: grid; gap: 15px; margin-bottom: 25px;">
            ${meals.map(meal => createMealCard(meal)).join('')}
        </div>
        
        <div style="background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 25px; border-radius: 15px; color: white;">
            <h4 style="margin-bottom: 15px; font-size: 18px;">📊 每日营养总计</h4>
            <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); gap: 15px; text-align: center;">
                <div>
                    <div style="font-size: 12px; opacity: 0.9;">总热量</div>
                    <div style="font-size: 28px; font-weight: 700;">${Math.round(totalCalories)}</div>
                    <div style="font-size: 11px; opacity: 0.8;">目标: ${Math.round(currentUser.dailyCalorieGoal)} (${Math.round(totalCalories / currentUser.dailyCalorieGoal * 100)}%)</div>
                </div>
                <div>
                    <div style="font-size: 12px; opacity: 0.9;">蛋白质</div>
                    <div style="font-size: 28px; font-weight: 700;">${Math.round(totalProtein)}g</div>
                    <div style="font-size: 11px; opacity: 0.8;">目标: ${Math.round(currentUser.dailyProteinGoal)}g (${Math.round(totalProtein / currentUser.dailyProteinGoal * 100)}%)</div>
                </div>
                <div>
                    <div style="font-size: 12px; opacity: 0.9;">碳水化合物</div>
                    <div style="font-size: 28px; font-weight: 700;">${Math.round(totalCarbs)}g</div>
                    <div style="font-size: 11px; opacity: 0.8;">目标: ${Math.round(currentUser.dailyCarbsGoal)}g</div>
                </div>
                <div>
                    <div style="font-size: 12px; opacity: 0.9;">脂肪</div>
                    <div style="font-size: 28px; font-weight: 700;">${Math.round(totalFat)}g</div>
                    <div style="font-size: 11px; opacity: 0.8;">目标: ${Math.round(currentUser.dailyFatGoal)}g</div>
                </div>
            </div>
        </div>
    `;
    
    document.getElementById('saveRecommendBtn').addEventListener('click', async () => {
        const result = await apiCall('/api/meals/save', {
            method: 'POST',
            body: JSON.stringify({ date })
        });
        
        if (result) {
            showToast(result.message || '推荐已保存到历史记录！');
        }
    });
}

if (authToken) {
    loadMainPage();
} else {
    switchPage('auth');
}
