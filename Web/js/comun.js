// Configuración central de la API REST Crow.
const API_BASE = "http://localhost:18080";
const API_USUARIOS = API_BASE;
const API_ASISTENTES = API_BASE, API_EVENTOS = API_BASE, API_PONENTES = API_BASE,
      API_CHARLAS = API_BASE, API_INSCRIPCIONES = API_BASE, API_CERTIFICADOS = API_BASE;

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
    const buscador = document.getElementById("buscador");
    if (buscador) buscador.addEventListener("input", aplicarBusqueda);
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
    const nombre = (sesion.asistenteNombre || sesion.clienteNombre || sesion.nombreUsuario || "Usuario").trim();
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

// Monta sidebar y topbar de Eventia.
function montarLayout(paginaActiva, tituloBusqueda = "Buscar en la tabla...") {
    const sesion = requerirSesion();
    if (!sesion) return null;

    const nombre = sesion.asistenteNombre || sesion.clienteNombre || sesion.nombreUsuario || "Usuario";
    const enlaces = [
        { id: "dashboard", texto: "Dashboard", href: "dashboard.html", icono: "bi-grid-1x2-fill", grupo: "Principal" },
        { id: "eventos", texto: "Eventos", href: "eventos.html", icono: "bi-calendar-event", grupo: "Programa" },
        { id: "charlas", texto: "Charlas", href: "charlas.html", icono: "bi-mic", grupo: "Programa" },
        { id: "ponentes", texto: "Ponentes", href: "ponentes.html", icono: "bi-person-video3", grupo: "Programa" },
        { id: "asistentes", texto: "Asistentes", href: "asistentes.html", icono: "bi-people", grupo: "Participantes" },
        { id: "inscripciones", texto: "Inscripciones", href: "inscripciones.html", icono: "bi-ticket-perforated", grupo: "Participantes" },
        { id: "certificados", texto: "Certificados", href: "certificados.html", icono: "bi-patch-check", grupo: "Participantes" },
        { id: "usuarios", texto: "Usuarios", href: "usuarios.html", icono: "bi-shield-lock", grupo: "Administración" }
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
            const actual = enlace.id === paginaActiva ? ' aria-current="page"' : "";
            const href = enlace.pendiente ? "#" : enlace.href;
            const disabled = enlace.pendiente ? 'aria-disabled="true" title="Módulo pendiente de integración" onclick="event.preventDefault(); return false;"' : "";
            return `${encabezado}<a href="${href}" class="${clase}"${actual} ${disabled}><i class="bi ${enlace.icono}"></i> ${esc(enlace.texto)}</a>`;
        }).join("");

        sidebar.innerHTML = `
            <div class="sidebar-brand">
                <a href="dashboard.html" class="brand"><span class="brand-mark" aria-hidden="true">E</span>Eventia</a>
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
                <input type="search" id="buscador" placeholder="${esc(tituloBusqueda)}" aria-label="Buscar en la tabla">
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

    const buscador = document.getElementById("buscador");
    if (buscador) buscador.addEventListener("input", aplicarBusqueda);
    return sesion;
}

// ---------- Utilidades compartidas ----------
const norm = t => String(t ?? "").normalize("NFD").replace(/[\u0300-\u036f]/g, "").toLowerCase();
function aplicarBusqueda() {
    const q = norm((document.getElementById("buscador") || {}).value || "").trim();
    document.querySelectorAll(".table-custom tbody tr").forEach(tr => { tr.hidden = !!q && !norm(tr.textContent).includes(q); });
}
async function cargarLista(ruta) {
    const r = await api(ruta);
    if (!Array.isArray(r)) throw new Error(`La API no devolvió una lista válida en ${ruta}.`);
    return r;
}
function llenarSelect(id, items, valor, texto, vacio) {
    const el = document.getElementById(id), previo = el.value;
    el.innerHTML = `<option value="">${esc(vacio)}</option>` + items.map(i => `<option value="${esc(valor(i))}">${esc(texto(i))}</option>`).join("");
    el.value = previo;
}
const esVerdadero = v => v === true || v === 1 || v === "1" || v === "true";
const fechaISO = v => String(v ?? "").slice(0, 10);
function hoyISO() { const d = new Date(); return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, "0")}-${String(d.getDate()).padStart(2, "0")}`; }
function fechaLegible(v) {
    const [a, m, d] = fechaISO(v).split("-").map(Number);
    return a ? new Date(a, m - 1, d).toLocaleDateString("es", { day: "numeric", month: "short", year: "numeric" }) : "—";
}
function estadoEvento(e) {
    const h = hoyISO();
    if (h < fechaISO(e.fechaInicio)) return { texto: "Próximo", clase: "tag--muted", orden: 1 };
    if (h > fechaISO(e.fechaFin)) return { texto: "Finalizado", clase: "tag--dark", orden: 2 };
    return { texto: "En curso", clase: "tag--accent", orden: 0 };
}
const filaVacia = (cols, msg, clase = "text-muted") => `<tr><td colspan="${cols}" class="text-center ${clase} py-4">${esc(msg)}</td></tr>`;
const botonesFila = (editar, eliminar, id) => `<td class="text-nowrap">
    <button class="btn btn-sm btn-outline-primary me-1" onclick="${editar}(${Number(id)})"><i class="bi bi-pencil"></i> Editar</button>
    <button class="btn btn-sm btn-outline-danger" onclick="${eliminar}(${Number(id)})"><i class="bi bi-trash"></i> Eliminar</button></td>`;
