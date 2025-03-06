import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os

def read_and_process_csv(csv_file, sampling_rate=None):
    """
    Reads the CSV file and returns a processed DataFrame with 'time' and detection of frequency changes.
    
    Parameters:
    - csv_file: str, path to the CSV file.
    - sampling_rate: float or None, samples per second. If None, sample index is used as time.
    
    Returns:
    - df: pandas DataFrame with added 'time' and 'frequency_change' columns.
    """
    if not os.path.exists(csv_file):
        raise FileNotFoundError(f"The file '{csv_file}' does not exist.")
        
    df = pd.read_csv(csv_file)
    
    # Validate required columns
    if not {'frequency', 'accel_z'}.issubset(df.columns):
        raise ValueError("CSV file must contain 'frequency' and 'accel_z' columns.")
    
    # Assign time based on sampling rate or sample index
    if sampling_rate is not None and sampling_rate > 0:
        df['time'] = df.index / sampling_rate  # time in seconds
    else:
        df['time'] = df.index  # use sample index as time
    
    # Detect frequency change points
    df['frequency_change'] = df['frequency'].diff().fillna(0) != 0
    return df

def plot_vibration_subplot(ax, df, title, color='green'):
    """
    Plots accel_z over time on the given Axes object and marks frequency change points.
    
    Parameters:
    - ax: matplotlib Axes object to plot on.
    - df: pandas DataFrame with 'time', 'accel_z', and 'frequency_change' columns.
    - title: str, title for the subplot.
    - color: str, color for the accel_z line.
    """
    # Set Y-axis limits
    ax.set_ylim(-3, 3)
    
    # Plot accel_z
    ax.plot(df['time'], df['accel_z'], color=color, linewidth=1, label='Accel Z')
    
    # Identify frequency change points
    change_points = df[df['frequency_change']].index.tolist()
    freq_values = df.loc[change_points, 'frequency'].tolist()
    
    # Get current Y-axis limits for text placement
    ymin, ymax = ax.get_ylim()
    
    # Add vertical lines and frequency labels
    for cp, freq in zip(change_points, freq_values):
        time_cp = df.at[cp, 'time']
        ax.axvline(x=time_cp, color='red', linestyle='--', linewidth=1)
        ax.text(time_cp, ymax, f'{freq} Hz', rotation=90,
                verticalalignment='top', horizontalalignment='right', color='red', fontsize=8)
    
    # Set subplot title and labels
    ax.set_title(title)
    ax.set_xlabel('Time (s)' if 'Time' in title else 'Sample Index')
    ax.set_ylabel('Acceleration (m/s²)')
    ax.grid(True, linestyle='--', alpha=0.5)
    ax.legend()

def plot_vibration_two_subplots(csv_file_1, csv_file_2, output_image=None, sampling_rate=None):
    """
    Reads two CSV files and plots their accel_z data on separate subplots.
    
    Parameters:
    - csv_file_1: str, path to the first CSV file.
    - csv_file_2: str, path to the second CSV file.
    - output_image: str or None, path to save the plot image. If None, the plot is shown.
    - sampling_rate: float or None, samples per second. If None, sample index is used as time.
    """
    try:
        # Process both CSV files
        df1 = read_and_process_csv(csv_file_1, sampling_rate=sampling_rate)
        df2 = read_and_process_csv(csv_file_2, sampling_rate=sampling_rate)
        
        # Determine x-axis label based on sampling rate
        if sampling_rate is not None and sampling_rate > 0:
            x_label = 'Time (s)'
        else:
            x_label = 'Sample Index'
        
        # Create a figure with two subplots (2 rows, 1 column)
        fig, axs = plt.subplots(2, 1, figsize=(15, 12), sharex=False)
        
        # Plot first CSV on the first subplot (Without_PCB.csv) in green
        plot_vibration_subplot(axs[0], df1, title='Vibration Amplitude without PCB', color='green')
        
        # Plot second CSV on the second subplot (With_PCB_Data.csv) in blue
        plot_vibration_subplot(axs[1], df2, title='Vibration Amplitude with PCB', color='blue')
        
        # Adjust layout
        plt.tight_layout()
        
        # Save or show the plot
        if output_image:
            plt.savefig(output_image, dpi=300, bbox_inches='tight')
            print(f"Plot saved as '{output_image}'")
        else:
            plt.show()
    
    except FileNotFoundError as fnf_error:
        print(f"Error: {fnf_error}")
    except ValueError as val_error:
        print(f"Error: {val_error}")
    except pd.errors.EmptyDataError:
        print("Error: One of the CSV files is empty.")
    except pd.errors.ParserError:
        print("Error: One of the CSV files is malformed.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def main():
    """
    Main function to execute the plotting of two CSV files with specified colors.
    """
    # Define file paths
    csv_path_without_pcb = 'Without_PCB.csv'       # Path to your first CSV file (Without PCB)
    csv_path_with_pcb = 'With_PCB_Data.csv'        # Path to your second CSV file (With PCB)
    output_plot = 'vibration_comparison.png'       # Path to save the plot image (set to None to display)
    
    # Define sampling rate (set to your actual sampling rate if known, e.g., 60 Hz)
    sampling_rate = 60  # samples per second
    
    # Plot the two subplots
    plot_vibration_two_subplots(csv_path_without_pcb, csv_path_with_pcb, output_plot, sampling_rate)

if __name__ == "__main__":
    main()
