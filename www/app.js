const API_BASE = '';
let authToken = localStorage.getItem('authToken');
let currentUser = null;
let availableTags = [];
let allFoods = [];

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

async function loadProfile() {
    document.getElementById('profileAge').value = currentUser.age;
    document.getElementById('profileWeight').value = currentUser.weight;
    document.getElementById('profileHeight').value = currentUser.height;
    document.getElementById('profileGender').value = currentUser.gender;
    document.getElementById('profileActivity').value = currentUser.activityLevel;
    
    // 确保加载食物数据
    if (allFoods.length === 0) {
        const result = await apiCall('/api/foods');
        if (result && result.data) {
            allFoods = result.data;
        }
    }
    
    // 显示标签选择UI
    extractAndDisplayAvailableTags();
    
    // 延迟显示标签按钮，确保数据已加载
    setTimeout(() => {
        displayTagSelectionUI();
    }, 100);
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
        showToast('✅ 个人信息更新成功！');
        updateDashboard();
        
        await savePreferences();
    }
});

function extractAndDisplayAvailableTags() {
    if (!allFoods || allFoods.length === 0) return;
    
    const tagsSet = new Set();
    for (const food of allFoods) {
        if (food.tags && Array.isArray(food.tags)) {
            for (const tag of food.tags) {
                tagsSet.add(tag);
            }
        }
    }
    availableTags = Array.from(tagsSet).sort();
    
    displayTagSelectionUI();
}

function displayTagSelectionUI() {
    if (!availableTags || availableTags.length === 0) {
        return;
    }
    
    displayTagButtons('preferredTagsContainer', 'preferredTags', currentUser.preferredTags || []);
    displayTagButtons('avoidedTagsContainer', 'avoidedTags', currentUser.avoidedTags || []);
    displayTagButtons('allergensContainer', 'allergens', currentUser.allergens || []);
}

function displayTagButtons(containerId, fieldId, selectedTags) {
    const container = document.getElementById(containerId);
    console.log(`显示 ${containerId} 标签，已选择:`, selectedTags);
    
    container.innerHTML = '';
    
    if (!availableTags || availableTags.length === 0) {
        container.innerHTML = '<span style="color: #999; font-size: 12px; padding: 10px;">暂无标签数据，请刷新页面重试</span>';
        return;
    }
    
    console.log(`为 ${containerId} 显示 ${availableTags.length} 个标签`);
    
    for (const tag of availableTags) {
        const isSelected = selectedTags.includes(tag);
        const button = document.createElement('button');
        button.type = 'button';
        button.className = `tag-button ${isSelected ? 'selected' : ''}`;
        button.textContent = tag;
        button.dataset.tag = tag; // 添加标签数据
        button.style.cssText = `
            padding: 8px 16px;
            border: 2px solid ${isSelected ? '#667eea' : '#ddd'};
            border-radius: 20px;
            background: ${isSelected ? '#667eea' : 'white'};
            color: ${isSelected ? 'white' : '#333'};
            cursor: pointer;
            transition: all 0.2s ease;
            font-size: 14px;
            font-weight: ${isSelected ? '600' : '400'};
            white-space: nowrap;
            box-shadow: ${isSelected ? '0 2px 8px rgba(102, 126, 234, 0.3)' : '0 1px 3px rgba(0,0,0,0.1)'};
            margin: 4px;
        `;
        
        button.addEventListener('mouseenter', () => {
            if (!button.classList.contains('selected')) {
                button.style.borderColor = '#667eea';
                button.style.transform = 'translateY(-1px)';
                button.style.boxShadow = '0 2px 5px rgba(0,0,0,0.15)';
            }
        });
        
        button.addEventListener('mouseleave', () => {
            if (!button.classList.contains('selected')) {
                button.style.borderColor = '#ddd';
                button.style.transform = 'translateY(0)';
                button.style.boxShadow = '0 1px 3px rgba(0,0,0,0.1)';
            }
        });
        
        button.addEventListener('click', (e) => {
            e.preventDefault();
            console.log(`点击标签: ${tag}`);
            button.classList.toggle('selected');
            if (button.classList.contains('selected')) {
                button.style.background = '#667eea';
                button.style.color = 'white';
                button.style.borderColor = '#667eea';
                button.style.fontWeight = '600';
                button.style.boxShadow = '0 2px 8px rgba(102, 126, 234, 0.3)';
            } else {
                button.style.background = 'white';
                button.style.color = '#333';
                button.style.borderColor = '#ddd';
                button.style.fontWeight = '400';
                button.style.boxShadow = '0 1px 3px rgba(0,0,0,0.1)';
            }
        });
        
        container.appendChild(button);
    }
}

