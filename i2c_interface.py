import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLabel, QSlider, QPushButton, QHBoxLayout
from PyQt5.QtCore import Qt

class WaveformController(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        # Initialize serial communication
        self.ser = serial.Serial('COM3', 9600)  # Adjust 'COM3' to your port

    def initUI(self):
        self.setWindowTitle('DA7280 Waveform Controller')

        layout = QVBoxLayout()

        # Frequency Slider
        self.freq_slider = QSlider(Qt.Horizontal)
        self.freq_slider.setRange(0, 255)
        self.freq_slider.setValue(128)
        self.freq_label = QLabel('Frequency: 128')
        self.freq_slider.valueChanged.connect(self.update_freq_label)
        layout.addWidget(self.freq_label)
        layout.addWidget(self.freq_slider)

        # Amplitude Slider
        self.amp_slider = QSlider(Qt.Horizontal)
        self.amp_slider.setRange(0, 255)
        self.amp_slider.setValue(128)
        self.amp_label = QLabel('Amplitude: 128')
        self.amp_slider.valueChanged.connect(self.update_amp_label)
        layout.addWidget(self.amp_label)
        layout.addWidget(self.amp_slider)

        # Duration Slider
        self.dur_slider = QSlider(Qt.Horizontal)
        self.dur_slider.setRange(0, 1000)
        self.dur_slider.setValue(500)
        self.dur_label = QLabel('Duration: 500 ms')
        self.dur_slider.valueChanged.connect(self.update_dur_label)
        layout.addWidget(self.dur_label)
        layout.addWidget(self.dur_slider)

        # Send Button
        send_button = QPushButton('Send Waveform')
        send_button.clicked.connect(self.send_waveform)
        layout.addWidget(send_button)

        self.setLayout(layout)

    def update_freq_label(self, value):
        self.freq_label.setText(f'Frequency: {value}')

    def update_amp_label(self, value):
        self.amp_label.setText(f'Amplitude: {value}')

    def update_dur_label(self, value):
        self.dur_label.setText(f'Duration: {value} ms')

    def send_waveform(self):
        freq = self.freq_slider.value()
        amp = self.amp_slider.value()
        dur = self.dur_slider.value()
        waveform_data = f'FREQ:{freq},AMP:{amp},DUR:{dur}\n'
        self.ser.write(waveform_data.encode())

if __name__ == '__main__':
    app = QApplication(sys.argv)
    controller = WaveformController()
    controller.show()
    sys.exit(app.exec_())
