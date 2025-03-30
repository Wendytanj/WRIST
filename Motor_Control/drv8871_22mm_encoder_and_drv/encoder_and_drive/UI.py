import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QLabel
from PyQt5.QtGui import QFont
from PyQt5.QtCore import QTimer

SERIAL_PORT = 'COM9'
BAUD_RATE = 9600

class MotorControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.serial_conn = serial.Serial(port, baud, timeout=1)

        self.setWindowTitle("Motor Control UI")
        layout = QVBoxLayout()
        
        # Motor control buttons
        self.btn_cw = QPushButton("Clockwise")
        self.btn_cw.setCheckable(False)
        self.btn_cw.pressed.connect(self.start_cw)
        self.btn_cw.released.connect(self.stop_motors)
        
        self.btn_ccw = QPushButton("Counter-Clockwise")
        self.btn_ccw.setCheckable(False)
        self.btn_ccw.pressed.connect(self.start_ccw)
        self.btn_ccw.released.connect(self.stop_motors)
        
        layout.addWidget(self.btn_cw)
        layout.addWidget(self.btn_ccw)
        
        # Labels to display encoder data
        self.label_pos = QLabel("Position: --")
        self.label_angle = QLabel("Angle: --")
        layout.addWidget(self.label_pos)
        layout.addWidget(self.label_angle)
        
        self.setLayout(layout)
        
        # Set up a timer to update encoder data periodically
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_encoder_data)
        self.timer.start(100)  # update every 100 milliseconds

    def start_cw(self):
        self.send_command("L\n")

    def start_ccw(self):
        self.send_command("R\n")

    def stop_motors(self):
        self.send_command("X\n")

    def send_command(self, command):
        if self.serial_conn.is_open:
            self.serial_conn.write(command.encode('utf-8'))
            
    def update_encoder_data(self):
        # Read a line from the serial port if available
        if self.serial_conn.in_waiting:
            try:
                # Read and decode a line (e.g. "123,45.67")
                line = self.serial_conn.readline().decode('utf-8').strip()
                parts = line.split(',')
                if len(parts) >= 2:
                    pos = parts[0].strip()
                    angle = parts[1].strip()
                    # Update the labels with the new data
                    self.label_pos.setText(f"Position: {pos}")
                    self.label_angle.setText(f"Angle: {angle}°")
            except Exception as e:
                print("Error reading encoder data:", e)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    font = QFont("Arial", 12)
    app.setFont(font)
    window = MotorControlUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
