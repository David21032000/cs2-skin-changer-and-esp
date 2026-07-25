from http.server import HTTPServer, BaseHTTPRequestHandler
import json

class VerifyHandler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_POST(self):
        content_len = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_len).decode('utf-8') if content_len else '{}'
        try:
            data = json.loads(body)
            key = data.get('key', '')
            hwid = data.get('hwid', '')
            print(f'[AUTH] Key: {key}')
            print(f'[AUTH] HWID: {hwid}')
        except:
            pass

        response = {'status': 'ok', 'message': 'Key verified'}
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode('utf-8'))

    def log_message(self, format, *args):
        print(f'[SERVER] {args[0]} {args[1]} {args[2]}')

print('Starting auth server on http://127.0.0.1:8080')
print('Waiting for cheat verification...')
HTTPServer(('127.0.0.1', 8080), VerifyHandler).serve_forever()
