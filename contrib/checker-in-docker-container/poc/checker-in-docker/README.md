```bash
curl -X POST http://localhost:4109/put \
  -H "Content-Type: application/json" \
  -d '["service1", "id1", "flag1"]'
```

```bash
curl -X POST http://localhost:4109/check \
  -H "Content-Type: application/json" \
  -d '["service1", "id1", "flag1"]'
```