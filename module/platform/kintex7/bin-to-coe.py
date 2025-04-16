# bin-to-coe.py
# Convert binary file to COE

import sys


def bin_to_coe(bin_file, coe_file, length):
    with open(bin_file, "rb") as f:
        content = f.read()

    with open(coe_file, "w") as f:
        f.write("memory_initialization_radix=16;\n")
        f.write("memory_initialization_vector=\n")

        if len(content) == 0:
            f.write("00000000;\n")
        else:
            for i in range(0, len(content), length):
                word = content[i : min(len(content) - 1, i + length)]
                f.write(f"{int.from_bytes(word, byteorder='little'):08x},\n")

        f.seek(f.tell() - 2, 0)
        f.write(";\n")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: bin_to_coe.py <input_bin_file> <output_coe_file> <size>")
        sys.exit(1)

    bin_file = sys.argv[1]
    coe_file = sys.argv[2]
    input_size = int(sys.argv[3])

    bin_to_coe(bin_file, coe_file, input_size)
