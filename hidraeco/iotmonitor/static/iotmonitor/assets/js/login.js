function togglePassword(fieldId) {
    const passwordField = document.getElementById(fieldId);
    const toggleBtn = passwordField.nextElementSibling;
    const toggleIcon = document.getElementById('toggle-icon-' + fieldId);

    if (passwordField.type === 'password') {
        passwordField.type = 'text';
        // Note: As URLs estáticas do Django não funcionam em arquivos JS separados
        // Você pode definir essas URLs como variáveis globais no HTML ou usar caminhos diretos
        toggleIcon.src = "/static/iotmonitor/assets/img/senha_visivel.png";
        toggleIcon.alt = 'Ocultar senha';
        toggleBtn.title = 'Ocultar senha';
    } else {
        passwordField.type = 'password';
        toggleIcon.src = "/static/iotmonitor/assets/img/senha_invisivel.png";
        toggleIcon.alt = 'Mostrar senha';
        toggleBtn.title = 'Mostrar senha';
    }
}

// Inicialização quando o DOM estiver carregado
document.addEventListener('DOMContentLoaded', function() {
    initializeFormValidation();
    initializeFormSubmission();
    setupPasswordStrengthIndicator();
});

function initializeFormValidation() {
    const usernameField = document.getElementById('username');
    const emailField = document.getElementById('email') || document.getElementById('email2');
    const passwordField = document.getElementById('password') || document.getElementById('password2');
    const passwordConfirmField = document.getElementById('password_confirm');

    // Validação de nome de usuário
    if (usernameField) {
        let usernameTimeout;
        usernameField.addEventListener('input', function() {
            clearTimeout(usernameTimeout);
            usernameTimeout = setTimeout(() => validateUsername(this), 500);
        });
    }

    // Validação de email
    if (emailField) {
        let emailTimeout;
        emailField.addEventListener('input', function() {
            clearTimeout(emailTimeout);
            emailTimeout = setTimeout(() => validateEmail(this), 500);
        });
    }

    // Validação de senha
    if (passwordField) {
        passwordField.addEventListener('input', function() {
            validatePassword(this);
            if (passwordConfirmField) {
                validatePasswordConfirm();
            }
        });
    }

    // Validação de confirmação de senha
    if (passwordConfirmField) {
        passwordConfirmField.addEventListener('input', validatePasswordConfirm);
    }
}

function validateUsername(field) {
    const username = field.value.trim();
    const errorDiv = document.getElementById('username-error');
    
    if (username.length === 0) {
        clearFieldValidation(field, errorDiv);
        return;
    }
    
    if (username.length < 3) {
        showFieldError(field, errorDiv, 'Nome de usuário deve ter pelo menos 3 caracteres.');
        return;
    }
    
    if (!/^[a-zA-Z0-9_]+$/.test(username)) {
        showFieldError(field, errorDiv, 'Nome de usuário pode conter apenas letras, números e underscore.');
        return;
    }
    
    // Verificar disponibilidade via AJAX
    fetch('/check-username/', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ username: username })
    })
    .then(response => response.json())
    .then(data => {
        if (data.error) {
            showFieldError(field, errorDiv, 'Erro ao verificar disponibilidade.');
        } else if (data.exists) {
            showFieldError(field, errorDiv, 'Este nome de usuário já existe.');
        } else if (!data.valid) {
            showFieldError(field, errorDiv, 'Nome de usuário inválido.');
        } else {
            showFieldSuccess(field, errorDiv);
        }
    })
    .catch(error => {
        console.error('Erro ao verificar username:', error);
        showFieldError(field, errorDiv, 'Erro ao verificar disponibilidade.');
    });
}

function validateEmail(field) {
    const email = field.value.trim();
    const errorDiv = document.getElementById('email-error');
    
    if (email.length === 0) {
        clearFieldValidation(field, errorDiv);
        return;
    }
    
    if (!isValidEmail(email)) {
        showFieldError(field, errorDiv, 'Digite um email válido.');
        return;
    }
    
    // Verificar se email já existe via AJAX (apenas para cadastro)
    if (document.getElementById('cadastroForm')) {
        fetch('/check-email/', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ email: email })
        })
        .then(response => response.json())
        .then(data => {
            if (data.error) {
                showFieldError(field, errorDiv, 'Erro ao verificar email.');
            } else if (data.exists) {
                showFieldError(field, errorDiv, 'Este email já está cadastrado.');
            } else if (!data.valid) {
                showFieldError(field, errorDiv, 'Email inválido.');
            } else {
                showFieldSuccess(field, errorDiv);
            }
        })
        .catch(error => {
            console.error('Erro ao verificar email:', error);
            showFieldError(field, errorDiv, 'Erro ao verificar email.');
        });
    } else {
        showFieldSuccess(field, errorDiv);
    }
}

function validatePassword(field) {
    const password = field.value;
    const errorDiv = document.getElementById('password-error');
    
    if (password.length === 0) {
        clearFieldValidation(field, errorDiv);
        updatePasswordStrength(0);
        return;
    }
    
    if (password.length < 8) {
        showFieldError(field, errorDiv, 'A senha deve ter pelo menos 8 caracteres.');
        updatePasswordStrength(1);
        return;
    }
    
    // Calcular força da senha
    const strength = calculatePasswordStrength(password);
    updatePasswordStrength(strength);
    
    if (strength >= 3) {
        showFieldSuccess(field, errorDiv);
    } else {
        showFieldError(field, errorDiv, 'Senha muito fraca. Use letras maiúsculas, números e símbolos.');
    }
}

