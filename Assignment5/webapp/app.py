"""Small stateless portal used for reproducible deployment measurements."""
from flask import Flask, jsonify
import hashlib
import os
from urllib.parse import urlparse

app = Flask(__name__)


@app.get("/")
def index():
    return f"{os.getenv('APP_NAME', 'University Result Portal')} is ready.\n"


@app.get("/health")
def health():
    return jsonify(status="ok", service=os.getenv("APP_NAME", "result-portal"))


@app.get("/simulate/login")
def simulate_login():
    # Deterministic CPU work approximates a small password-hash cost without
    # storing credentials or any real student data in the assignment.
    digest = hashlib.pbkdf2_hmac("sha256", b"synthetic-password", b"assignment5-salt", 10_000)
    return jsonify(authenticated=bool(digest), service=os.getenv("APP_NAME", "result-portal"))


@app.get("/results/<int:student_id>")
def result(student_id):
    # Fixed mock result; deliberately no real student data is used in this lab.
    database_url = os.getenv("DATABASE_URL")
    if database_url:
        import psycopg2
        from psycopg2.extras import RealDictCursor
        parsed = urlparse(database_url)
        with psycopg2.connect(
            host=parsed.hostname, port=parsed.port or 5432,
            dbname=parsed.path.lstrip("/"), user=parsed.username,
            password=parsed.password, connect_timeout=2,
        ) as connection:
            with connection.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute("SELECT student_id, semester, grade FROM demo_results WHERE student_id = %s", (student_id,))
                row = cursor.fetchone()
        if row is None:
            return jsonify(error="not found"), 404
        return jsonify(dict(row) | {"service": os.getenv("APP_NAME", "result-portal")})
    return jsonify(student_id=student_id, semester="2025-odd", grade="A", service=os.getenv("APP_NAME", "result-portal"))


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "8000")))
