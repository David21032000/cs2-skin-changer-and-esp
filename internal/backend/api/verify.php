<?php
header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['status' => 'error', 'message' => 'Method not allowed']);
    exit;
}

$input = json_decode(file_get_contents('php://input'), true);

if (!$input || empty($input['key'])) {
    http_response_code(400);
    echo json_encode(['status' => 'error', 'message' => 'Missing key parameter']);
    exit;
}

$key = $input['key'];
$hwid = $input['hwid'] ?? null;

$mysqli = new mysqli('localhost', 'camus_user', 'passwordplaceholder', 'camus_db');

if ($mysqli->connect_error) {
    http_response_code(500);
    echo json_encode(['status' => 'error', 'message' => 'Database connection failed']);
    exit;
}

$stmt = $mysqli->prepare("SELECT hwid, expires_at, active FROM keys WHERE key_code = ?");
$stmt->bind_param("s", $key);
$stmt->execute();
$result = $stmt->get_result();
$row = $result->fetch_assoc();
$stmt->close();

if (!$row) {
    echo json_encode(['status' => 'error', 'message' => 'Invalid key']);
    $mysqli->close();
    exit;
}

if (!$row['active']) {
    echo json_encode(['status' => 'error', 'message' => 'Key is disabled']);
    $mysqli->close();
    exit;
}

if ($row['expires_at'] && strtotime($row['expires_at']) < time()) {
    echo json_encode(['status' => 'error', 'message' => 'Key has expired']);
    $mysqli->close();
    exit;
}

if ($hwid) {
    if ($row['hwid'] === null) {
        $updateStmt = $mysqli->prepare("UPDATE keys SET hwid = ?, last_login = NOW() WHERE key_code = ?");
        $updateStmt->bind_param("ss", $hwid, $key);
        $updateStmt->execute();
        $updateStmt->close();
    } elseif ($row['hwid'] !== $hwid) {
        echo json_encode(['status' => 'error', 'message' => 'HWID mismatch']);
        $mysqli->close();
        exit;
    } else {
        $updateStmt = $mysqli->prepare("UPDATE keys SET last_login = NOW() WHERE key_code = ?");
        $updateStmt->bind_param("s", $key);
        $updateStmt->execute();
        $updateStmt->close();
    }
}

$mysqli->close();

echo json_encode([
    'status' => 'ok',
    'expires' => $row['expires_at']
]);
