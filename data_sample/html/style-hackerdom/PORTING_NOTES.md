Аудит того, что было в `.ep` шаблонах, но не доехало в наш фронт.

## Перенесено

| checksystem | наш фронт | где |
|---|---|---|
| Тема Bootstrap (navbar, table-condensed) | ✓ | `index.html`, `css/bootstrap*.css` |
| Цветовая схема статусов (`.status_up/corrupt/mumble/down`) | ✓ | `css/default.css` |
| Header-полоса со статусами | ✓ | `index.html:25-32` |
| Гид-стиль «1/2/3 место» (золото/серебро/бронза `tr:nth-child`) | ✓ | `css/default.css:39-53` |
| Cell-вёрстка с `SLA/FP/⚑` (`param_name`+`param_value`) | ✓ | `scoreboard.js:60-80` |
| Лого команды + fallback при 404 | ✓ | `scoreboard.js:85-87` (`onerror`→`/logo.png`) |
| `status_shit` (которого в checksystem не было) | ✓ доп. | `css/default.css` |

## Не перенесено (точечно, можно дозалить)

| Что | Где было в checksystem | Почему пропустил |
|---|---|---|
| `(+N)` / `(−N)` после места — delta | `scoreboard.html.ep:25-26` | в JSON ctf01d нет `delta` (см. предыдущий ответ) |
| `-<sflags>` (правая часть ⚑) | `scoreboard.html.ep:53` | в JSON нет `sflags` |
| `?wide=1` режим (растягивает container на всю ширину) | `scoreboard.html.ep:2` | тривиально, просто не сделал — пропустил |
| `tooltip` (стандартный `title=`) с `stdout` чекера на ячейке сервиса | `scoreboard.html.ep:37` | админка, не публично |
| Кликабельное имя команды → `/team/<id>` | `scoreboard.html.ep:29` | нет эндпоинта `team/<id>`, у меня `<a href="#">` пустая ссылка |
| WebSocket push-обновления | `main/index.html.ep:11-39` | заменено на `setInterval` 5 сек |
| `Round N` в навбаре | `main/index.html.ep:5-6` | заменил на `HH:MM:SS` от старта игры |
| Имя игры из `app->ctf_name` в брэнде navbar | `layouts/default.html.ep:15-17` | ✓ есть — но из `/api/v1/game[game_name]`, отображается одинаково |
| Глобальные ссылки в navbar (`app->config->{cs}{links}`) | `layouts/default.html.ep:21-23` | в ctf01d нет такой конфиг-секции; пропустил |
| `glyphicon-asterisk` кнопка → `admin_view` | `scoreboard.html.ep:42-46` | admin-only |
| Таблица описания статусов с `$checker->statuses` (динамический список) | `scoreboard.html.ep:4-9` | у меня захардкожены 4 статуса в `index.html` (нет эндпоинта со списком возможных статусов) |

## Целые страницы / роуты — не перенесено вовсе

| `.ep` шаблон | Что делал | Почему не сделал |
|---|---|---|
| `templates/main/team.html.ep` | страница одной команды (раунды) | нет `/api/v1/team/<id>` |
| `templates/admin/index.html.ep` | админская копия scoreboard | нет ни auth, ни админ API |
| `templates/admin/view.html.ep` | просмотр чекерного запуска (stdout/stderr) | нет API |
| `templates/admin/info.html.ep` | статус игры / расписание | частично можно собрать из `/api/v1/game`, не сделал |

## Что ещё стоит докрутить именно во фронте (без новых эндпоинтов)

1. **Имя команды → якорь на её строку** (раз `team/<id>` страницы нет): `<a href="#team-<id>">` со скроллом, или `data-team-id` для подсветки.
2. **`?wide=1`** — `<style>` правит `#scoreboard_wrapper{width:auto}` при флаге в URL. 5 строк JS.
3. **Список статусов из `s_sta` ключей** — у нас он сейчас захардкожен. В JSON `s_sta` есть динамика per-service, но не список *возможных статусов*. Можно собрать union из всех `ts_sta[*][svc].status` — лёгкий, но шумный.
4. **`game_has_coffee_break` + `tc`** уже в `/api/v1/game` — можно показать баннер «кофе-брейк до HH:MM».
5. **Иконки glyphicon** — в bootstrap-theme.css ссылаются на `/fonts/glyphicons-halflings-regular.*`, файлы у нас есть, но фактически в верстке glyphicon'ы появляются только в admin-кнопке. Без admin'а можно вообще выкинуть bootstrap-theme.css + fonts/ — экономия трафика.

---

Прагматичное предложение: из этого списка перенос окупается только в пп. 1–2 и (если допилить ctf01d) — delta+sflags из предыдущего ответа. Всё остальное либо требует новых эндпоинтов на бэкенде, либо это admin, который сознательно не делается публично.
