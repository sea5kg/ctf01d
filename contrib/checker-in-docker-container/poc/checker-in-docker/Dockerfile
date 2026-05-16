FROM python:3.13-slim

WORKDIR /app

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

# копируем только requirements, сам checker будет через volume
CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:4109", "checker:app"]
