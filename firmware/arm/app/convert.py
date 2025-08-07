def parse_line(line):
    """
    Parse a line like 'R:80D:00' into (0x80, 0x00)
    Uses rpartition('D') to safely extract the correct value.
    """
    try:
        parts = line.strip().split(':')
        if len(parts) != 3 or not parts[0].startswith('R'):
            raise ValueError(f"Invalid format: {line}")

        mosi_str = parts[1]
        mosi_hex, sep, _ = mosi_str.rpartition('D')  # safely extract value before last 'D'
        if not mosi_hex:
            raise ValueError(f"Cannot extract MOSI hex from: {line}")

        mosi_val = int(mosi_hex, 16)
        miso_val = int(parts[2], 16)

        return mosi_val, miso_val
    except Exception as e:
        print(f"Skipping invalid line: {line.strip()} | Error: {e}")
        return None


def process_file(input_filename, output_filename):
    with open(input_filename, 'r') as file:
        lines = file.readlines()

    parsed = [parse_line(line) for line in lines]
    parsed = [p for p in parsed if p is not None]

    with open(output_filename, 'w') as out_file:
        for i in range(0, len(parsed) - 2, 3):
            mosi = [f"0x{parsed[i][0]:02X}", f"0x{parsed[i+1][0]:02X}", f"0x{parsed[i+2][0]:02X}"]
            miso = [f"0x{parsed[i][1]:02X}", f"0x{parsed[i+1][1]:02X}", f"0x{parsed[i+2][1]:02X}"]
            out_file.write(f"MOSI{{{', '.join(mosi)}}} MISO{{{', '.join(miso)}}}\n")

if __name__ == "__main__":
    input_file = "uart_data.txt"         # Your input file
    output_file = "parsed_output.txt"    # Your desired output file
    process_file(input_file, output_file)
