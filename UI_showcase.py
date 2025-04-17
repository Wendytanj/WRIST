import sys
import serial
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QSlider, QGroupBox, QMessageBox
)
from PyQt5.QtGui import QFont, QIcon
from PyQt5.QtCore import Qt

SERIAL_PORT = 'COM15'
BAUD_RATE = 115200

# Styles for indicators
LED_ON_STYLE = "background-color: #0f0; border-radius: 12px;"
LED_OFF_STYLE = "background-color: #555; border-radius: 12px;"

class MotorVibrationControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.setWindowTitle("Motor & Vibration Control UI")
        self.resize(400, 700)

        # Attempt serial connection
        try:
            self.serial_conn = serial.Serial(port, baud, timeout=1)
        except Exception as e:
            self.serial_conn = None
            QMessageBox.warning(
                self,
                "Serial Connection",
                f"Could not open {port}: {e}\nRunning in UI-only mode."
            )

        layout = QVBoxLayout()

        # Motor control group
        motor_group = QGroupBox("Motor Control")
        motor_layout = QVBoxLayout()

        # Direction buttons with icons and text
        dir_layout = QHBoxLayout()
        cw_btn = QPushButton("CW")
        cw_btn.setIcon(QIcon('icons/cw.png'))  # replace with actual icon path
        cw_btn.setIconSize(cw_btn.sizeHint())
        cw_btn.pressed.connect(lambda: self.send_command("L\n"))
        cw_btn.released.connect(lambda: self.send_command("X\n"))
        ccw_btn = QPushButton("CCW")
        ccw_btn.setIcon(QIcon('icons/ccw.png'))
        ccw_btn.setIconSize(ccw_btn.sizeHint())
        ccw_btn.pressed.connect(lambda: self.send_command("R\n"))
        ccw_btn.released.connect(lambda: self.send_command("X\n"))
        dir_layout.addWidget(cw_btn)
        dir_layout.addWidget(ccw_btn)
        motor_layout.addLayout(dir_layout)

        # Angle slider
        self.angle_label = QLabel("Angle: 90°")
        self.angle_slider = QSlider(Qt.Horizontal)
        self.angle_slider.setRange(0, 360)
        self.angle_slider.setValue(90)
        self.angle_slider.valueChanged.connect(
            lambda v: self.angle_label.setText(f"Angle: {v}°")
        )
        motor_layout.addWidget(self.angle_label)
        motor_layout.addWidget(self.angle_slider)

        # Speed slider
        self.speed_label = QLabel("Speed: 80")
        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setRange(0, 255)
        self.speed_slider.setValue(80)
        self.speed_slider.valueChanged.connect(
            lambda v: self.speed_label.setText(f"Speed: {v}")
        )
        motor_layout.addWidget(self.speed_label)
        motor_layout.addWidget(self.speed_slider)

        # Set speed button
        speed_btn = QPushButton("Set Speed")
        speed_btn.pressed.connect(
            lambda: self.send_command(f"S {self.speed_slider.value()}\n")
        )
        motor_layout.addWidget(speed_btn)

        # Turn angle buttons
        turn_layout = QHBoxLayout()
        turn_cw_btn = QPushButton("Turn CW")
        turn_cw_btn.pressed.connect(
            lambda: self.send_command(f"A {self.angle_slider.value()}\n")
        )
        turn_ccw_btn = QPushButton("Turn CCW")
        turn_ccw_btn.pressed.connect(
            lambda: self.send_command(f"B {self.angle_slider.value()}\n")
        )
        turn_layout.addWidget(turn_cw_btn)
        turn_layout.addWidget(turn_ccw_btn)
        motor_layout.addLayout(turn_layout)

        motor_group.setLayout(motor_layout)
        layout.addWidget(motor_group)

        # Vibration control group with indicators
        vib_group = QGroupBox("Vibration Control")
        vib_layout = QVBoxLayout()

        # Amplitude slider
        self.vib_amp_label = QLabel("Amp: 70")
        self.vib_amp_slider = QSlider(Qt.Horizontal)
        self.vib_amp_slider.setRange(0, 255)
        self.vib_amp_slider.setValue(70)
        self.vib_amp_slider.valueChanged.connect(
            lambda v: self.vib_amp_label.setText(f"Amp: {v}")
        )
        vib_layout.addWidget(self.vib_amp_label)
        vib_layout.addWidget(self.vib_amp_slider)

        # Indicator lights for 5 devices
        indicator_layout = QHBoxLayout()
        self.indicators = []
        for _ in range(5):
            led = QLabel()
            led.setFixedSize(24, 24)
            led.setStyleSheet(LED_OFF_STYLE)
            indicator_layout.addWidget(led)
            self.indicators.append(led)
        vib_layout.addLayout(indicator_layout)

        # Device buttons
        btns_layout = QHBoxLayout()
        for i in range(1, 6):
            btn = QPushButton(str(i))
            btn.setFixedWidth(50)
            btn.pressed.connect(lambda i=i: self.activate_device(i))
            btns_layout.addWidget(btn)
        vib_layout.addLayout(btns_layout)

        # Big 'Set All' button
        all_btn = QPushButton("Set All Vibrators")
        all_btn.setMinimumHeight(40)
        all_btn.pressed.connect(self.activate_all)
        vib_layout.addWidget(all_btn)

        # 'Stop All' button
        stop_btn = QPushButton("Stop All")
        stop_btn.setMinimumHeight(40)
        stop_btn.pressed.connect(self.deactivate_all)
        vib_layout.addWidget(stop_btn)

        vib_group.setLayout(vib_layout)
        layout.addWidget(vib_group)

        self.setLayout(layout)

    def activate_device(self, device_id):
        amp = self.vib_amp_slider.value()
        hex_amp = f"{amp:02X}"
        self.send_command(f"W {device_id} {hex_amp}\n")
        # update indicator based on amp
        style = LED_ON_STYLE if amp > 0 else LED_OFF_STYLE
        self.indicators[device_id-1].setStyleSheet(style)

    def activate_all(self):
        amp = self.vib_amp_slider.value()
        hex_amp = f"{amp:02X}"
        self.send_command(f"V {hex_amp}\n")
        # update all indicators
        style = LED_ON_STYLE if amp > 0 else LED_OFF_STYLE
        for led in self.indicators:
            led.setStyleSheet(style)

    def deactivate_all(self):
        # send zero amplitude to all
        self.send_command("V 00\n")
        # clear all lights
        for led in self.indicators:
            led.setStyleSheet(LED_OFF_STYLE)

    def send_command(self, cmd):
        if getattr(self, 'serial_conn', None) and self.serial_conn.is_open:
            self.serial_conn.write(cmd.encode('utf-8'))
        else:
            print(f"[UI-only] Would send: {cmd.strip()}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setFont(QFont("Arial", 12))
    window = MotorVibrationControlUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
