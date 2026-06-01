# Grafana dashboard for ctf01d

## What this is

`GET /api/v1/metrics` exports the current ctf01d game state in Prometheus exposition format. The endpoint is built from the same in-memory scoreboard JSON as `GET /api/v1/scoreboard`.

The endpoint is **disabled by default** and is meant for the jury, not players. See [Enabling the endpoint](#enabling-the-endpoint) below.

Inspect the live payload with `curl` or use this metric catalog:

| Metric | Type | Labels |
|---|---|---|
| `ctf01d_build_info` | gauge | `version` |
| `ctf01d_game_start_timestamp_seconds` | gauge | - |
| `ctf01d_game_end_timestamp_seconds` | gauge | - |
| `ctf01d_game_coffee_break_start_timestamp_seconds` | gauge | - |
| `ctf01d_game_coffee_break_end_timestamp_seconds` | gauge | - |
| `ctf01d_game_current_time_seconds` | gauge | - |
| `ctf01d_teams_total` | gauge | - |
| `ctf01d_services_total` | gauge | - |
| `ctf01d_team_score` | gauge | `team` |
| `ctf01d_team_place` | gauge | `team` |
| `ctf01d_team_tries_total` | counter | `team` |
| `ctf01d_team_service_sla_percent` | gauge | `team`, `service` |
| `ctf01d_team_service_status` | gauge | `team`, `service`, `status` |
| `ctf01d_team_service_defense_flags_total` | counter | `team`, `service` |
| `ctf01d_team_service_defense_points` | gauge | `team`, `service` |
| `ctf01d_team_service_attack_flags_total` | counter | `team`, `service` |
| `ctf01d_team_service_attack_points` | gauge | `team`, `service` |
| `ctf01d_service_attack_flags_total` | counter | `service` |
| `ctf01d_service_defense_flags_total` | counter | `service` |
| `ctf01d_service_first_blood_timestamp_seconds` | gauge | `service`, `team` |
| `ctf01d_flag_attempts_total` | counter | - |
| `ctf01d_flags_live` | gauge | - |

## Enabling the endpoint

The endpoint is off by default. Enable it in the jury `config.yml`:

```yaml
scoreboard:
  metrics_prometheus: yes                        # turn the endpoint on
  metrics_prometheus_allowed: "172.16.0.0/12"    # allowed scrapers, comma-separated IPs/CIDRs
```

Access rules, checked on every request (so toggling the flag does not require a restart):

- Only loopback (`127.0.0.0/8`, `::1`) is **always allowed**.
- Every other scraper must be listed in `metrics_prometheus_allowed`, e.g. `"172.18.0.0/16, 10.10.100.0/24, 203.0.113.5"`. This deliberately includes private ranges: on an A/D CTF the team game network is usually private (`10.0.0.0/8` etc.), so private addresses are **not** allowed implicitly — add only your monitoring/Docker subnet here, not the game network.
- Only IPv4 entries are matched in the allowlist (IPv6 clients other than `::1` are rejected).
- When the endpoint is disabled, or the client IP is not allowed, it returns **HTTP 403**.

## Wiring Prometheus

Copy the scrape job from `prometheus.example.yml` into the `scrape_configs:` section of your existing `prometheus.yml`, then reload Prometheus:

```bash
curl -X POST "$PROM/-/reload"
```

You can also send `SIGHUP` to the Prometheus process if lifecycle reloads are disabled.

## Importing the dashboard

In Grafana, open `Dashboards -> Import`, upload `dashboard.json`, and select your Prometheus datasource for the `$ds` variable.

## Sanity checks

```bash
curl -s http://localhost:8080/api/v1/metrics | head
curl -s http://localhost:8080/api/v1/metrics | promtool check metrics
```

In the Prometheus UI, `up{job="ctf01d"}` should be `1`.

## Security note

Unlike the other `/api/v1/*` endpoints, `/api/v1/metrics` is disabled by default and gated by the IP allowlist described in [Enabling the endpoint](#enabling-the-endpoint). It exposes game-health internals intended for the jury. Only loopback is allowed implicitly; keep `metrics_prometheus_allowed` limited to your management/monitoring hosts and never add the team game network there. The allowlist matches the socket peer address only — it does not honor `X-Forwarded-For`, so behind a reverse proxy every request appears to come from the proxy; rely on the proxy's own auth/ACLs in that setup. For defense in depth on a jury host reachable from the game network, also restrict scrapes with a firewall.
