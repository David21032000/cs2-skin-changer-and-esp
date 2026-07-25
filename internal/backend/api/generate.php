<?php
header('Content-Type: application/json');

$admin_hash = '$2y$12$LJ3m4ys3Lk4Hx8R5W6z7AeBgTq2WxY9zFvNpKcRfDsGhJkMlQnPo';

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['status' => 'error', 'message' => 'Method not allowed']);
    exit;
}

$input = json_decode(file_get_contents('php://input'), true);

if (!$input || empty($input['admin_password']) || empty($input['days'])) {
    http_response_code(400);
    echo json_encode(['status' => 'error', 'message' => 'Missing admin_password or days']);
    exit;
}

if (!password_verify($input['admin_password'], $admin_hash)) {
    http_response_code(403);
    echo json_encode(['status' => 'error', 'message' => 'Invalid admin password']);
    exit;
}

$days = intval($input['days']);
if ($days < 1 || $days > 3650) {
    http_response_code(400);
    echo json_encode(['status' => 'error', 'message' => 'Days must be between 1 and 3650']);
    exit;
}

$key = bin2hex(random_bytes(16));
$expires_at = date('Y-m-d H:i:s', strtotime("+{$days} days"));

$mysqli = new mysqli('localhost', 'camus_user', 'passwordplaceholder', 'camus_db');

if ($mysqli->connect_error) {
    http_response_code(500);
    echo json_encode(['status' => 'error', 'message' => 'Database connection failed']);
    exit;
}

$stmt = $mysqli->prepare("INSERT INTO keys (key_code, days, expires_at) VALUES (?, ?, ?)");
$stmt->bind_param("sis", $key, $days, $expires_at);

if ($stmt->execute()) {
    echo json_encode([
        'status' => 'ok',
        'key' => $key,
        'days' => $days,
        'expires_at' => $expires_at
    ]);
} else {
    http_response_code(500);
    echo json_encode(['status' => 'error', 'message' => 'Failed to insert key']);
}

$stmt->close();
$mysqli->close();
