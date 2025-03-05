import sys
import serial
import serial.tools.list_ports
import re
from PyQt5 import QtWidgets, QtCore
from PyQt5.QtCore import Qt, QThread, pyqtSignal
import pyqtgraph as pg
from collections import deque
from syntacts import *
from time import sleep
import csv

# Serial Thread to handle serial communication
class SerialThread(QThread):
    data_received = pyqtSignal(dict)
    error = pyqtSignal(str)

    def __init__(self, port, baudrate=115200):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self._is_running = True
        self.ser = None

    def run(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
            while self._is_running:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8', errors='replace').strip()
                    if line:
                        data = self.parse_line(line)
                        if data:
                            self.data_received.emit(data)
        except serial.SerialException as e:
            self.error.emit(str(e))
        finally:
            if self.ser and self.ser.is_open:
                self.ser.close()

    def stop(self):
        self._is_running = False
        self.wait()

    def parse_line(self, line):
        """
        Parses a line of serial data and extracts sensor values.
        Expected format:
        Accel X: -0.023928,Accel Y: 9.810574,Accel Z: 0.550349,Gyro X: -0.015882,Gyro Y: -0.070860,Gyro Z: -0.037874
        """
        pattern = (r'Accel X:\s*([-+]?\d*\.\d+|\d+),'
                   r'Accel Y:\s*([-+]?\d*\.\d+|\d+),'
                   r'Accel Z:\s*([-+]?\d*\.\d+|\d+),'
                   r'Gyro X:\s*([-+]?\d*\.\d+|\d+),'
                   r'Gyro Y:\s*([-+]?\d*\.\d+|\d+),'
                   r'Gyro Z:\s*([-+]?\d*\.\d+|\d+)')
        match = re.match(pattern, line)
        if match:
            accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z = map(float, match.groups())
            return {
                'accel_x': accel_x,
                'accel_y': accel_y,
                'accel_z': accel_z,
                'gyro_x': gyro_x,
                'gyro_y': gyro_y,
                'gyro_z': gyro_z
            }
        else:
            # Optionally emit an error or ignore malformed lines
            return None

# Syntacts Thread to handle signal playback
class SyntactsThread(QThread):
    playback_finished = pyqtSignal()
    playback_error = pyqtSignal(str)
    frequency_changed = pyqtSignal(int)  # Signal to emit current frequency

    def __init__(self, parent=None):
        super().__init__(parent)
        self._is_running = True

    def run(self):
        try:
            s = Session()
            s.open()

            for freq in range(20, 340, 10):
                if not self._is_running:
                    break
                signal = Sine(freq) * Envelope(2, 1)  # Duration 2s, Attack 1s
                s.play(0, signal)
                self.frequency_changed.emit(freq)  # Emit current frequency
                sleep(signal.length)  # Wait for signal to finish

            s.close()

            if self._is_running:
                self.playback_finished.emit()
        except Exception as e:
            self.playback_error.emit(str(e))

    def stop(self):
        self._is_running = False
        self.wait()

# Main Window of the GUI
class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Arduino Sensor Data Viewer")
        self.setGeometry(100, 100, 800, 600)  # Adjusted window size for better layout

        # Central widget
        self.central_widget = QtWidgets.QWidget()
        self.setCentralWidget(self.central_widget)

        # Layouts
        self.main_layout = QtWidgets.QVBoxLayout(self.central_widget)
        self.controls_layout = QtWidgets.QHBoxLayout()
        self.data_layout = QtWidgets.QHBoxLayout()
        self.plots_layout = QtWidgets.QHBoxLayout()

        self.main_layout.addLayout(self.controls_layout)
        self.main_layout.addLayout(self.data_layout)
        self.main_layout.addLayout(self.plots_layout)

        # Controls
        self.port_label = QtWidgets.QLabel("Serial Port:")
        self.port_combo = QtWidgets.QComboBox()
        self.refresh_button = QtWidgets.QPushButton("Refresh")
        self.connect_button = QtWidgets.QPushButton("Connect")
        self.disconnect_button = QtWidgets.QPushButton("Disconnect")
        self.disconnect_button.setEnabled(False)

        self.play_button = QtWidgets.QPushButton("Play Sequence")
        self.play_button.setEnabled(True)  # Enabled by default

        self.controls_layout.addWidget(self.port_label)
        self.controls_layout.addWidget(self.port_combo)
        self.controls_layout.addWidget(self.refresh_button)
        self.controls_layout.addWidget(self.connect_button)
        self.controls_layout.addWidget(self.disconnect_button)
        self.controls_layout.addWidget(self.play_button)  # Add Play button

        # Data Display (Only Accelerometer Z-Axis)
        self.accel_z_label = QtWidgets.QLabel("Accel Z: N/A")
        self.accel_z_label.setAlignment(Qt.AlignLeft)
        self.data_layout.addWidget(self.accel_z_label)

        # Data Buffers for Plotting (Only Accelerometer Z-Axis)
        self.buffer_size = 100  # Number of points to display

        # Plots (Only Accelerometer Z-Axis Over Time)
        pg.setConfigOptions(antialias=True)

        self.accel_z_plot = pg.PlotWidget(title="Accelerometer Z-Axis Over Time")
        self.accel_z_plot.setBackground('w')  # Set background to white
        self.accel_z_plot.addLegend()
        self.accel_z_plot.setLabel('left', 'Acceleration', units='m/s²', color='k')  # Set label color to black
        self.accel_z_plot.setLabel('bottom', 'Time (s)', color='k')  # Set label color to black

        # Customize Grid Lines
        self.accel_z_plot.showGrid(x=True, y=True, alpha=0.3)
        self.accel_z_plot.setXRange(0, 100, padding=0)  # Initial range; will adjust dynamically
        self.accel_z_curve = self.accel_z_plot.plot(pen=pg.mkPen(color='b', width=2), name='Accel Z Over Time')  # Blue line

        self.plots_layout.addWidget(self.accel_z_plot)

        # Buffer for time plot
        self.time_data = deque([0], maxlen=1000)  # Adjust maxlen as needed
        self.accel_z_time_data = deque([0], maxlen=1000)

        # Logging Setup
        self.log_file_handle = None
        self.log_writer = None
        self.current_frequency = None
        self.start_time = None  # To track elapsed time

        # Connect Signals
        self.refresh_button.clicked.connect(self.refresh_ports)
        self.connect_button.clicked.connect(self.connect_serial)
        self.disconnect_button.clicked.connect(self.disconnect_serial)
        self.play_button.clicked.connect(self.play_sequence)

        # Initialize
        self.refresh_ports()

        # Serial and Play Threads Placeholders
        self.serial_thread = None
        self.play_thread = None

    def refresh_ports(self):
        """Refresh the list of available serial ports."""
        self.port_combo.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(port.device)

    def connect_serial(self):
        """Connect to the selected serial port."""
        port = self.port_combo.currentText()
        if port:
            self.serial_thread = SerialThread(port)
            self.serial_thread.data_received.connect(self.update_data)
            self.serial_thread.error.connect(self.handle_error)
            self.serial_thread.start()
            self.connect_button.setEnabled(False)
            self.disconnect_button.setEnabled(True)
            self.refresh_button.setEnabled(False)
        else:
            QtWidgets.QMessageBox.warning(self, "No Port Selected", "Please select a serial port.")

    def disconnect_serial(self):
        """Disconnect from the serial port."""
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread = None
            self.connect_button.setEnabled(True)
            self.disconnect_button.setEnabled(False)
            self.refresh_button.setEnabled(True)

    def handle_error(self, error_message):
        """Handle errors from the serial thread."""
        QtWidgets.QMessageBox.critical(self, "Serial Error", error_message)
        self.disconnect_serial()

    def update_data(self, data):
        """Update the GUI with new sensor data and log it."""
        # Update Labels (Only Accelerometer Z-Axis)
        self.accel_z_label.setText(f"Accel Z: {data['accel_z']:.6f} m/s²")

        # Update Plot Data (Only Accelerometer Z-Axis Over Time)
        if self.start_time is None:
            self.start_time = QtCore.QTime.currentTime()
        elapsed = self.start_time.msecsTo(QtCore.QTime.currentTime()) / 1000.0  # seconds
        self.time_data.append(elapsed)
        self.accel_z_time_data.append(data['accel_z'])

        self.accel_z_curve.setData(list(self.time_data), list(self.accel_z_time_data))
        self.accel_z_plot.setXRange(max(0, elapsed - 10), elapsed + 1)  # Show last 10 seconds

        # Logging
        if self.current_frequency is not None and self.log_writer is not None:
            self.log_writer.writerow({'frequency': self.current_frequency, 'accel_z': data['accel_z']})

    def play_sequence(self):
        """Handle the Play Sequence button click."""
        if not self.play_thread or not self.play_thread.isRunning():
            self.play_thread = SyntactsThread()
            self.play_thread.playback_finished.connect(self.playback_finished)
            self.play_thread.playback_error.connect(self.playback_error)
            self.play_thread.frequency_changed.connect(self.update_frequency)
            self.play_thread.start()
            self.play_button.setEnabled(False)

            # Open CSV Logging
            try:
                self.log_file_handle = open('sensor_data_log.csv', 'w', newline='')
                self.log_writer = csv.DictWriter(self.log_file_handle, fieldnames=['frequency', 'accel_z'])
                self.log_writer.writeheader()
                self.start_time = None  # Reset start time for time plot
                self.time_data.clear()
                self.accel_z_time_data.clear()
                self.accel_z_curve.clear()
            except Exception as e:
                QtWidgets.QMessageBox.critical(self, "Logging Error", f"Failed to open log file: {e}")
                self.log_writer = None
        else:
            QtWidgets.QMessageBox.information(self, "Playback", "Playback is already running.")

    def update_frequency(self, freq):
        """Update the current frequency being played."""
        self.current_frequency = freq
        # (Progress Bar removed, so no updates here)

    def playback_finished(self):
        """Handle the playback completion."""
        QtWidgets.QMessageBox.information(self, "Playback", "Sequence playback finished.")
        self.play_button.setEnabled(True)
        self.current_frequency = None

        # Close CSV Logging
        if self.log_file_handle:
            self.log_file_handle.close()
            self.log_file_handle = None
            self.log_writer = None

    def playback_error(self, error_message):
        """Handle playback errors."""
        QtWidgets.QMessageBox.critical(self, "Playback Error", error_message)
        self.play_button.setEnabled(True)
        self.current_frequency = None

        # Close CSV Logging
        if self.log_file_handle:
            self.log_file_handle.close()
            self.log_file_handle = None
            self.log_writer = None

    def closeEvent(self, event):
        """Handle the window close event to ensure all threads are stopped."""
        if self.serial_thread:
            self.serial_thread.stop()
        if self.play_thread and self.play_thread.isRunning():
            self.play_thread.stop()
        if self.log_file_handle:
            self.log_file_handle.close()
        event.accept()

# Main function to run the application
def main():
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
