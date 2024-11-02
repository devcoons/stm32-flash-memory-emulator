import os
import time
import threading
from flask import Flask, send_from_directory
from flask_socketio import SocketIO
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

app = Flask(__name__)
socketio = SocketIO(app, async_mode='threading', cors_allowed_origins='*')
flash_memory_file = os.path.abspath('flash_memory.bin')
flash_operations_file = os.path.abspath('flash_operations.log')
event_handler = None

class FileChangeHandler(FileSystemEventHandler):
    def __init__(self, files_to_watch, debounce_interval=0.05):
        super().__init__()
        self.files_to_watch = set(files_to_watch)
        self.debounce_interval = debounce_interval
        self.debounce_timer = None  
        self.lock = threading.Lock()
        self.update_in_progress = False
        self.pending_update = False

    def on_modified(self, event):
        file_path = os.path.abspath(event.src_path)
        if file_path in self.files_to_watch:
            with self.lock:
                if self.debounce_timer:
                    self.debounce_timer.cancel()
                self.debounce_timer = threading.Timer(self.debounce_interval, self.schedule_update)
                self.debounce_timer.start()

    def schedule_update(self):
        with self.lock:
            if not self.update_in_progress:
                self.update_in_progress = True
                print("Emitting update to client")
                socketio.emit('update')
            else:
                self.pending_update = True

@socketio.on('update_ack')
def handle_update_ack():
    handler = get_file_change_handler()
    with handler.lock:
        if handler.pending_update:
            handler.pending_update = False
            print("Emitting pending update to client")
            socketio.emit('update')
        else:
            handler.update_in_progress = False

def get_file_change_handler():
    return event_handler 

def start_file_monitoring():
    global event_handler
    event_handler = FileChangeHandler([flash_memory_file, flash_operations_file])
    observer = Observer()
    directories = {os.path.dirname(file) or '.' for file in [flash_memory_file, flash_operations_file]}
    for directory in directories:
        observer.schedule(event_handler, path=directory, recursive=False)
    observer.start()
    print(f"Monitoring started for: {flash_memory_file}, {flash_operations_file}")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

@app.route('/')
def index():
    return send_from_directory(os.path.dirname('index.html'), os.path.basename('index.html')) 

@app.route('/socket.io.min.js')
def serve_socket_io():
    return send_from_directory(os.path.dirname("socket.io.min.js"), os.path.basename("socket.io.min.js"))

@app.route('/flash_memory.bin')
def serve_flash_memory():
    return send_from_directory(os.path.dirname(flash_memory_file), os.path.basename(flash_memory_file))
    
@app.route('/flash_operations.log')
def serve_flash_operations():
    return send_from_directory(os.path.dirname(flash_operations_file), os.path.basename(flash_operations_file))

if __name__ == '__main__':
    socketio.start_background_task(target=start_file_monitoring)
    socketio.run(app)