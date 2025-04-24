import sys
import serial
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QPushButton, QLabel, QSlider
)
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt

SERIAL_PORT = 'COM10'  
BAUD_RATE = 115200

class MotorControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.serial_conn = serial.Serial(port, baud, timeout=1)

        self.setWindowTitle("Motor Control UI")
        layout = QVBoxLayout()
        
        # Motor direction buttons
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
        
        # Angle slider and label
        self.angle_label = QLabel("Angle: 90°")
        self.angle_slider = QSlider(Qt.Horizontal)
        self.angle_slider.setMinimum(0)
        self.angle_slider.setMaximum(360)
        self.angle_slider.setValue(90)
        self.angle_slider.setTickInterval(10)
        self.angle_slider.setTickPosition(QSlider.TicksBelow)
        self.angle_slider.valueChanged.connect(self.update_angle_label)
        
        layout.addWidget(self.angle_label)
        layout.addWidget(self.angle_slider)

        #Speed slide
        self.speed_label = QLabel("Speed: 70")
        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setMinimum(70)
        self.speed_slider.setMaximum(255)
        self.speed_slider.setValue(80)
        self.speed_slider.setTickInterval(5)
        self.speed_slider.setTickPosition(QSlider.TicksBelow)
        self.speed_slider.valueChanged.connect(self.update_speed_label)
        
        layout.addWidget(self.speed_label)
        layout.addWidget(self.speed_slider)

        self.btn_set_speed = QPushButton("Set Speed")
        self.btn_set_speed.setCheckable(False)
        self.btn_set_speed.pressed.connect(lambda: self.set_speed(self.speed_slider.value()))
        
        layout.addWidget(self.btn_set_speed)
        
        # Turn angle buttons
        self.btn_turn_cw = QPushButton("Turn Clockwise Angle")
        self.btn_turn_cw.setCheckable(False)
        # When pressed, send the slider's current value as the angle
        self.btn_turn_cw.pressed.connect(lambda: self.start_turn_cw(self.angle_slider.value()))
        
        self.btn_turn_ccw = QPushButton("Turn Counter-Clockwise Angle")
        self.btn_turn_ccw.setCheckable(False)
        self.btn_turn_ccw.pressed.connect(lambda: self.start_turn_ccw(self.angle_slider.value()))

        layout.addWidget(self.btn_turn_cw)
        layout.addWidget(self.btn_turn_ccw)
        
        
        self.setLayout(layout)

    def update_angle_label(self, value):
        self.angle_label.setText(f"Angle: {value}°")

    def update_speed_label(self, value):
        self.speed_label.setText(f"Speed: {value}")
        
    def start_cw(self):
        self.send_command("L\n")

    def start_ccw(self):
        self.send_command("R\n")

    def start_turn_cw(self, angle):
        # Command "A" followed by the angle and newline
        self.send_command("A", angle, "\n")

    def start_turn_ccw(self, angle):
        # Command "B" followed by the angle and newline
        self.send_command("B", angle, "\n")

    def stop_motors(self):
        self.send_command("X\n")
    
    def set_speed(self, speed):
        self.send_command("S", speed, "\n")

    # send_command now concatenates its arguments
    def send_command(self, *args):
        if self.serial_conn.is_open:
            command = ''.join(str(arg) for arg in args)
            self.serial_conn.write(command.encode('utf-8'))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    font = QFont("Arial", 12)
    app.setFont(font)
    window = MotorControlUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
