import sys
import serial
import time
import math
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QComboBox, QSlider, QGroupBox
)
from PyQt5.QtCore import Qt

class VibrationGUI(QWidget):
    def __init__(self, port='COM7', baud=115200):
        super().__init__()
        self.setWindowTitle("DA7281 Vibration Control")
        
        # Attempt to open serial port
        try:
            self.ser = serial.Serial(port, baud, timeout=1)
            time.sleep(2)  # wait for Arduino reset
        except serial.SerialException:
            print(f"Could not open serial port {port}")
            self.ser = None

        mainLayout = QVBoxLayout()
        self.setLayout(mainLayout)
        
        # Address selection
        addrLayout = QHBoxLayout()
        addrLabel = QLabel("Haptic Driver Address:")
        self.addrCombo = QComboBox()
        self.addrCombo.addItem("48")
        self.addrCombo.addItem("49")
        addrLayout.addWidget(addrLabel)
        addrLayout.addWidget(self.addrCombo)
        mainLayout.addLayout(addrLayout)
        
        # One-shot buzz buttons: Exponential Decay and Short Buzz
        buzzButtonLayout = QHBoxLayout()
        self.expDecayButton = QPushButton("Exponential Decay Buzz")
        self.expDecayButton.clicked.connect(self.onExpDecayBuzz)
        self.shortBuzzButton = QPushButton("Short Buzz")
        self.shortBuzzButton.clicked.connect(self.onShortBuzz)
        buzzButtonLayout.addWidget(self.expDecayButton)
        buzzButtonLayout.addWidget(self.shortBuzzButton)
        mainLayout.addLayout(buzzButtonLayout)
        
        # Constant Buzz group: amplitude slider and start/stop buttons
        constantGroup = QGroupBox("Constant Buzz")
        constantLayout = QVBoxLayout()
        constantGroup.setLayout(constantLayout)
        
        sliderLayout = QHBoxLayout()
        self.ampLabel = QLabel("Amplitude:")
        self.ampSlider = QSlider(Qt.Horizontal)
        self.ampSlider.setMinimum(0)
        self.ampSlider.setMaximum(255)
        self.ampSlider.setValue(128)
        self.ampSlider.valueChanged.connect(self.onAmpSliderChanged)
        self.ampValueLabel = QLabel("128")
        sliderLayout.addWidget(self.ampLabel)
        sliderLayout.addWidget(self.ampSlider)
        sliderLayout.addWidget(self.ampValueLabel)
        constantLayout.addLayout(sliderLayout)
        
        buttonLayout = QHBoxLayout()
        self.startButton = QPushButton("Start Constant Buzz")
        self.startButton.clicked.connect(self.onStartConstantBuzz)
        self.stopButton = QPushButton("Stop Constant Buzz")
        self.stopButton.clicked.connect(self.onStopConstantBuzz)
        buttonLayout.addWidget(self.startButton)
        buttonLayout.addWidget(self.stopButton)
        constantLayout.addLayout(buttonLayout)
        
        mainLayout.addWidget(constantGroup)
        
    def onAmpSliderChanged(self, value):
        self.ampValueLabel.setText(str(value))
        
    def onExpDecayBuzz(self):
        # Generate an exponential decay waveform: 16 samples, 50ms per sample.
        sampleCount = 16
        timeStep = 50
        samples = self.generateExpDecay(sampleCount)
        sampleStr = " ".join(f"{sample:02X}" for sample in samples)
        address_str = self.addrCombo.currentText().strip()
        cmd = f"W {address_str} {sampleCount} {timeStep} {sampleStr}\n"
        self.sendCommand(cmd)
        
    def onShortBuzz(self):
        # Short buzz: a quick burst defined as 4 samples, 20ms per sample, e.g., [00, FF, FF, 00].
        sampleCount = 4
        timeStep = 20
        samples = [0x00, 0xFF, 0xFF, 0x00]
        sampleStr = " ".join(f"{sample:02X}" for sample in samples)
        address_str = self.addrCombo.currentText().strip()
        cmd = f"W {address_str} {sampleCount} {timeStep} {sampleStr}\n"
        self.sendCommand(cmd)
        
    def onStartConstantBuzz(self):
        # Constant buzz command: "C <addr> <amplitude>"
        address_str = self.addrCombo.currentText().strip()
        amplitude = self.ampSlider.value()
        cmd = f"C {address_str} {amplitude}\n"
        self.sendCommand(cmd)
        
    def onStopConstantBuzz(self):
        # Stop constant buzz by sending amplitude 0.
        address_str = self.addrCombo.currentText().strip()
        cmd = f"C {address_str} 0\n"
        self.sendCommand(cmd)
        
    def generateExpDecay(self, length=16):
        samples = []
        tau = length / 3.0  # Decay constant: adjust as needed
        for i in range(length):
            amp = int(255 * math.exp(-i / tau))
            samples.append(amp)
        return samples
        
    def sendCommand(self, cmd):
        if self.ser is None:
            print("Serial port not open!")
            return
        print("Sending:", cmd.strip())
        self.ser.write(cmd.encode('utf-8'))
        response = self.ser.readline().decode('utf-8').strip()
        print("Arduino response:", response)
        
    def closeEvent(self, event):
        if self.ser:
            self.ser.close()
        event.accept()

def main():
    app = QApplication(sys.argv)
    gui = VibrationGUI(port='COM7', baud=115200)
    gui.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
