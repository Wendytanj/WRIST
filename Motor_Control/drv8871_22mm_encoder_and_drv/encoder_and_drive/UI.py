import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton
from PyQt5.QtGui import QFont

SERIAL_PORT = 'COM3'  
BAUD_RATE = 9600

class MotorControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.serial_conn = serial.Serial(port, baud, timeout=1)

        self.setWindowTitle("Motor Control UI")
        
        layout = QVBoxLayout()
        
        # Tighten button
        self.btn_cw = QPushButton("Clockwise")
        self.btn_cw.setCheckable(False)
        self.btn_cw.pressed.connect(self.start_cw)
        self.btn_cw.released.connect(self.stop_motors)
        
        # Release button
        self.btn_ccw = QPushButton("Counter-Clockwise")
        self.btn_ccw.setCheckable(False)
        self.btn_ccw.pressed.connect(self.start_ccw)
        self.btn_ccw.released.connect(self.stop_motors)
        
        layout.addWidget(self.btn_cw)
        layout.addWidget(self.btn_ccw)
        
        self.setLayout(layout)

    def start_cw(self):
        self.send_command("L\n")

    def start_ccw(self):
        self.send_command("R\n")

    def stop_motors(self):
        self.send_command("X\n")

    def send_command(self, command):
        if self.serial_conn.is_open:
            self.serial_conn.write(command.encode('utf-8'))


if __name__ == "__main__":
    
    app = QApplication(sys.argv)
    font = QFont("Arial", 12) 
    app.setFont(font)
    window = MotorControlUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
