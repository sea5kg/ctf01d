# Grafana dashboard for ctf01d

## What this is

`GET /api/v1/metrics` exports the current ctf01d game state in Prometheus exposition format. The endpoint is built from the same in-memory scoreboard JSON as `GET /api/v1/scoreboard`.

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

The metrics endpoint is public, like the other `/api/v1/*` endpoints. If the jury host is reachable from the game network, allow Prometheus scrapes only from the management subnet with a firewall, or put `/api/v1/metrics` behind a reverse proxy with basic auth.