async function savePreferences() {
    console.log('开始保存偏好设置...');
    
    const preferredButtons = document.querySelectorAll('#preferredTagsContainer .selected');
    const avoidedButtons = document.querySelectorAll('#avoidedTagsContainer .selected');
    const allergenButtons = document.querySelectorAll('#allergensContainer .selected');
    
    console.log('找到的已选择标签:', {
        preferred: preferredButtons.length,
        avoided: avoidedButtons.length,
        allergens: allergenButtons.length
    });
    
    const preferredTags = Array.from(preferredButtons).map(btn => btn.textContent);
    const avoidedTags = Array.from(avoidedButtons).map(btn => btn.textContent);
    const allergens = Array.from(allergenButtons).map(btn => btn.textContent);
    
    const preferencesData = {
        preferredTags: preferredTags,
        avoidedTags: avoidedTags,
        allergens: allergens
    };
    
    console.log('准备保存的数据:', preferencesData);
    
    const result = await apiCall('/api/user/preferences', {
        method: 'PUT',
        body: JSON.stringify(preferencesData)
    });
    
    if (result && result.data) {
        currentUser = result.data;
        const prefCount = preferredTags.length;
        const avoidCount = avoidedTags.length;
        const allergenCount = allergens.length;
        showToast(`✅ 口味偏好已保存！喜欢: ${prefCount}项，避免: ${avoidCount}项，过敏源: ${allergenCount}项`);
        console.log('偏好设置保存成功');
    } else {
        console.error('偏好设置保存失败:', result);
    }
}

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
                        <h3 style="color: #333; margin: 0; font-size: 20px;">📅 ${date}</h3>
                        <button onclick="deleteMealsByDate('${date}')" style="
                            background: #ff4757; color: white; border: none;
                            padding: 8px 12px; border-radius: 8px; cursor: pointer;
                            font-size: 13px; flex-shrink: 0;
                        " title="删除当天所有餐单">🗑️ 删除当天</button>
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
}

