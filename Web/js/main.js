/* ==========================================================================
   EVENTIA — JavaScript principal (sin dependencias)
   --------------------------------------------------------------------------
   Módulos: cabecera, menú móvil, scroll suave, revelado, contadores,
   gráfico, pestañas de roles, modal de acceso, validación, lazy loading.
   ========================================================================== */
(function () {
  "use strict";

  document.documentElement.classList.remove("no-js");

  var $ = function (sel, ctx) { return (ctx || document).querySelector(sel); };
  var $$ = function (sel, ctx) { return Array.prototype.slice.call((ctx || document).querySelectorAll(sel)); };
  var reduceMotion = window.matchMedia && window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  var hasIO = "IntersectionObserver" in window;

  /* ---------- 1. Cabecera con sombra al hacer scroll ---------- */
  var header = $(".site-header");
  var onScroll = function () {
    if (header) header.classList.toggle("is-scrolled", window.scrollY > 12);
  };
  window.addEventListener("scroll", onScroll, { passive: true });
  onScroll();

  /* ---------- 2. Menú móvil ---------- */
  var burger = $("#burger");
  var mobileMenu = $("#mobile-menu");
  function setMenu(open) {
    if (!burger || !mobileMenu) return;
    burger.setAttribute("aria-expanded", String(open));
    burger.setAttribute("aria-label", open ? "Cerrar menú" : "Abrir menú");
    mobileMenu.classList.toggle("is-open", open);
    mobileMenu.setAttribute("aria-hidden", String(!open));
    document.body.classList.toggle("no-scroll", open);
  }
  if (burger) {
    burger.addEventListener("click", function () {
      setMenu(burger.getAttribute("aria-expanded") !== "true");
    });
    $$("a", mobileMenu).forEach(function (a) { a.addEventListener("click", function () { setMenu(false); }); });
    window.addEventListener("resize", function () { if (window.innerWidth >= 1024) setMenu(false); });
  }

  /* ---------- 3. Scroll suave con compensación de cabecera ---------- */
  $$('a[href^="#"]').forEach(function (link) {
    link.addEventListener("click", function (e) {
      var id = link.getAttribute("href");
      if (id.length < 2) return;
      var target = document.getElementById(id.slice(1));
      if (!target) return;
      e.preventDefault();
      var top = target.getBoundingClientRect().top + window.scrollY - (header ? header.offsetHeight : 0) - 8;
      window.scrollTo({ top: top, behavior: reduceMotion ? "auto" : "smooth" });
      history.pushState(null, "", id);
      target.setAttribute("tabindex", "-1");
      target.focus({ preventScroll: true });
    });
  });

  /* ---------- 4. Enlace activo según la sección visible ---------- */
  var navLinks = $$(".nav-link[href^='#']");
  if (hasIO && navLinks.length) {
    var spy = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (!entry.isIntersecting) return;
        navLinks.forEach(function (l) {
          var active = l.getAttribute("href") === "#" + entry.target.id;
          l.classList.toggle("is-active", active);
          if (active) l.setAttribute("aria-current", "true"); else l.removeAttribute("aria-current");
        });
      });
    }, { rootMargin: "-45% 0px -50% 0px" });
    navLinks.forEach(function (l) {
      var s = document.getElementById(l.getAttribute("href").slice(1));
      if (s) spy.observe(s);
    });
  }

  /* ---------- 5. Revelado al hacer scroll ---------- */
  var reveals = $$(".reveal");
  if (hasIO && !reduceMotion) {
    var revealIO = new IntersectionObserver(function (entries, obs) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) { entry.target.classList.add("is-visible"); obs.unobserve(entry.target); }
      });
    }, { threshold: 0.12, rootMargin: "0px 0px -40px 0px" });
    reveals.forEach(function (el) { revealIO.observe(el); });
  } else {
    reveals.forEach(function (el) { el.classList.add("is-visible"); });
  }

  /* ---------- 6. Contadores animados ---------- */
  function animateCount(el) {
    var end = parseFloat(el.getAttribute("data-count"));
    var decimals = (el.getAttribute("data-count").split(".")[1] || "").length;
    var suffix = el.getAttribute("data-suffix") || "";
    var fmt = function (n) { return n.toLocaleString("es-ES", { minimumFractionDigits: decimals, maximumFractionDigits: decimals }) + suffix; };
    if (reduceMotion) { el.textContent = fmt(end); return; }
    var start = null, dur = 1600;
    function step(ts) {
      if (!start) start = ts;
      var p = Math.min((ts - start) / dur, 1);
      el.textContent = fmt(end * (1 - Math.pow(1 - p, 3)));
      if (p < 1) requestAnimationFrame(step);
    }
    requestAnimationFrame(step);
  }
  var counters = $$("[data-count]");
  if (hasIO) {
    var countIO = new IntersectionObserver(function (entries, obs) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) { animateCount(entry.target); obs.unobserve(entry.target); }
      });
    }, { threshold: 0.6 });
    counters.forEach(function (c) { countIO.observe(c); });
  } else { counters.forEach(animateCount); }

  /* ---------- 7. Gráfico del panel (se anima al entrar en pantalla) ---------- */
  $$(".chart-bars").forEach(function (chart) {
    var bars = $$(".bar", chart);
    var grow = function () { bars.forEach(function (b) { b.style.height = b.getAttribute("data-h") + "%"; }); };
    if (hasIO) {
      var io = new IntersectionObserver(function (entries) {
        if (entries[0].isIntersecting) { grow(); io.disconnect(); }
      }, { threshold: 0.3 });
      io.observe(chart);
    } else { grow(); }
  });

  /* ---------- 8. Pestañas de roles (patrón ARIA tabs) ---------- */
  $$("[role='tablist']").forEach(function (list) {
    var tabs = $$("[role='tab']", list);
    function select(tab) {
      tabs.forEach(function (t) {
        var on = t === tab;
        t.setAttribute("aria-selected", String(on));
        t.tabIndex = on ? 0 : -1;
        var panel = document.getElementById(t.getAttribute("aria-controls"));
        if (panel) panel.hidden = !on;
      });
    }
    tabs.forEach(function (tab, i) {
      tab.addEventListener("click", function () { select(tab); });
      tab.addEventListener("keydown", function (e) {
        var next = null;
        if (e.key === "ArrowRight") next = tabs[(i + 1) % tabs.length];
        if (e.key === "ArrowLeft") next = tabs[(i - 1 + tabs.length) % tabs.length];
        if (e.key === "Home") next = tabs[0];
        if (e.key === "End") next = tabs[tabs.length - 1];
        if (next) { e.preventDefault(); select(next); next.focus(); }
      });
    });
  });

  /* ---------- 9. Toast ---------- */
  var toastEl = $("#toast");
  var toastTimer;
  function toast(msg) {
    if (!toastEl) return;
    toastEl.textContent = msg;
    toastEl.classList.add("is-visible");
    clearTimeout(toastTimer);
    toastTimer = setTimeout(function () { toastEl.classList.remove("is-visible"); }, 4200);
  }

  /* ---------- 10. Modal de acceso / registro ---------- */
  var modal = $("#auth-modal");
  var lastFocus = null;
  var FOCUSABLE = 'a[href], button:not([disabled]), input:not([disabled]), select:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

  function setAuthMode(mode) {
    if (!modal) return;
    var isRegister = mode === "register";
    $("#auth-title").textContent = isRegister ? "Crea tu cuenta" : "Bienvenido de nuevo";
    $("#auth-sub").textContent = isRegister ? "Empieza gratis. Sin tarjeta de crédito." : "Accede a tu panel de eventos.";
    $$("[data-register-only]", modal).forEach(function (el) {
      el.hidden = !isRegister;
      $$("input", el).forEach(function (i) { i.disabled = !isRegister; });
    });
    $("#auth-submit").textContent = isRegister ? "Crear cuenta" : "Iniciar sesión";
    $("#auth-switch-text").textContent = isRegister ? "¿Ya tienes cuenta?" : "¿Aún no tienes cuenta?";
    var sw = $("#auth-switch");
    sw.textContent = isRegister ? "Inicia sesión" : "Regístrate";
    sw.setAttribute("data-mode", isRegister ? "login" : "register");
    modal.setAttribute("data-mode", mode);
  }

  function openModal(mode, role) {
    if (!modal) return;
    lastFocus = document.activeElement;
    setAuthMode(mode || "login");
    if (role) {
      var r = $('input[name="role"][value="' + role + '"]', modal);
      if (r) r.checked = true;
    }
    modal.classList.add("is-open");
    modal.setAttribute("aria-hidden", "false");
    document.body.classList.add("no-scroll");
    setTimeout(function () { var f = $("input:not([disabled]):not([type=radio])", modal); if (f) f.focus(); }, 60);
  }
  function closeModal() {
    if (!modal) return;
    modal.classList.remove("is-open");
    modal.setAttribute("aria-hidden", "true");
    document.body.classList.remove("no-scroll");
    if (lastFocus) lastFocus.focus();
  }

  $$("[data-open-modal]").forEach(function (btn) {
    btn.addEventListener("click", function (e) {
      e.preventDefault();
      setMenu(false);
      openModal(btn.getAttribute("data-open-modal"), btn.getAttribute("data-role"));
    });
  });
  if (modal) {
    $$("[data-close-modal]", modal).forEach(function (b) { b.addEventListener("click", closeModal); });
    modal.addEventListener("click", function (e) { if (e.target === modal) closeModal(); });
    $("#auth-switch").addEventListener("click", function () { setAuthMode(this.getAttribute("data-mode")); });
    document.addEventListener("keydown", function (e) {
      if (!modal.classList.contains("is-open")) return;
      if (e.key === "Escape") { closeModal(); return; }
      if (e.key === "Tab") { // Trampa de foco
        var items = $$(FOCUSABLE, modal).filter(function (el) { return el.offsetParent !== null; });
        if (!items.length) return;
        var first = items[0], last = items[items.length - 1];
        if (e.shiftKey && document.activeElement === first) { e.preventDefault(); last.focus(); }
        else if (!e.shiftKey && document.activeElement === last) { e.preventDefault(); first.focus(); }
      }
    });
  }

  /* ---------- 11. Validación de formularios ---------- */
  var MESSAGES = {
    valueMissing: "Este campo es obligatorio.",
    typeMismatch: "Introduce un correo electrónico válido.",
    tooShort: function (el) { return "Debe tener al menos " + el.getAttribute("minlength") + " caracteres."; },
    patternMismatch: function (el) { return el.getAttribute("data-pattern-msg") || "El formato no es válido."; }
  };
  function validateField(el) {
    if (el.disabled || el.type === "radio") return true;
    var errEl = document.getElementById(el.getAttribute("aria-describedby"));
    var msg = "";
    var v = el.validity;
    if (el.type === "checkbox" && el.required && !el.checked) msg = "Debes aceptar para continuar.";
    else if (v.valueMissing) msg = MESSAGES.valueMissing;
    else if (v.typeMismatch) msg = MESSAGES.typeMismatch;
    else if (v.tooShort) msg = MESSAGES.tooShort(el);
    else if (v.patternMismatch) msg = MESSAGES.patternMismatch(el);
    el.setAttribute("aria-invalid", msg ? "true" : "false");
    if (errEl) errEl.textContent = msg;
    return !msg;
  }

  $$("form[data-validate]").forEach(function (form) {
    form.setAttribute("novalidate", "");
    var fields = $$("input, select, textarea", form);
    fields.forEach(function (f) {
      f.addEventListener("blur", function () { if (f.value || f.getAttribute("aria-invalid")) validateField(f); });
      f.addEventListener("input", function () { if (f.getAttribute("aria-invalid") === "true") validateField(f); });
    });
    form.addEventListener("submit", function (e) {
      e.preventDefault();
      var firstInvalid = null;
      fields.forEach(function (f) { if (!validateField(f) && !firstInvalid) firstInvalid = f; });
      if (firstInvalid) { firstInvalid.focus(); return; }

      // Honeypot anti-spam
      var hp = $("[name='website']", form);
      if (hp && hp.value) return;

      var btn = $("[type='submit']", form);
      var original = btn.textContent;
      btn.disabled = true;
      btn.textContent = "Enviando…";

      /* Sustituye este bloque por tu llamada real, p. ej.:
         fetch(form.action, { method: "POST", body: new FormData(form) }) */
      setTimeout(function () {
        btn.disabled = false;
        btn.textContent = original;
        form.reset();
        fields.forEach(function (f) { f.removeAttribute("aria-invalid"); });
        if (form.id === "auth-form") {
          var mode = modal.getAttribute("data-mode");
          closeModal();
          toast(mode === "register" ? "¡Cuenta creada! Revisa tu correo para confirmarla." : "Sesión iniciada correctamente.");
        } else {
          toast("¡Gracias! Te responderemos en menos de 24 horas.");
        }
      }, 900);
    });
  });

  /* Mostrar / ocultar contraseña */
  $$("[data-toggle-password]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      var input = document.getElementById(btn.getAttribute("data-toggle-password"));
      var show = input.type === "password";
      input.type = show ? "text" : "password";
      btn.textContent = show ? "Ocultar" : "Mostrar";
      btn.setAttribute("aria-pressed", String(show));
    });
  });

  /* ---------- 12. Lazy loading de imágenes (img.lazy[data-src]) ---------- */
  var lazyImgs = $$("img.lazy[data-src]");
  function loadImg(img) {
    img.src = img.getAttribute("data-src");
    img.addEventListener("load", function () { img.classList.add("is-loaded"); }, { once: true });
  }
  if (hasIO) {
    var imgIO = new IntersectionObserver(function (entries, obs) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) { loadImg(entry.target); obs.unobserve(entry.target); }
      });
    }, { rootMargin: "200px" });
    lazyImgs.forEach(function (img) { imgIO.observe(img); });
  } else { lazyImgs.forEach(loadImg); }

  /* ---------- 13. Año actual en el pie ---------- */
  $$("[data-year]").forEach(function (el) { el.textContent = new Date().getFullYear(); });
})();
