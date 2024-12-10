import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import matplotlib.ticker as ticker
import os

def plot_vibration_over_time(csv_file, output_image=None, sampling_rate=None):
    """
    Reads the CSV file, plots accel_z over time, and marks frequency change points.
    
    Parameters:
    - csv_file: str, path to the CSV file.
    - output_image: str or None, path to save the plot image. If None, the plot is shown.
    - sampling_rate: float or None, samples per second. If None, sample index is used as time.
    """
    try:
        # Check if the CSV file exists
        if not os.path.exists(csv_file):
            print(f"Error: The file '{csv_file}' does not exist.")
            return
        
        # Read the CSV file into a pandas DataFrame
        df = pd.read_csv(csv_file)
        
        # Validate the required columns exist
        if not {'frequency', 'accel_z'}.issubset(df.columns):
            print("Error: CSV file must contain 'frequency' and 'accel_z' columns.")
            return
        
        # Assign time based on sampling rate or sample index
        if sampling_rate is not None and sampling_rate > 0:
            df['time'] = df.index / sampling_rate  # time in seconds
            time_label = 'Time (s)'
        else:
            df['time'] = df.index  # use sample index as time
            time_label = 'Sample Index'
        
        # Detect frequency change points
        df['frequency_change'] = df['frequency'].diff().fillna(0) != 0
        change_points = df[df['frequency_change']].index.tolist()
        
        # Get frequency values at change points
        freq_values = df.loc[change_points, 'frequency'].tolist()
        
        # Plotting
        plt.figure(figsize=(15, 8))
        plt.plot(df['time'], df['accel_z'], color='blue', linewidth=1, label='Accel Z')
        
        # Add vertical lines at frequency change points
        for cp, freq in zip(change_points, freq_values):
            plt.axvline(x=df.at[cp, 'time'], color='red', linestyle='--', linewidth=1)
            plt.text(df.at[cp, 'time'], plt.ylim()[1], f'{freq} Hz', rotation=90,
                     verticalalignment='top', horizontalalignment='right', color='red', fontsize=8)
        
        plt.title('Vibration Amplitude with Frequency Bands')
        plt.xlabel('Sample')
        plt.ylabel('Acceleration (m/s²)')
        plt.grid(True, linestyle='--', alpha=0.5)
        plt.legend()
        
        # Improve x-axis formatting if time is in seconds
        if sampling_rate is not None and sampling_rate > 0:
            plt.gca().xaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
        
        plt.tight_layout()
        
        # Save or show the plot
        if output_image:
            plt.savefig(output_image, dpi=300, bbox_inches='tight')
            print(f"Plot saved as '{output_image}'")
        else:
            plt.show()
    
    except pd.errors.EmptyDataError:
        print("Error: The CSV file is empty.")
    except pd.errors.ParserError:
        print("Error: The CSV file is malformed.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage:
if __name__ == "__main__":
    csv_path = 'sensor_data_log.csv'  # Path to your CSV file
    output_plot = 'vibration_over_time.png'  # Change to None to display the plot instead
    sampling_rate = None  # Set to your actual sampling rate (samples per second) if known
    
    plot_vibration_over_time(csv_path, output_plot, sampling_rate)
