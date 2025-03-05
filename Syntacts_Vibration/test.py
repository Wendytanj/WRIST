import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import os
from scipy.signal import hilbert
import numpy as np

def read_and_process_csv(csv_file, sampling_rate=None):
    """
    Reads the CSV file and returns a processed DataFrame with 'time' and 'accel_z' columns.
    
    Parameters:
    - csv_file: str, path to the CSV file.
    - sampling_rate: float or None, samples per second. If None, sample index is used as time.
    
    Returns:
    - df: pandas DataFrame with added 'time' column.
    """
    if not os.path.exists(csv_file):
        raise FileNotFoundError(f"The file '{csv_file}' does not exist.")
        
    df = pd.read_csv(csv_file)
    
    # Validate required columns
    if not {'accel_z'}.issubset(df.columns):
        raise ValueError("CSV file must contain 'accel_z' column.")
    
    # Assign time based on sampling rate or sample index
    if sampling_rate is not None and sampling_rate > 0:
        df['time'] = df.index / sampling_rate  # time in seconds
    else:
        df['time'] = df.index  # use sample index as time
    
    return df

def compute_hilbert_envelope(accel_z):
    """
    Computes the Hilbert transform of the accel_z signal and returns the envelope.
    
    Parameters:
    - accel_z: numpy array, the acceleration signal.
    
    Returns:
    - envelope: numpy array, the instantaneous amplitude envelope of the signal.
    """
    analytic_signal = hilbert(accel_z)
    envelope = np.abs(analytic_signal)
    return envelope

def plot_hilbert_transform(ax, time, accel_z, envelope, title, color='green'):
    """
    Plots the original accel_z signal and its Hilbert envelope on the given Axes.
    
    Parameters:
    - ax: matplotlib Axes object to plot on.
    - time: numpy array or pandas Series, time axis.
    - accel_z: numpy array, original acceleration signal.
    - envelope: numpy array, Hilbert transform envelope.
    - title: str, title for the subplot.
    - color: str, color for the accel_z line.
    """
    ax.plot(time, accel_z, color=color, linewidth=1, label='Accel Z')
    ax.plot(time, envelope, color='orange', linewidth=1, linestyle='--', label='Envelope')
    
    ax.set_title(title)
    ax.set_xlabel('Time (s)' if isinstance(time, (np.ndarray, pd.Series)) and np.issubdtype(time.dtype, np.number) else 'Sample Index')
    ax.set_ylabel('Acceleration (m/s²)')
    ax.legend()
    ax.grid(True, linestyle='--', alpha=0.5)

def plot_hilbert_two_subplots(csv_file_1, csv_file_2, output_image=None, sampling_rate=None):
    """
    Reads two CSV files, computes Hilbert transform on 'accel_z', and plots the results in subplots.
    
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
        
        # Compute Hilbert envelopes
        envelope1 = compute_hilbert_envelope(df1['accel_z'].values)
        envelope2 = compute_hilbert_envelope(df2['accel_z'].values)
        
        # Create a figure with two subplots (2 rows, 1 column)
        fig, axs = plt.subplots(2, 1, figsize=(15, 10), sharex=False)
        
        # Plot first CSV on the first subplot (Without_PCB.csv) in green
        plot_hilbert_transform(
            ax=axs[0],
            time=df1['time'],
            accel_z=df1['accel_z'],
            envelope=envelope1,
            title='Hilbert Transform - Without PCB',
            color='green'
        )
        
        # Plot second CSV on the second subplot (With_PCB_Data.csv) in blue
        plot_hilbert_transform(
            ax=axs[1],
            time=df2['time'],
            accel_z=df2['accel_z'],
            envelope=envelope2,
            title='Hilbert Transform - With PCB',
            color='blue'
        )
        
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
    Main function to execute the Hilbert transform plotting for two CSV files.
    """
    # Define file paths
    csv_path_without_pcb = 'Without_PCB.csv'       # Path to your first CSV file (Without PCB)
    csv_path_with_pcb = 'With_PCB_Data.csv'        # Path to your second CSV file (With PCB)
    output_plot = 'hilbert_comparison.png'         # Path to save the plot image (set to None to display)
    
    # Define sampling rate (set to your actual sampling rate if known, e.g., 60 Hz)
    sampling_rate = 60  # samples per second
    
    # Plot the Hilbert transforms
    plot_hilbert_two_subplots(csv_path_without_pcb, csv_path_with_pcb, output_plot, sampling_rate)

if __name__ == "__main__":
    main()
