import os
import io

def count_lines_of_code(directory):
    extensions = {".cpp", ".c", ".h", ".vhd", "Makefile"}
    total_lines = 0
    file_counts = {}

    # Subdirectory counters
    rtl_lines = 0
    linux_lines = 0
    x86_lines = 0
    common_lines = 0
    app_lines = 0
    test_lines = 0  # New counter

    # Add folders to check
    target_folders = ['firmware', 'rtl', 'other/random_sources']

    for root, dirs, files in os.walk(directory):
        # Normalize path separators for portability
        normalized_root = root.replace("\\", "/")

        if any(folder in normalized_root for folder in target_folders):
            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    file_path = os.path.join(root, file)
                    try:
                        with io.open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                            line_count = sum(1 for _ in f)
                            total_lines += line_count
                            file_counts[file_path] = line_count

                            # Check subdirectories and update respective counters
                            if "rtl" in normalized_root:
                                rtl_lines += line_count
                            elif "firmware/linux" in normalized_root:
                                linux_lines += line_count
                            elif "firmware/x86" in normalized_root:
                                x86_lines += line_count
                            elif "firmware/common" in normalized_root:
                                common_lines += line_count
                            elif "firmware/app" in normalized_root:
                                app_lines += line_count
                            elif "other/random_sources" in normalized_root:
                                test_lines += line_count

                    except Exception as e:
                        print("Error reading {}: {}".format(file_path, e))

    print("Lines of code per file:")
    for file, count in file_counts.items():
        print("{}: {} lines".format(file, count))

    print("\n           FPGA Logic -> {} lines".format(rtl_lines))
    print("        Kernel Module -> {} lines".format(linux_lines))
    print("   Common Kernel Code -> {} lines".format(common_lines))
    print("User Space Appliation -> {} lines".format(app_lines))
    print("Master x86 Controller -> {} lines".format(x86_lines))
    print("      Testing Sources -> {} lines".format(test_lines))
    print("----------------------------------------------")
    print("                Total -> {} lines of code".format(total_lines))

if __name__ == "__main__":
    target_directory = os.getcwd()
    count_lines_of_code(target_directory)
