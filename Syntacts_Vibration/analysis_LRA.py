import pandas as pd
import matplotlib.pyplot as plt

def plot_max_vibration(csv_file, output_image=None):
    """
    Reads the CSV file, computes the maximum accel_z for each frequency,
    and plots Frequency vs. Maximum Vibration Amplitude.
    
    Parameters:
    - csv_file: str, path to the CSV file.
    - output_image: str or None, path to save the plot image. If None, the plot is shown.
    """
    try:
        # Read the CSV file into a pandas DataFrame
        df = pd.read_csv(csv_file)
        
        # Validate the required columns exist
        if not {'frequency', 'accel_z'}.issubset(df.columns):
            raise ValueError("CSV file must contain 'frequency' and 'accel_z' columns.")
        
        # Group the data by 'frequency' and compute the maximum 'accel_z' for each group
        max_vibration = df.groupby('frequency')['accel_z'].max().reset_index()
        
        # Sort the data by frequency for better plotting
        max_vibration = max_vibration.sort_values(by='frequency')
        
        # Plotting
        plt.figure(figsize=(10, 6))
        plt.plot(max_vibration['frequency'], max_vibration['accel_z'], marker='o', linestyle='-',
                 color='b', label='Max Vibration Amplitude')
        
        plt.title('Maximum Vibration Amplitude Over Frequency')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Maximum Acceleration (m/s²)')
        plt.grid(True)
        plt.legend()
        
        # Save or show the plot
        if output_image:
            plt.savefig(output_image, dpi=300, bbox_inches='tight')
            print(f"Plot saved as {output_image}")
        else:
            plt.show()
    
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
    except pd.errors.EmptyDataError:
        print("Error: The CSV file is empty.")
    except pd.errors.ParserError:
        print("Error: The CSV file is malformed.")
    except Exception as e:
        print(f"An error occurred: {e}")

# Example usage:
if __name__ == "__main__":
    csv_path = 'sensor_data_log.csv'  # Path to your CSV file
    output_plot = 'max_vibration_over_frequency.png'  # Change to None to display the plot instead
    
    plot_max_vibration(csv_path, output_plot)
