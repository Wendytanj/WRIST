import pandas as pd
import matplotlib.pyplot as plt
import os

def plot_max_vibration(csv_files, labels, output_image=None):
    """
    Reads multiple CSV files, computes the maximum absolute accel_z for each frequency,
    plots their maximum amplitudes, and adds vertical dashed lines at the frequency with the
    highest amplitude for each dataset. Annotations for the vertical lines are placed at the
    bottom of the plot to avoid overlapping with the waveforms.
    
    Parameters:
    - csv_files: list of str, paths to the CSV files.
    - labels: list of str, labels corresponding to each CSV file for the legend.
    - output_image: str or None, path to save the plot image. If None, the plot is shown.
    """
    # Validate input lengths
    if len(csv_files) != len(labels):
        raise ValueError("The number of CSV files must match the number of labels.")
    
    plt.figure(figsize=(12, 8))
    
    # Colors for plotting
    colors = ['blue', 'green', 'red', 'cyan', 'magenta', 'yellow', 'black']  # Extend if more datasets are added
    
    # Dictionary to store maximum frequencies
    max_freqs = {}
    
    # Plot each dataset
    for idx, csv_file in enumerate(csv_files):
        label = labels[idx]
        color = colors[idx % len(colors)]
        
        # Check if the CSV file exists
        if not os.path.exists(csv_file):
            raise FileNotFoundError(f"The file '{csv_file}' does not exist.")
        
        # Read the CSV file into a pandas DataFrame
        df = pd.read_csv(csv_file)
        
        # Validate the required columns exist
        if not {'frequency', 'accel_z'}.issubset(df.columns):
            raise ValueError(f"CSV file '{csv_file}' must contain 'frequency' and 'accel_z' columns.")
        
        # Compute the maximum of absolute accel_z for each frequency
        max_vibration = df.groupby('frequency')['accel_z'].apply(lambda x: x.abs().max()).reset_index()
        
        # Sort the data by frequency for better plotting
        max_vibration = max_vibration.sort_values(by='frequency')
        
        # Find the frequency with the maximum amplitude
        idx_max = max_vibration['accel_z'].idxmax()
        freq_max = max_vibration.loc[idx_max, 'frequency']
        amp_max = max_vibration.loc[idx_max, 'accel_z']
        max_freqs[label] = freq_max
        
        # Plot the max vibration amplitude vs frequency
        plt.plot(
            max_vibration['frequency'],
            max_vibration['accel_z'],
            marker='o',
            linestyle='-',
            color=color,
            label=label
        )
        
        # Add a vertical dashed line at the frequency with maximum amplitude
        plt.axvline(x=freq_max, color=color, linestyle='--', linewidth=1)
        
        # Annotate the vertical line at the bottom
        plt.text(
            freq_max, 
            plt.ylim()[0],  # Position at the bottom y-limit
            f'{freq_max} Hz', 
            rotation=90,
            verticalalignment='bottom', 
            horizontalalignment='right',
            color=color, 
            fontsize=9, 
            fontweight='bold'
        )
    
    # Plot Enhancements
    plt.title('Maximum Vibration Amplitude Over Frequency (Absolute Values)', fontsize=16)
    plt.xlabel('Frequency (Hz)', fontsize=14)
    plt.ylabel('Maximum |Acceleration| (m/s²)', fontsize=14)
    plt.grid(True, which='both', linestyle='--', linewidth=0.5, alpha=0.7)
    plt.legend()
    plt.tight_layout()
    
    # Save or show the plot
    if output_image:
        plt.savefig(output_image, dpi=300, bbox_inches='tight')
        print(f"Plot saved as '{output_image}'")
    else:
        plt.show()

# Example usage:
if __name__ == "__main__":
    # Define your CSV files and their corresponding labels
    csv_files = ['With_PCB_Data.csv', 'Without_PCB.csv']
    labels = ['With PCB', 'Without PCB']
    
    # Define output plot path or set to None to display
    output_plot = 'max_vibration_comparison.png'  # Change to None to display the plot instead
    
    plot_max_vibration(csv_files, labels, output_plot)
