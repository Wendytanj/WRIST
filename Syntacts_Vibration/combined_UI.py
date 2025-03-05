import sys
import time
import serial

from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QGroupBox, QLabel
)
from syntacts import *  # Make sure the syntacts library is installed and available
from PyQt5.QtGui import QFont

# -----------------------------
# Configuration
# -----------------------------
SERIAL_PORT = 'COM9'
BAUD_RATE = 9600

# Define the channels for your LRAs
CHANNELS = [1,0,5,2,4]

# -----------------------------
# Motor + Vibration UI
# -----------------------------
class MotorVibrationUI(QWidget):
    def __init__(self, port, baud):
        super().__init__()
        self.setWindowTitle("WRIST UI")

        # ---- Serial Setup for Motor Control ----
        self.serial_conn = serial.Serial(port, baud, timeout=1)

        # ---- Syntacts Setup for Vibration ----
        self.session = Session()
        self.session.open()

        # Main layout
        main_layout = QVBoxLayout()

        # 1) Motor Control Group
        motor_group = QGroupBox("Motor Control")
        motor_layout = QHBoxLayout()

        self.btn_tighten = QPushButton("Tighten")
        self.btn_tighten.pressed.connect(self.start_tighten)
        self.btn_tighten.released.connect(self.stop_motors)

        self.btn_release = QPushButton("Release")
        self.btn_release.pressed.connect(self.start_release)
        self.btn_release.released.connect(self.stop_motors)

        motor_layout.addWidget(self.btn_tighten)
        motor_layout.addWidget(self.btn_release)
        motor_group.setLayout(motor_layout)

        # 2) Vibration Control Group
        vib_group = QGroupBox("Vibration Control")
        vib_layout = QVBoxLayout()

        # Row 1: Buttons for overall patterns
        pattern_buttons_layout = QHBoxLayout()

        self.btn_play_pattern = QPushButton("Play Pattern")
        self.btn_play_pattern.clicked.connect(self.play_pattern)

        self.btn_play_individual_lra = QPushButton("Play Individual (0.5s)")
        self.btn_play_individual_lra.clicked.connect(self.play_individual_lra)

        self.btn_play_all_once = QPushButton("Play All (0.5s)")
        self.btn_play_all_once.clicked.connect(self.play_all_once)

        pattern_buttons_layout.addWidget(self.btn_play_pattern)
        pattern_buttons_layout.addWidget(self.btn_play_individual_lra)
        pattern_buttons_layout.addWidget(self.btn_play_all_once)

        vib_layout.addLayout(pattern_buttons_layout)

        # Row 2: Individual channel buttons
        channel_buttons_layout = QHBoxLayout()
        for ch in CHANNELS:
            btn = QPushButton(f"Channel {ch}")
            btn.clicked.connect(lambda _, c=ch: self.play_single_channel(c, 0.5))
            channel_buttons_layout.addWidget(btn)

        vib_layout.addLayout(channel_buttons_layout)

        vib_group.setLayout(vib_layout)

        # Add groups to main layout
        main_layout.addWidget(motor_group)
        main_layout.addWidget(vib_group)

        self.setLayout(main_layout)

    # -----------------------------
    # Motor Control Methods
    # -----------------------------
    def start_tighten(self):
        """ Send 'T' command (Tighten) """
        self.send_command("T\n")

    def start_release(self):
        """ Send 'R' command (Release) """
        self.send_command("R\n")

    def stop_motors(self):
        """ Send 'X' command (Stop) """
        self.send_command("X\n")

    def send_command(self, command):
        """ Low-level command sending over serial """
        if self.serial_conn.is_open:
            self.serial_conn.write(command.encode('utf-8'))

    # -----------------------------
    # Vibration Methods
    # -----------------------------
    def create_tactor_signal(self, frequency, duration):
        """Creates a single Sine signal with an Exponential Decay, then envelopes it."""
        amplitude = 1.0
        decay_rate = 6.9
        return Sine(frequency) * ExponentialDecay(amplitude, decay_rate) * Envelope(duration)

    def play_individual(self, duration):
        """
        Plays a pulse on each channel individually with
        an exponential decay sine wave for 'duration' seconds.
        """
        for ch in CHANNELS:
            signal = self.create_tactor_signal(170, duration)
            self.session.play(ch, signal)
            time.sleep(signal.length)  # Blocks UI for demonstration
            self.session.stop(ch)

    def play_individual_lra(self):
        """Plays each LRA for 0.5s in sequence."""
        self.play_individual(0.5)

    def play_all_once(self):
        """Plays all channels simultaneously for 0.5s."""
        duration = 0.5
        # Create a signal for each channel and play
        signal = self.create_tactor_signal(170, duration)
        for ch in CHANNELS:
            self.session.play(ch, signal)
        time.sleep(signal.length)  # Blocks UI
        # Stop them
        for ch in CHANNELS:
            self.session.stop(ch)

    def play_single_channel(self, channel, duration):
        """
        Plays a single channel for a given duration (e.g., 0.5s).
        """
        signal = self.create_tactor_signal(170, duration)
        self.session.play(channel, signal)
        time.sleep(signal.length)  # Blocks UI
        self.session.stop(channel)

    def play_pattern(self):
        """
        Reproduces your original code snippet:
         1) Plays 'play_individual()' at durations 1.0, 0.5, 0.2, 0.1
         2) Plays a custom pattern on all channels
        """
        print("Playing pattern: individual pulses of descending duration.")
        self.play_individual(1.0)
        self.play_individual(0.5)
        self.play_individual(0.2)
        self.play_individual(0.1)

        print("Playing custom pattern (Sine(170) * Envelope(0.75)) on all channels.")
        signal1 = Sine(170) * Envelope(0.75)  # length = 0.75
        sequence = Sequence()
        sequence << signal1

        for ch in CHANNELS:
            self.session.play(ch, sequence)

        # Wait for that sequence to finish
        time.sleep(sequence.length + 0.1)
        # We do NOT close the session here so user can continue playing

    # -----------------------------
    # Cleanup
    # -----------------------------
    def closeEvent(self, event):
        """Close out the Syntacts session and serial gracefully."""
        if self.serial_conn.is_open:
            self.serial_conn.close()
        self.session.close()
        super().closeEvent(event)

# -----------------------------
# Main
# -----------------------------
if __name__ == "__main__":
    app = QApplication(sys.argv)
    font = QFont("Arial", 12) 
    app.setFont(font)
    window = MotorVibrationUI(SERIAL_PORT, BAUD_RATE)
    window.show()
    sys.exit(app.exec_())
