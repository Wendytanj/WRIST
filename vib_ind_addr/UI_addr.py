import sys
import serial
import time
import math
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QCheckBox, QSlider, QGroupBox
)
from PyQt5.QtCore import Qt

class VibrationGUI(QWidget):
    def __init__(self, port='COM7', baud=115200):
        super().__init__()
        self.setWindowTitle("DA7281 Vibration Control")

        try:
            self.ser = serial.Serial(port, baud, timeout=1)
            time.sleep(2)  # Wait for MCU reset
        except serial.SerialException:
            print(f"Could not open serial port {port}")
            self.ser = None
        
        mainLayout = QVBoxLayout()
        self.setLayout(mainLayout)
        
        # Address selection group with check boxes
        addrGroup = QGroupBox("Select Addresses")
        addrLayout = QHBoxLayout()
        self.cb48 = QCheckBox("0x48")
        self.cb49 = QCheckBox("0x49")
        self.cb4A = QCheckBox("0x4A")
        # Default them to True, or as desired
        self.cb48.setChecked(True)
        self.cb49.setChecked(True)
        self.cb4A.setChecked(True)
        addrLayout.addWidget(self.cb48)
        addrLayout.addWidget(self.cb49)
        addrLayout.addWidget(self.cb4A)
        addrGroup.setLayout(addrLayout)
        mainLayout.addWidget(addrGroup)
        
        # Row 1: Waveform commands
        buttonLayout1 = QHBoxLayout()
        self.expDecayButton = QPushButton("Exponential Decay Buzz")
        self.expDecayButton.clicked.connect(self.onExpDecayBuzz)
        self.shortBuzzButton = QPushButton("Short Buzz")
        self.shortBuzzButton.clicked.connect(self.onShortBuzz)
        buttonLayout1.addWidget(self.expDecayButton)
        buttonLayout1.addWidget(self.shortBuzzButton)
        mainLayout.addLayout(buttonLayout1)
        
        # Row 2: More waveform commands
        buttonLayout2 = QHBoxLayout()
        self.squareBuzzButton = QPushButton("Square Wave Buzz")
        self.squareBuzzButton.clicked.connect(self.onSquareBuzz)
        self.sawBuzzButton = QPushButton("Saw Wave Buzz")
        self.sawBuzzButton.clicked.connect(self.onSawBuzz)
        buttonLayout2.addWidget(self.squareBuzzButton)
        buttonLayout2.addWidget(self.sawBuzzButton)
        mainLayout.addLayout(buttonLayout2)
        
        # Constant Buzz group
        constantGroup = QGroupBox("Constant Buzz (All Selected Addresses)")
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
        
        btnLayout = QHBoxLayout()
        self.startButton = QPushButton("Start Constant Buzz")
        self.startButton.clicked.connect(self.onStartConstBuzz)
        self.stopButton = QPushButton("Stop Constant Buzz")
        self.stopButton.clicked.connect(self.onStopConstBuzz)
        btnLayout.addWidget(self.startButton)
        btnLayout.addWidget(self.stopButton)
        constantLayout.addLayout(btnLayout)
        
        mainLayout.addWidget(constantGroup)
        
        # Label for timing info
        self.timeLabel = QLabel("Last Waveform Time: (none)")
        mainLayout.addWidget(self.timeLabel)

    # ---------------------------
    # Waveform Generators
    # ---------------------------
    def generateExpDecay(self, length=20):
        samples = []
        tau = length / 3.0
        for i in range(length):
            amp = int(255 * math.exp(-i / tau))
            samples.append(amp)
        return samples
    
    def generateSquareWave(self, length=16):
        samples = []
        half = length // 2
        for i in range(length):
            samples.append(0 if i < half else 255)
        return samples
    
    def generateSawWave(self, length=16):
        samples = []
        for i in range(length):
            val = int((i / (length - 1)) * 255)
            samples.append(val)
        return samples

    # ---------------------------
    # Waveform Commands
    # ---------------------------
    def onExpDecayBuzz(self):
        sampleCount = 20
        timeStep = 10
        samples = self.generateExpDecay(sampleCount)
        self.sendWaveCommand(sampleCount, timeStep, samples)

    def onShortBuzz(self):
        sampleCount = 1
        timeStep = 20
        samples = [0xFF]
        self.sendWaveCommand(sampleCount, timeStep, samples)

    def onSquareBuzz(self):
        sampleCount = 16
        timeStep = 50
        samples = self.generateSquareWave(sampleCount)
        self.sendWaveCommand(sampleCount, timeStep, samples)

    def onSawBuzz(self):
        sampleCount = 16
        timeStep = 50
        samples = self.generateSawWave(sampleCount)
        self.sendWaveCommand(sampleCount, timeStep, samples)

    def sendWaveCommand(self, sampleCount, timeStep, samples):
        """Send wave command with the format:
           W <num_addr> <addr1> <addr2> ... <waveLen> <stepMs> <amp0> ... <ampN-1>
        """
        addresses = self.getCheckedAddresses()
        if not addresses:
            print("No addresses selected!")
            return
        
        sampleStr = " ".join(f"{amp:02X}" for amp in samples)
        numAddr = len(addresses)
        cmd = f"W {numAddr} {' '.join(addresses)} {sampleCount} {timeStep} {sampleStr}\n"
        self.sendCommand(cmd, wait_for_response=True)

    # ---------------------------
    # Constant Buzz
    # ---------------------------
    def onAmpSliderChanged(self, value):
        self.ampValueLabel.setText(str(value))
        # Real-time updates (no blocking)
        self.sendConstBuzzCommand(value, wait_for_response=False)

    def onStartConstBuzz(self):
        amp = self.ampSlider.value()
        self.sendConstBuzzCommand(amp, wait_for_response=True)

    def onStopConstBuzz(self):
        self.sendConstBuzzCommand(0, wait_for_response=True)

    def sendConstBuzzCommand(self, amplitude, wait_for_response=True):
        """C <num_addr> <addr1> <addr2> ... <amplitude>"""
        addresses = self.getCheckedAddresses()
        if not addresses:
            print("No addresses selected!")
            return
        numAddr = len(addresses)
        cmd = f"C {numAddr} {' '.join(addresses)} {amplitude}\n"
        self.sendCommand(cmd, wait_for_response=wait_for_response)

    def getCheckedAddresses(self):
        """Return a list of addresses in hex (without '0x') for each checked box."""
        addresses = []
        if self.cb48.isChecked():
            addresses.append("48")
        if self.cb49.isChecked():
            addresses.append("49")
        if self.cb4A.isChecked():
            addresses.append("4A")
        return addresses

    # ---------------------------
    # Serial Communication
    # ---------------------------
    def sendCommand(self, cmd, wait_for_response=True):
        if self.ser is None:
            print("Serial port not open!")
            return
        print("Sending:", cmd.strip())
        self.ser.write(cmd.encode('utf-8'))
        if wait_for_response:
            response = self.ser.readline().decode('utf-8').strip()
            print("Arduino response:", response)
            if response.startswith("DONE "):
                # e.g. "DONE 123"
                ms_str = response[5:].strip()
                self.timeLabel.setText(f"Last Waveform Time: {ms_str} ms")
            else:
                self.timeLabel.setText("No timing info received.")

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
