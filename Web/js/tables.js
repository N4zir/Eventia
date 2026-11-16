/* EVENTIA — Listados: búsqueda, filtro por enlace (?q=) y alta de filas (demo) */
(function () {
  "use strict";
  var table = document.querySelector("[data-table]");
  if (!table) return;

  var body = table.tBodies[0];
  var search = document.getElementById("search");
  var count = document.getElementById("count");
  var empty = document.getElementById("empty");
  var status = document.getElementById("status");
  var MONTHS = ["ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic"];

  function norm(s) { return s.toLowerCase().normalize("NFD").replace(/[\u0300-\u036f]/g, ""); }

  function applyFilter() {
    var q = norm(search.value.trim());
    var rows = Array.prototype.slice.call(body.rows);
    var shown = 0;
    rows.forEach(function (r) {
      var ok = !q || norm(r.textContent).indexOf(q) > -1;
      r.hidden = !ok;
      if (ok) shown++;
    });
    count.textContent = shown + " de " + rows.length;
    empty.hidden = shown > 0;
  }

  search.addEventListener("input", applyFilter);
  var preset = new URLSearchParams(window.location.search).get("q");
  if (preset) search.value = preset;
  applyFilter();

  /* ---- Alta de filas (solo en pantalla; conecta aquí tu backend) ---- */
  var dlg = document.getElementById("new-dialog");
  if (!dlg || typeof dlg.showModal !== "function") return;
  var form = dlg.querySelector("form");

  document.getElementById("open-new").addEventListener("click", function () {
    form.reset();
    dlg.showModal();
  });
  dlg.querySelector("[data-cancel]").addEventListener("click", function () { dlg.close(); });

  function fmtDate(v, withTime) {
    var m = v.match(/^(\d{4})-(\d{2})-(\d{2})(?:T(\d{2}:\d{2}))?/);
    if (!m) return v;
    var out = parseInt(m[3], 10) + " " + MONTHS[parseInt(m[2], 10) - 1] + " " + m[1];
    return withTime && m[4] ? out + ", " + m[4] : out;
  }

  /* Reglas equivalentes a las restricciones de la base de datos */
  var groups = (table.getAttribute("data-unique") || "").split(";").filter(Boolean).map(function (g) { return g.split(","); });
  var heads = Array.prototype.map.call(table.tHead.rows[0].cells, function (th) { return th.getAttribute("data-field"); });

  function val(el) { return el.tagName === "SELECT" ? el.options[el.selectedIndex].text : el.value; }

  function duplicateField() {
    for (var g = 0; g < groups.length; g++) {
      var fields = groups[g];
      var idx = fields.map(function (f) { return heads.indexOf(f); });
      if (idx.indexOf(-1) > -1 || fields.some(function (f) { return !form.elements[f]; })) continue;
      var mine = fields.map(function (f) { return norm(val(form.elements[f]).trim()); });
      for (var r = 0; r < body.rows.length; r++) {
        var same = idx.every(function (ci, k) { return norm(body.rows[r].cells[ci].textContent.trim()) === mine[k]; });
        if (same) return { field: fields[0], composite: fields.length > 1 };
      }
    }
    return null;
  }

  function clearValidity() {
    Array.prototype.forEach.call(form.elements, function (el) { if (el.setCustomValidity) el.setCustomValidity(""); });
  }
  form.addEventListener("input", clearValidity);
  form.addEventListener("change", clearValidity);

  form.addEventListener("submit", function (e) {
    e.preventDefault();
    clearValidity();

    var ini = form.elements["FechaInicio"], fin = form.elements["FechaFin"];
    if (ini && fin && ini.value && fin.value && fin.value < ini.value)
      fin.setCustomValidity("La fecha de fin no puede ser anterior a la de inicio.");

    var dup = duplicateField();
    if (dup) form.elements[dup.field].setCustomValidity(dup.composite ? "Esa combinación ya existe." : "Ya existe un registro con este valor.");

    if (!form.checkValidity()) { form.reportValidity(); return; }

    /* Ejemplo real: fetch("/api/<tabla>", { method: "POST", body: new FormData(form) }) */
    var ids = Array.prototype.map.call(body.rows, function (r) { return parseInt(r.getAttribute("data-id"), 10) || 0; });
    var id = Math.max.apply(null, ids.concat(0)) + 1;
    var tr = document.createElement("tr");
    tr.className = "is-new";
    tr.setAttribute("data-id", id);

    Array.prototype.forEach.call(table.tHead.rows[0].cells, function (th) {
      var td = document.createElement("td");
      var f = th.getAttribute("data-field");
      var el = f ? form.elements[f] : null;
      if (f === "Id") { td.className = "id"; td.textContent = id; }
      else if (th.hasAttribute("data-num")) { td.className = "num"; td.textContent = el ? el.value : "0"; }
      else if (!el) { td.textContent = "—"; }
      else if (el.type === "checkbox") { td.textContent = el.checked ? (el.getAttribute("data-yes") || "Sí") : (el.getAttribute("data-no") || "No"); }
      else if (el.tagName === "SELECT") { td.textContent = el.value === "" ? "—" : el.options[el.selectedIndex].text; }
      else if (el.type === "date") { td.textContent = fmtDate(el.value, false); }
      else if (el.type === "datetime-local") { td.textContent = fmtDate(el.value, true); }
      else { td.textContent = el.value; }
      tr.appendChild(td);
    });

    body.appendChild(tr);
    dlg.close();
    search.value = "";
    applyFilter();
    status.textContent = "Fila añadida (vista previa: aún no se guarda en la base de datos).";
    tr.scrollIntoView({ block: "nearest" });
  });
})();
