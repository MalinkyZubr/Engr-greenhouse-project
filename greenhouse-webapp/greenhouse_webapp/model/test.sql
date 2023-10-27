CREATE TABLE test_table (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255),
    age INT,
    email VARCHAR(255),
    created_at TIMESTAMP
);

INSERT INTO test_table (name, age, email, created_at)
VALUES ('John Doe', 30, 'john.doe@example.com', NOW());

-- Insert the second example entry
INSERT INTO test_table (name, age, email, created_at)
VALUES ('Jane Smith', 25, 'jane.smith@example.com', NOW());

-- Insert the third example entry
INSERT INTO test_table (name, age, email, created_at)
VALUES ('Bob Johnson', 40, 'bob.johnson@example.com', NOW());