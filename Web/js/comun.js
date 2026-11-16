// Configuración central de la API REST Crow.
const API_BASE = "http://localhost:18080";
const API_USUARIOS = API_BASE;
const API_CLIENTES = API_BASE;

function obtenerSesion() {
    try {
        const datos = sessionStorage.getItem("usuarioActual") ||
                      localStorage.getItem("usuarioActual");
        return datos ? JSON.parse(datos) : null;
    } catch {
        sessionStorage.removeItem("usuarioActual");
        localStorage.removeItem("usuarioActual");
        return null;
    }
}

function guardarSesion(usuario, recordar = false) {
    sessionStorage.removeItem("usuarioActual");
    localStorage.removeItem("usuarioActual");
    (recordar ? localStorage : sessionStorage).setItem("usuarioActual", JSON.stringify(usuario));
}

function cerrarSesion() {
    sessionStorage.removeItem("usuarioActual");
    localStorage.removeItem("usuarioActual");
    window.location.replace("login.html");
}

function requerirSesion() {
    const sesion = obtenerSesion();
    if (!sesion || sesion.activo !== true) {
        cerrarSesion();
        return null;
    }
    return sesion;
}

async function api(ruta, metodo = "GET", datos = null, base = API_BASE) {
    const opciones = { method: metodo, headers: { "Accept": "application/json" } };
    if (datos !== null && datos !== undefined) {
        opciones.headers["Content-Type"] = "application/json";
        opciones.body = JSON.stringify(datos);
    }

    let respuesta;
    try {
        respuesta = await fetch(base + ruta, opciones);
    } catch {
        throw new Error("No se pudo conectar con Crow. Verifica que la API esté ejecutándose en http://localhost:18080.");
    }

    const texto = await respuesta.text();
    let cuerpo = {};
    if (texto) {
        try { cuerpo = JSON.parse(texto); }
        catch { cuerpo = { error: texto }; }
    }
    if (!respuesta.ok) {
        throw new Error(cuerpo.error || cuerpo.mensaje || `Error HTTP ${respuesta.status}`);
    }
    return cuerpo;
}

function esc(valor) {
    const elemento = document.createElement("span");
    elemento.textContent = valor == null ? "" : String(valor);
    return elemento.innerHTML;
}

function inicialesUsuario(sesion) {
    const nombre = (sesion.clienteNombre || sesion.nombreUsuario || "Usuario").trim();
    const partes = nombre.split(/\s+/).filter(Boolean);
    return partes.length > 1
        ? (partes[0][0] + partes[1][0]).toUpperCase()
        : nombre.substring(0, 2).toUpperCase();
}

function mostrarAlerta(mensaje, tipo = "info") {
    const contenedor = document.getElementById("alerta");
    if (!contenedor) return;
    contenedor.innerHTML = `
        <div class="alert alert-${esc(tipo)} alert-dismissible fade show" role="alert">
            ${esc(mensaje)}
            <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Cerrar"></button>
        </div>`;
}

// Monta el mismo armazón visual del dashboard adjunto.
function montarLayout(paginaActiva, tituloBusqueda = "Buscar producto, venta o proveedor...") {
    const sesion = requerirSesion();
    if (!sesion) return null;

    const nombre = sesion.clienteNombre || sesion.nombreUsuario || "Usuario";
    const enlaces = [
        { id: "dashboard", texto: "Dashboard", href: "dashboard.html", icono: "bi-grid-1x2-fill", grupo: "Principal" },
        { id: "clientes", texto: "Clientes", href: "clientes.html", icono: "bi-person-lines-fill", grupo: "Principal" },
        { id: "inventario", texto: "Inventario", href: "#", icono: "bi-box-seam", grupo: "Principal", pendiente: true },
        { id: "ventas", texto: "Ventas", href: "#", icono: "bi-cart3", grupo: "Principal", pendiente: true },
        { id: "proveedores", texto: "Proveedores", href: "#", icono: "bi-truck", grupo: "Principal", pendiente: true },
        { id: "usuarios", texto: "Usuarios", href: "usuarios.html", icono: "bi-people", grupo: "Administración" },
        { id: "reportes", texto: "Reportes", href: "#", icono: "bi-file-earmark-bar-graph", grupo: "Administración", pendiente: true },
        { id: "configuracion", texto: "Configuración", href: "#", icono: "bi-gear", grupo: "Administración", pendiente: true }
    ];

    const sidebar = document.getElementById("sidebar");
    if (sidebar) {
        let grupoActual = "";
        const nav = enlaces.map(enlace => {
            let encabezado = "";
            if (enlace.grupo !== grupoActual) {
                grupoActual = enlace.grupo;
                encabezado = `<p class="nav-section">${esc(grupoActual)}</p>`;
            }
            const clase = `nav-link-custom ${enlace.id === paginaActiva ? "active" : ""}`;
            const href = enlace.pendiente ? "#" : enlace.href;
            const disabled = enlace.pendiente ? 'aria-disabled="true" title="Módulo pendiente de integración" onclick="event.preventDefault(); return false;"' : "";
            return `${encabezado}<a href="${href}" class="${clase}" ${disabled}><i class="bi ${enlace.icono}"></i> ${esc(enlace.texto)}</a>`;
        }).join("");

        sidebar.innerHTML = `
            <div class="sidebar-brand">
                <div class="brand-logo-sm"><i class="bi bi-capsule-pill"></i></div>
                <span class="brand-name">Farmacia</span>
            </div>
            <nav class="sidebar-nav" aria-label="Menú principal">${nav}</nav>
            <div class="sidebar-footer">
                <a href="#" class="nav-link-custom logout" id="btnCerrarSesion">
                    <i class="bi bi-box-arrow-left"></i> Cerrar sesión
                </a>
            </div>`;
    }

    const topbar = document.getElementById("topbar");
    if (topbar) {
        topbar.innerHTML = `
            <button class="btn btn-menu d-lg-none" id="btnMenu" aria-label="Abrir menú">
                <i class="bi bi-list"></i>
            </button>
            <div class="search-box d-none d-md-flex">
                <i class="bi bi-search"></i>
                <input type="text" placeholder="${esc(tituloBusqueda)}" disabled>
            </div>
            <div class="topbar-actions ms-auto">
                <div class="user-chip">
                    <div class="avatar" id="avatarUsuario">${esc(inicialesUsuario(sesion))}</div>
                    <div class="d-none d-sm-block">
                        <p class="user-name mb-0" id="nombreUsuarioSesion">${esc(nombre)}</p>
                        <p class="user-role mb-0" id="detalleUsuarioSesion">@${esc(sesion.nombreUsuario || "usuario")}</p>
                    </div>
                </div>
            </div>`;
    }

    const cerrar = document.getElementById("btnCerrarSesion");
    if (cerrar) cerrar.addEventListener("click", e => {
        e.preventDefault();
        cerrarSesion();
    });

    const btnMenu = document.getElementById("btnMenu");
    const overlay = document.getElementById("overlay");
    if (btnMenu && sidebar && overlay) {
        btnMenu.addEventListener("click", () => {
            sidebar.classList.add("open");
            overlay.classList.add("show");
        });
        overlay.addEventListener("click", () => {
            sidebar.classList.remove("open");
            overlay.classList.remove("show");
        });
    }

    return sesion;
}
