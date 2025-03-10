import sys
import serial
import time
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QComboBox
)

class VibrationGUI(QWidget):
    def __init__(self, port='COM4', baud=115200):
        super().__init__()
        self.setWindowTitle("DA7281 Vibration Control")

        # Attempt to open serial
        try:
            self.ser = serial.Serial(port, baud, timeout=1)
            time.sleep(2)  # wait for Arduino reset
        except serial.SerialException:
            print(f"Could not open serial port {port}")
            self.ser = None

        # Create UI
        layout = QVBoxLayout()
        self.setLayout(layout)

        # Address selection
        self.addrCombo = QComboBox()
        self.addrCombo.addItem("0x48")
        self.addrCombo.addItem("0x49")
        layout.addWidget(QLabel("Select Haptic Driver Address:"))
        layout.addWidget(self.addrCombo)

        # Buzz button
        self.buzzButton = QPushButton("Buzz!")
        self.buzzButton.clicked.connect(self.onBuzzClicked)
        layout.addWidget(self.buzzButton)

    def onBuzzClicked(self):
        if self.ser is None:
            print("Serial port not open!")
            return

        address_str = self.addrCombo.currentText()  # e.g. "0x48"
        cmd = f"VIB {address_str}\n"
        print("Sending:", cmd.strip())

        # Write to Arduino
        self.ser.write(cmd.encode('utf-8'))
        
        response = self.ser.readline().decode('utf-8').strip()
        print("Arduino response:", response)

    def closeEvent(self, event):
        # Cleanup
        if self.ser:
            self.ser.close()
        event.accept()

def main():
    app = QApplication(sys.argv)
    gui = VibrationGUI(port='COM4', baud=115200)
    gui.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