function createMealCard(meal, showDelete = false) {
    const mealTypeNames = {
        'breakfast': '早餐 🌅',
        'lunch': '午餐 ☀️',
        'dinner': '晚餐 🌙'
    };
    
    const deleteButton = showDelete ? `
        <button class="delete-meal-btn" onclick="deleteMeal(${meal.id}, '${meal.date}', '${meal.mealType}')" 
                style="
                    position: absolute; top: 10px; right: 10px;
                    background: #ff4757; color: white; border: none;
                    border-radius: 50%; width: 30px; height: 30px;
                    cursor: pointer; font-size: 14px; line-height: 1;
                    display: flex; align-items: center; justify-content: center;
                " title="删除此餐单">🗑️</button>
    ` : '';
    
    const cardStyle = showDelete ? 'position: relative;' : '';
    
    return `
        <div class="meal-card" style="${cardStyle}">
            ${deleteButton}
            <div class="meal-header">
                <div class="meal-type">${mealTypeNames[meal.mealType] || meal.mealType}</div>
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

const checkDateBtn = document.getElementById('checkDateBtn');
if (checkDateBtn) {
    checkDateBtn.addEventListener('click', async () => {
        const date = document.getElementById('recommendDate').value;
        if (!date) {
            showToast('请选择日期', 'error');
            return;
        }
        
        const result = await apiCall(`/api/meals/check-date?date=${date}`);
        if (result && result.data) {
            if (result.data.hasExisting) {
                showConfirmDialog('此日期已有保存的餐单，是否替换为新的推荐？', async () => {
                    await saveRecommendation(date, true);
                });
            } else {
                await saveRecommendation(date, false);
            }
        }
    });
}

async function saveRecommendation(date, replaceExisting) {
    const result = await apiCall('/api/meals/save', {
        method: 'POST',
        body: JSON.stringify({ date, replaceExisting: replaceExisting ? 1 : 0 })
    });
    
    if (result) {
        const message = replaceExisting ? '推荐已替换保存！' : '推荐已保存到历史记录！';
        showToast(message);
    }
}

function showConfirmDialog(message, onConfirm, onCancel = null) {
    // 创建确认对话框
    const overlay = document.createElement('div');
    overlay.style.cssText = `
        position: fixed; top: 0; left: 0; width: 100%; height: 100%;
        background: rgba(0,0,0,0.5); display: flex; align-items: center;
        justify-content: center; z-index: 10000;
    `;
    
    const dialog = document.createElement('div');
    dialog.style.cssText = `
        background: white; padding: 30px; border-radius: 15px;
        max-width: 400px; width: 90%; text-align: center;
        box-shadow: 0 10px 30px rgba(0,0,0,0.3);
    `;
    
    dialog.innerHTML = `
        <div style="font-size: 18px; margin-bottom: 20px; color: #333;">${message}</div>
        <div style="display: flex; gap: 15px; justify-content: center;">
            <button id="confirmBtn" style="
                background: #667eea; color: white; border: none; 
                padding: 10px 20px; border-radius: 8px; cursor: pointer;
                font-size: 14px;
            ">确认</button>
            <button id="cancelBtn" style="
                background: #ccc; color: #666; border: none;
                padding: 10px 20px; border-radius: 8px; cursor: pointer;
                font-size: 14px;
            ">取消</button>
        </div>
    `;
    
    overlay.appendChild(dialog);
    document.body.appendChild(overlay);
    
    document.getElementById('confirmBtn').onclick = () => {
        document.body.removeChild(overlay);
        if (onConfirm) onConfirm();
    };
    
    document.getElementById('cancelBtn').onclick = () => {
        document.body.removeChild(overlay);
        if (onCancel) onCancel();
    };
    
    // 点击背景关闭
    overlay.onclick = (e) => {
        if (e.target === overlay) {
            document.body.removeChild(overlay);
            if (onCancel) onCancel();
        }
    };
}

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
            <div style="display: flex; gap: 15px; justify-content: center; flex-wrap: wrap;">
                <button id="saveRecommendationBtn" class="btn-primary" style="background: white; color: #43e97b; max-width: 180px; margin: 0 auto;">
                    💾 保存餐单
                </button>
            </div>
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
    
    document.getElementById('saveRecommendationBtn').addEventListener('click', async () => {
        const result = await apiCall(`/api/meals/check-date?date=${encodeURIComponent(date)}`);
        if (result && result.data) {
            if (result.data.hasExisting) {
                showConfirmDialog('此日期已有保存的餐单，是否替换为新的推荐？', async () => {
                    await saveRecommendation(date, true);
                });
            } else {
                await saveRecommendation(date, false);
            }
        }
    });
}

async function deleteMeal(mealId, mealDate, mealType) {
    showConfirmDialog(
        `确定要删除 ${mealDate} 的${mealType === 'breakfast' ? '早餐' : mealType === 'lunch' ? '午餐' : '晚餐'}吗？`,
        async () => {
            const result = await apiCall(`/api/meals/${mealId}`, {
                method: 'DELETE'
            });
            
            if (result) {
                showToast('餐单删除成功！');
                loadHistory();
            }
        }
    );
}

async function deleteMealsByDate(mealDate) {
    showConfirmDialog(
        `确定要删除 ${mealDate} 当天的所有餐单吗？此操作不可恢复。`,
        async () => {
            const result = await apiCall(`/api/meals/by-date?date=${encodeURIComponent(mealDate)}`, {
                method: 'DELETE'
            });
            
            if (result) {
                showToast(result.message || '当天餐单已删除！');
                loadHistory();
            }
        }
    );
}

if (authToken) {
    loadMainPage();
} else {
    switchPage('auth');
}
