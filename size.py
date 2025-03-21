#
# Script written by chatGPT
#
import os
import io

def count_lines_of_code(directory):
    extensions = {".cpp", ".c", ".h", ".vhd"}
    total_lines = 0
    file_counts = {}

    # Add folders to check
    target_folders = ['firmware', 'rtl']

    for root, dirs, files in os.walk(directory):
        # Check if the folder is 'firmware' or 'rtl' (at any level)
        if any(folder in root for folder in target_folders):
            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    file_path = os.path.join(root, file)
                    try:
                        with io.open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                            line_count = sum(1 for _ in f)
                            total_lines += line_count
                            file_counts[file_path] = line_count
                    except Exception as e:
                        print("Error reading {}: {}".format(file_path, e))

    print("Lines of code per file:")
    for file, count in file_counts.items():
        print("{}: {} lines".format(file, count))

    print("\nTotal lines of code: {}".format(total_lines))

if __name__ == "__main__":
    target_directory = os.getcwd()
    count_lines_of_code(target_directory)
