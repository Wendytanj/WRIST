import numpy as np
import matplotlib.pyplot as plt

data = np.loadtxt('data.txt')

force = data[:, 0:3]   
torque = data[:, 3:6]  

num_samples = data.shape[0]
sampling_rate = 60  # Hz
time = np.arange(num_samples) / sampling_rate  

start_index = int(33 * sampling_rate)
end_index = int(34 * sampling_rate)

# Calculate the baseline (mean over 33-34 seconds)
baseline_force = np.mean(force[start_index:end_index, :], axis=0)
baseline_torque = np.mean(torque[start_index:end_index, :], axis=0)

print("Baseline force: ", baseline_force)

# Subtract the baseline from all data
adjusted_force = force - baseline_force
adjusted_torque = torque - baseline_torque

# Extract Force Z data for range calculation (after baseline adjustment)
force_z = adjusted_force[:, 2]
max_force_z = np.max(force_z)
min_force_z = np.min(force_z)
range_force_z = max_force_z - min_force_z

# Print the range to the terminal
print("The range (max - min) of adjusted Force Z data is:", range_force_z)

# Plot Adjusted Force Components
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(time, adjusted_force[:, 0], label='Force X (Adjusted)')
plt.plot(time, adjusted_force[:, 1], label='Force Y (Adjusted)')
plt.plot(time, adjusted_force[:, 2], label='Force Z (Adjusted)')
plt.title('Adjusted Force Components Over Time')
plt.xlabel('Time (s)')
plt.ylabel('Force (Adjusted)')
plt.legend()
plt.grid(True)

# Plot Adjusted Torque Components
plt.subplot(2, 1, 2)
plt.plot(time, adjusted_torque[:, 0], label='Torque X (Adjusted)')
plt.plot(time, adjusted_torque[:, 1], label='Torque Y (Adjusted)')
plt.plot(time, adjusted_torque[:, 2], label='Torque Z (Adjusted)')
plt.title('Adjusted Torque Components Over Time')
plt.xlabel('Time (s)')
plt.ylabel('Torque (Adjusted)')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()
