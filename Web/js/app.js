/* Login — usa las funciones de comun.js (api, guardarSesion, obtenerSesion).
   Debe cargarse DESPUÉS de comun.js. */
(function () {
  "use strict";

  /* ---- Ajusta estos valores a tu API Crow ---- */
  const RUTA_LOGIN = "/login";          // endpoint de inicio de sesión (POST)
  const DESTINO = "dashboard.html";     // página a la que se entra al iniciar sesión
  const campoUsuario = "nombreUsuario"; // nombres de los campos que espera la API
  const campoClave = "clave";

  const form = document.getElementById("formLogin");
  if (!form) return;

  if (typeof api !== "function" || typeof guardarSesion !== "function") {
    console.error("login: falta comun.js. Cárgalo antes de app.js.");
    return;
  }

  // Si ya hay una sesión válida, entra directo
  const previa = obtenerSesion();
  if (previa && previa.activo === true) { window.location.replace(DESTINO); return; }

  const usuario = document.getElementById("usuario");
  const clave = document.getElementById("clave");
  const recordar = form.elements["remember"];
  const toggle = document.getElementById("pw-toggle");
  const boton = document.getElementById("btnIngresar");
  const textoBoton = boton.textContent;

  // Mensaje general (credenciales incorrectas, API caída, etc.)
  const errorGeneral = document.createElement("p");
  errorGeneral.className = "error";
  errorGeneral.setAttribute("role", "alert");
  boton.parentNode.insertBefore(errorGeneral, boton);

  function setError(input, mensaje) {
    document.getElementById(input.getAttribute("aria-describedby")).textContent = mensaje;
    input.setAttribute("aria-invalid", mensaje ? "true" : "false");
    return !mensaje;
  }

  function validarUsuario() {
    const v = usuario.value.trim();
    if (!v) return setError(usuario, "Escribe tu nombre de usuario.");
    if (v.length < 3) return setError(usuario, "El nombre de usuario tiene al menos 3 caracteres.");
    if (/\s/.test(v)) return setError(usuario, "El nombre de usuario no puede llevar espacios.");
    return setError(usuario, "");
  }

  // Solo se exige que no esté vacía: la política de contraseñas la decide el servidor
  function validarClave() {
    return setError(clave, clave.value ? "" : "Escribe tu contraseña.");
  }

  usuario.addEventListener("blur", () => { if (usuario.value) validarUsuario(); });
  clave.addEventListener("blur", () => { if (clave.value) validarClave(); });
  usuario.addEventListener("input", () => {
    errorGeneral.textContent = "";
    if (usuario.getAttribute("aria-invalid") === "true") validarUsuario();
  });
  clave.addEventListener("input", () => {
    errorGeneral.textContent = "";
    if (clave.getAttribute("aria-invalid") === "true") validarClave();
  });

  toggle.setAttribute("aria-controls", "clave");
  toggle.addEventListener("click", () => {
    const mostrar = clave.type === "password";
    clave.type = mostrar ? "text" : "password";
    toggle.textContent = mostrar ? "Ocultar" : "Mostrar";
    toggle.setAttribute("aria-pressed", String(mostrar));
  });

  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    errorGeneral.textContent = "";

    const okUsuario = validarUsuario();
    const okClave = validarClave();
    if (!okUsuario) { usuario.focus(); return; }
    if (!okClave) { clave.focus(); return; }

    boton.disabled = true;
    boton.textContent = "Entrando…";

    try {
      const respuesta = await api(RUTA_LOGIN, "POST", {
        [campoUsuario]: usuario.value.trim(),
        [campoClave]: clave.value
      }, typeof API_USUARIOS !== "undefined" ? API_USUARIOS : undefined);

      // La API puede devolver el usuario directo o dentro de { usuario: {...} }
      const datos = respuesta.usuario || respuesta;
      if (datos.activo === false) throw new Error("Tu cuenta está inactiva. Contacta al administrador.");

      guardarSesion(Object.assign({}, datos, { activo: true }), recordar.checked);
      window.location.replace(DESTINO);
    } catch (err) {
      errorGeneral.textContent = err.message || "No se pudo iniciar sesión.";
      boton.disabled = false;
      boton.textContent = textoBoton;
      clave.value = "";
      clave.focus();
    }
  });
})();
