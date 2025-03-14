import sys
import os


def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_bin_file> <output_bin_file>")
        sys.exit(1)

    input_bin_file = sys.argv[1]
    output_bin_file = sys.argv[2]
    max_size = 32 * 1024 * 1024  # 32MB

    try:
        with open(input_bin_file, "rb") as f:
            data = f.read()

        if len(data) > max_size:
            print("Error: The input file is larger than 32MB.")
            sys.exit(1)

        # Fill the file to 32MB if it's smaller
        data += b"\x00" * (max_size - len(data))

        with open(output_bin_file, "wb") as f:
            f.write(data)

        print(f"File saved to {output_bin_file}")

    except FileNotFoundError:
        print(f"Error: The file {input_bin_file} does not exist.")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
