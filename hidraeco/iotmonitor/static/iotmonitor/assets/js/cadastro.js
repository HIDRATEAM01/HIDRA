function togglePassword(fieldId) {
    const passwordField = document.getElementById(fieldId);
    const toggleBtn = passwordField.nextElementSibling;
    const toggleIcon = document.getElementById('toggle-icon-' + fieldId);

    if (passwordField.type === 'password') {
        passwordField.type = 'text';
        // Nota: Estas imagens precisam estar no diretório static/iotmonitor/
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