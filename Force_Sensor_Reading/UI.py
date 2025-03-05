import sys
import serial
import serial.tools.list_ports
from PyQt5.QtCore import QThread, pyqtSignal, QTimer
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget,
    QVBoxLayout, QHBoxLayout, QPushButton, QLabel
)
import pyqtgraph as pg


class SerialWorker(QThread):
    data_received = pyqtSignal(float)
    
    def __init__(self, port, baudrate=57600, parent=None):
        super().__init__(parent)
        self.port = port
        self.baudrate = baudrate
        self.running = True
        self.ser = None
        self._send_tare = False
        
    def run(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            print(f"Opened {self.port} at {self.baudrate} baud.")
        except Exception as e:
            print("Failed to open serial port:", e)
            return
        
        while self.running:
            # If a tare command is requested, send it
            if self._send_tare:
                self.ser.write(b"TARE\n")
                self._send_tare = False

            try:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    # For example, Arduino prints:
                    # "Raw reading: 300   Force (PSI - NB): 45   Force (N): 0.88"
                    # We want to extract the force in newtons.
                    if "Force (N):" in line:
                        try:
                            parts = line.split("Force (N):")
                            force_str = parts[1].strip()
                            force = float(force_str)
                            self.data_received.emit(force)
                        except Exception as e:
                            # Could not parse; ignore this line.
                            pass
                    else:
                        # Alternatively, if the Arduino sends just a number:
                        try:
                            force = float(line)
                            self.data_received.emit(force)
                        except:
                            pass
            except Exception as e:
                print("Error reading serial:", e)
                
        if self.ser:
            self.ser.close()
            
    def stop(self):
        self.running = False
        self.wait()
        
    def send_tare(self):
        self._send_tare = True


class MainWindow(QMainWindow):
    def __init__(self, port):
        super().__init__()
        self.setWindowTitle("SingleTact Force Plot")
        self.resize(800, 600)
        
        # Set up serial worker
        self.serial_worker = SerialWorker(port)
        self.serial_worker.data_received.connect(self.on_new_data)
        self.serial_worker.start()
        
        # Data storage for plotting
        self.force_data = []
        self.max_points = 500  # e.g., store last 500 points
        
        # Set up pyqtgraph plot
        self.plot_widget = pg.PlotWidget()
        self.plot_curve = self.plot_widget.plot(pen='y')
        self.plot_widget.setLabel('left', "Force (N)")
        self.plot_widget.setLabel('bottom', "Samples")
        
        # Tare button and status label
        self.tare_button = QPushButton("Tare")
        self.tare_button.clicked.connect(self.on_tare)
        
        self.status_label = QLabel("Force (N): 0.00")
        
        # Layout
        main_layout = QVBoxLayout()
        main_layout.addWidget(self.plot_widget)
        h_layout = QHBoxLayout()
        h_layout.addWidget(self.tare_button)
        h_layout.addWidget(self.status_label)
        main_layout.addLayout(h_layout)
        
        container = QWidget()
        container.setLayout(main_layout)
        self.setCentralWidget(container)
        
        # Timer to refresh the plot periodically
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(50)  # update every 50 ms
        
    def on_new_data(self, force):
        """Slot called from the serial thread when new force data is received."""
        self.force_data.append(force)
        if len(self.force_data) > self.max_points:
            self.force_data = self.force_data[-self.max_points:]
        self.status_label.setText(f"Force (N): {force:.2f}")
        
    def update_plot(self):
        if self.force_data:
            self.plot_curve.setData(self.force_data)
        
    def on_tare(self):
        """Called when the Tare button is pressed."""
        self.serial_worker.send_tare()
        
    def closeEvent(self, event):
        self.serial_worker.stop()
        event.accept()


def find_com_port():
    """Return the first available COM port that might be the Arduino."""
    ports = list(serial.tools.list_ports.comports())
    if ports:
        # Optionally, filter ports by description if needed.
        return "COM10"
    return None


if __name__ == '__main__':
    port = find_com_port()
    if not port:
        print("No COM ports found.")
        sys.exit(1)
        
    print("Using COM port:", port)
    app = QApplication(sys.argv)
    window = MainWindow(port)
    window.show()
    sys.exit(app.exec_())
