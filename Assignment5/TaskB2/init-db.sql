CREATE TABLE IF NOT EXISTS demo_results (
  student_id INTEGER PRIMARY KEY,
  semester TEXT NOT NULL,
  grade TEXT NOT NULL
);
INSERT INTO demo_results (student_id, semester, grade)
VALUES (10001, '2025-odd', 'A')
ON CONFLICT (student_id) DO NOTHING;
