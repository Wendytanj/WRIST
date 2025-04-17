import sys
import serial
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QSlider, QGroupBox, QMessageBox
)
from PyQt5.QtGui import QFont, QIcon
from PyQt5.QtCore import Qt

SERIAL_PORT = 'COM10'
BAUD_RATE = 115200

# Styles for indicators
LED_ON_STYLE = "background-color: #0f0; border-radius: 12px;"
LED_OFF_STYLE = "background-color: #555; border-radius: 12px;"

class MotorVibrationControlUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.setWindowTitle("Motor & Vibration Control UI")
        self.resize(500, 800)
        self.model_sent = False

        # Attempt serial connection
        try:
            self.serial_conn = serial.Serial(port, baud, timeout=1)
        except Exception as e:
            self.serial_conn = None
            QMessageBox.warning(
                self, "Serial Connection",
                f"Could not open {port}: {e}\nRunning in UI-only mode."
            )

        layout = QVBoxLayout()

        # Motor control group
        motor_group = QGroupBox("Motor Control")
        motor_layout = QVBoxLayout()

        # Speed slider + Set Speed button (S command)
        self.speed_label = QLabel("Speed: 80")
        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setRange(0, 255)
        self.speed_slider.setValue(80)
        self.speed_slider.setTickInterval(5)
        self.speed_slider.valueChanged.connect(
            lambda v: self.speed_label.setText(f"Speed: {v}")
        )
        btn_set_speed = QPushButton("Set Speed (S)")
        btn_set_speed.pressed.connect(
            lambda: self.send_command(f"S {self.speed_slider.value()}\n")
        )
        motor_layout.addWidget(self.speed_label)
        motor_layout.addWidget(self.speed_slider)
        motor_layout.addWidget(btn_set_speed)

        # Angle slider
        self.angle_label = QLabel("Angle: 90°")
        self.angle_slider = QSlider(Qt.Horizontal)
        self.angle_slider.setRange(0, 360)
        self.angle_slider.setValue(90)
        self.angle_slider.setTickInterval(10)
        self.angle_slider.valueChanged.connect(
            lambda v: self.angle_label.setText(f"Angle: {v}°")
        )
        motor_layout.addWidget(self.angle_label)
        motor_layout.addWidget(self.angle_slider)

        # Command buttons: Model (M), Unmodel (T), Stop (X)
        cmd_layout = QHBoxLayout()
        self.btn_model = QPushButton("Model (M)")
        self.btn_model.pressed.connect(self.send_model)
        self.btn_un = QPushButton("Unmodel (T)")
        self.btn_un.pressed.connect(self.send_unmodel)
        self.btn_stop = QPushButton("Stop (X)")
        self.btn_stop.pressed.connect(self.send_stop)
        for btn in [self.btn_model, self.btn_un, self.btn_stop]:
            btn.setCheckable(False)
            cmd_layout.addWidget(btn)
        motor_layout.addLayout(cmd_layout)

        # Rotate angle commands: A B
        rotate_layout = QHBoxLayout()
        btn_rotate_cw = QPushButton("Rotate Squeeze (A)")
        btn_rotate_cw.pressed.connect(
            lambda: self.send_command(f"A {self.angle_slider.value()}\n")
        )
        btn_rotate_un = QPushButton("Rotate Unsqueeze (B)")
        btn_rotate_un.pressed.connect(
            lambda: self.send_command(f"B {self.angle_slider.value()}\n")
        )
        rotate_layout.addWidget(btn_rotate_cw)
        rotate_layout.addWidget(btn_rotate_un)
        motor_layout.addLayout(rotate_layout)

        # Pressure P slider and button
        p_layout = QHBoxLayout()
        self.p_label = QLabel("Pressure: 10")
        self.p_slider = QSlider(Qt.Horizontal)
        self.p_slider.setRange(1, 10)
        self.p_slider.setValue(10)
        self.p_slider.setTickInterval(1)
        self.p_slider.valueChanged.connect(
            lambda v: self.p_label.setText(f"Pressure: {v}")
        )
        btn_pressure = QPushButton("Set Pressure (P)")
        btn_pressure.pressed.connect(self.send_pressure)
        p_layout.addWidget(self.p_label)
        p_layout.addWidget(self.p_slider)
        p_layout.addWidget(btn_pressure)
        motor_layout.addLayout(p_layout)

        motor_group.setLayout(motor_layout)
        layout.addWidget(motor_group)

        # Vibration control group
        vib_group = QGroupBox("Vibration Control")
        vib_layout = QVBoxLayout()

        # Amp slider
        self.vib_amp_label = QLabel("Amp: 0x46")
        self.vib_amp_slider = QSlider(Qt.Horizontal)
        self.vib_amp_slider.setRange(0, 255)
        self.vib_amp_slider.setValue(0x46)
        self.vib_amp_slider.setTickInterval(5)
        self.vib_amp_slider.valueChanged.connect(
            lambda v: self.vib_amp_label.setText(f"Amp: 0x{v:02X}")
        )
        vib_layout.addWidget(self.vib_amp_label)
        vib_layout.addWidget(self.vib_amp_slider)

        # Indicators
        ind_layout = QHBoxLayout()
        self.indicators = []
        for _ in range(5):
            led = QLabel()
            led.setFixedSize(16, 16)
            led.setStyleSheet(LED_OFF_STYLE)
            ind_layout.addWidget(led)
            self.indicators.append(led)
        vib_layout.addLayout(ind_layout)

        # Device buttons
        dev_layout = QHBoxLayout()
        for i in range(1, 6):
            b = QPushButton(str(i))
            b.pressed.connect(lambda i=i: self.activate_device(i))
            dev_layout.addWidget(b)
        vib_layout.addLayout(dev_layout)

        # All/Stop
        all_btn = QPushButton("All Vibrators (V)")
        all_btn.pressed.connect(self.activate_all)
        stop_btn = QPushButton("Vibrators Off (V 00)")
        stop_btn.pressed.connect(self.deactivate_all)
        vib_layout.addWidget(all_btn)
        vib_layout.addWidget(stop_btn)

        vib_group.setLayout(vib_layout)
        layout.addWidget(vib_group)

        self.setLayout(layout)

    def send_model(self):
        if not self.model_sent:
            self.send_command("M \n")
            self.model_sent = True
            self.btn_model.setEnabled(False)

    def send_unmodel(self):
        self.send_command("T \n")
        self.model_sent = False
        self.btn_model.setEnabled(True)

    def send_stop(self):
        self.send_command("X \n")
        self.model_sent = False
        self.btn_model.setEnabled(True)

    def send_pressure(self):
        v = self.p_slider.value()
        self.send_command(f"P {v}\n")

    def activate_device(self, idx):
        amp = self.vib_amp_slider.value()
        hex_amp = f"{amp:02X}"
        self.send_command(f"W {idx} {hex_amp}\n")
        style = LED_ON_STYLE if amp>0 else LED_OFF_STYLE
        self.indicators[idx-1].setStyleSheet(style)

    def activate_all(self):
        amp = self.vib_amp_slider.value()
        hex_amp = f"{amp:02X}"
        self.send_command(f"V {hex_amp}\n")
        style = LED_ON_STYLE if amp>0 else LED_OFF_STYLE
        for led in self.indicators:
            led.setStyleSheet(style)

    def deactivate_all(self):
        self.send_command("V 00\n")
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
