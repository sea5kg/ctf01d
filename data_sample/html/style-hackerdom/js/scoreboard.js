(function () {
  'use strict';

  var POLL_INTERVAL_MS = 5000;
  var DEFAULT_LOGO = './logo.png';

  var state = {
    services: [],
    teams: {},
    headerRendered: false,
  };

  function fetchJson(url) {
    return fetch(url, { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error(url + ' -> ' + r.status);
      return r.json();
    });
  }

  function escapeHtml(s) {
    if (s == null) return '';
    return String(s)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }

  function buildHeader() {
    var thead = document.querySelector('#scoreboard thead tr.team');
    if (!thead) return;
    var svcHtml = state.services
      .map(function (s) {
        return '<th class="service_name">' + escapeHtml(s.name) + '</th>';
      })
      .join('');
    thead.innerHTML =
      '<th class="place">#</th>' +
      '<th colspan="2">team</th>' +
      '<th>score</th>' +
      svcHtml;
    state.headerRendered = true;
  }

  function statusClass(status) {
    if (!status) return '';
    return 'status_' + status;
  }

  function renderTeamRow(teamId, teamData) {
    var info = state.teams[teamId] || { name: teamId, ip_address: '', logo: DEFAULT_LOGO };
    var logoUrl = info.logo || DEFAULT_LOGO;
    var sb = teamData.ts_sta || {};

    var cells = state.services
      .map(function (svc) {
        var s = sb[svc.id] || {};
        var cls = statusClass(s.status);
        var sla = s.sla != null ? s.sla : 0;
        var fp = s.pt_def != null ? Number(s.pt_def).toFixed(1) : '0.0';
        var flags = s.att != null ? s.att : 0;
        return (
          '<td class="team_service ' + cls + '">' +
            '<div class="sla"><div class="param_name">SLA</div><div class="param_value">' +
              escapeHtml(sla) + '%</div></div>' +
            '<div class="fp"><div class="param_name">FP</div><div class="param_value">' +
              escapeHtml(fp) + '</div></div>' +
            '<div class="flags"><div class="param_name">⚑</div><div class="param_value">' +
              escapeHtml(flags) + '</div></div>' +
          '</td>'
        );
      })
      .join('');

    var place = teamData.place != null ? teamData.place : '';
    var points = teamData.points != null ? Number(teamData.points).toFixed(1) : '0.0';

    return (
      '<tr class="team" data-team-id="' + escapeHtml(teamId) + '">' +
        '<td class="place">' + escapeHtml(place) + '</td>' +
        '<td class="team_logo">' +
          '<img width="64px" class="img" src="' + escapeHtml("../" + logoUrl) + '" ' +
          'onerror="this.onerror=null;this.src=\'' + DEFAULT_LOGO + '\';">' +
        '</td>' +
        '<td class="team_info">' +
          '<div class="team_name">' +
            '<a href="#" data-team-id="' + escapeHtml(teamId) + '">' +
              escapeHtml(info.name) +
            '</a>' +
          '</div>' +
          '<div class="team_server">' + escapeHtml(info.ip_address || '') + '</div>' +
        '</td>' +
        '<td class="score">' + escapeHtml(points) + '</td>' +
        cells +
      '</tr>'
    );
  }

  function renderScoreboard(scoreboardJson) {
    if (!state.headerRendered) buildHeader();

    var sb = scoreboardJson.scoreboard || {};
    var ids = Object.keys(sb).sort(function (a, b) {
      var pa = sb[a].place != null ? sb[a].place : 9999;
      var pb = sb[b].place != null ? sb[b].place : 9999;
      if (pa !== pb) return pa - pb;
      return (sb[b].points || 0) - (sb[a].points || 0);
    });

    var body = document.getElementById('scoreboard_body');
    if (!body) return;
    body.innerHTML = ids.map(function (id) { return renderTeamRow(id, sb[id]); }).join('');

    var game = scoreboardJson.game || {};
    var roundEl = document.getElementById('round');
    if (roundEl && game.tc && game.t0) {
      var elapsed = Math.max(0, game.tc - game.t0);
      var hh = String(Math.floor(elapsed / 3600)).padStart(2, '0');
      var mm = String(Math.floor((elapsed % 3600) / 60)).padStart(2, '0');
      var ss = String(elapsed % 60).padStart(2, '0');
      roundEl.textContent = hh + ':' + mm + ':' + ss;
    }
  }

  function poll() {
    fetchJson('../api/v1/scoreboard')
      .then(renderScoreboard)
      .catch(function (e) { console.error('scoreboard poll failed:', e); });
  }

  function bootstrap() {
    fetchJson('../api/v1/game')
      .then(function (game) {
        state.services = (game.services || []).slice().sort(function (a, b) {
          return String(a.id).localeCompare(String(b.id));
        });
        (game.teams || []).forEach(function (t) {
          state.teams[t.id] = t;
        });
        var nameEl = document.getElementById('game_name');
        if (nameEl && game.game_name) nameEl.textContent = game.game_name;
        document.title = game.game_name || 'ctf01d';
        buildHeader();
        poll();
        setInterval(poll, POLL_INTERVAL_MS);
      })
      .catch(function (e) {
        console.error('bootstrap failed:', e);
        setTimeout(bootstrap, POLL_INTERVAL_MS);
      });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', bootstrap);
  } else {
    bootstrap();
  }
})();