function validatePasswordConfirm() {
    const passwordField = document.getElementById('password');
    const passwordConfirmField = document.getElementById('password_confirm');
    const errorDiv = document.getElementById('password-confirm-error');
    
    if (!passwordField || !passwordConfirmField) return;
    
    const password = passwordField.value;
    const passwordConfirm = passwordConfirmField.value;
    
    if (passwordConfirm.length === 0) {
        clearFieldValidation(passwordConfirmField, errorDiv);
        return;
    }
    
    if (password !== passwordConfirm) {
        showFieldError(passwordConfirmField, errorDiv, 'As senhas não coincidem.');
    } else {
        showFieldSuccess(passwordConfirmField, errorDiv);
    }
}

function calculatePasswordStrength(password) {
    let strength = 0;
    
    // Comprimento
    if (password.length >= 8) strength++;
    if (password.length >= 12) strength++;
    
    // Caracteres
    if (/[a-z]/.test(password)) strength++;
    if (/[A-Z]/.test(password)) strength++;
    if (/[0-9]/.test(password)) strength++;
    if (/[^A-Za-z0-9]/.test(password)) strength++;
    
    return Math.min(strength, 5);
}

function updatePasswordStrength(strength) {
    const strengthIndicator = document.getElementById('password-strength');
    if (!strengthIndicator) return;
    
    const colors = ['#ff4757', '#ff6348', '#ffa502', '#2ed573', '#1e90ff'];
    const texts = ['Muito fraca', 'Fraca', 'Regular', 'Boa', 'Excelente'];
    
    strengthIndicator.style.width = (strength * 20) + '%';
    strengthIndicator.style.backgroundColor = colors[strength - 1] || '#e0e0e0';
    
    const strengthText = document.getElementById('password-strength-text');
    if (strengthText) {
        strengthText.textContent = strength > 0 ? texts[strength - 1] : '';
    }
}

function setupPasswordStrengthIndicator() {
    const passwordField = document.getElementById('password');
    if (passwordField && !document.getElementById('password-strength')) {
        const strengthHTML = `
            <div class="password-strength-container" style="margin-top: 5px;">
                <div class="password-strength-bar" style="width: 100%; height: 4px; background-color: #e0e0e0; border-radius: 2px;">
                    <div id="password-strength" style="height: 100%; border-radius: 2px; transition: all 0.3s ease; width: 0%;"></div>
                </div>
                <small id="password-strength-text" style="font-size: 11px; color: #666;"></small>
            </div>
        `;
        passwordField.parentNode.insertAdjacentHTML('afterend', strengthHTML);
    }
}

function initializeFormSubmission() {
    const forms = document.querySelectorAll('#loginForm, #cadastroForm');
    
    forms.forEach(form => {
        form.addEventListener('submit', function(e) {
            const submitBtn = form.querySelector('.login_btn');
            
            // Adicionar loading ao botão
            if (submitBtn) {
                submitBtn.disabled = true;
                submitBtn.classList.add('loading');
                submitBtn.textContent = 'Processando...';
            }
            
            // Verificar se há erros de validação
            const invalidFields = form.querySelectorAll('.is-invalid');
            if (invalidFields.length > 0) {
                e.preventDefault();
                if (submitBtn) {
                    submitBtn.disabled = false;
                    submitBtn.classList.remove('loading');
                    submitBtn.textContent = form.id === 'loginForm' ? 'Entrar' : 'Cadastrar';
                }
                showAlert('Por favor, corrija os erros antes de continuar.', 'error');
                return;
            }
        });
    });
}

// Funções auxiliares
function showFieldError(field, errorDiv, message) {
    field.classList.add('is-invalid');
    field.classList.remove('is-valid');
    if (errorDiv) {
        errorDiv.textContent = message;
        errorDiv.style.display = 'block';
    }
}

function showFieldSuccess(field, errorDiv) {
    field.classList.remove('is-invalid');
    field.classList.add('is-valid');
    if (errorDiv) {
        errorDiv.textContent = '';
        errorDiv.style.display = 'none';
    }
}

function clearFieldValidation(field, errorDiv) {
    field.classList.remove('is-invalid', 'is-valid');
    if (errorDiv) {
        errorDiv.textContent = '';
        errorDiv.style.display = 'none';
    }
}

function isValidEmail(email) {
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return emailRegex.test(email);
}

function showAlert(message, type = 'info') {
    const alertHTML = `
        <div class="alert alert-${type} alert-dismissible fade show" role="alert">
            ${message}
            <button type="button" class="close" data-dismiss="alert">
                <span>&times;</span>
            </button>
        </div>
    `;
    
    const container = document.querySelector('.login_wrapper');
    if (container) {
        container.insertAdjacentHTML('afterbegin', alertHTML);
        
        // Auto-remove após 5 segundos
        setTimeout(() => {
            const alert = container.querySelector('.alert');
            if (alert) {
                alert.classList.remove('show');
                setTimeout(() => alert.remove(), 150);
            }
        }, 5000);
    }
}