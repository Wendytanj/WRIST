import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton

SERIAL_PORT = 'COM6'  
BAUD_RATE = 9600

class MotorControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.serial_conn = serial.Serial(port, baud, timeout=1)

        self.setWindowTitle("Motor Control UI")
        
        layout = QVBoxLayout()
        
        # Tighten button
        self.btn_tighten = QPushButton("Tighten")
        self.btn_tighten.setCheckable(False)
        self.btn_tighten.pressed.connect(self.start_tighten)
        self.btn_tighten.released.connect(self.stop_motors)
        
        # Release button
        self.btn_release = QPushButton("Release")
        self.btn_release.setCheckable(False)
        self.btn_release.pressed.connect(self.start_release)
        self.btn_release.released.connect(self.stop_motors)
        
        layout.addWidget(self.btn_tighten)
        layout.addWidget(self.btn_release)
        
        self.setLayout(layout)

    def start_tighten(self):
        self.send_command("T\n")

    def start_release(self):
        self.send_command("R\n")

    def stop_motors(self):
        self.send_command("X\n")

    def send_command(self, command):
        if self.serial_conn.is_open:
            self.serial_conn.write(command.encode('utf-8'))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MotorControlUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
