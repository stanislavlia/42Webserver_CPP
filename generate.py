def generate_large_file(file_name, size_in_gb):
    size_in_bytes = int(size_in_gb * 1024 * 1024 * 1024)
    chunk_size = 1024 * 1024  # 1MB
    chunk_data = 'A' * chunk_size

    with open(file_name, 'w') as f:
        for _ in range(size_in_bytes // chunk_size):
            f.write(chunk_data)
        # Write the remaining bytes
        remaining_bytes = size_in_bytes % chunk_size
        if remaining_bytes > 0:
            f.write('A' * remaining_bytes)

if __name__ == "__main__":
    generate_large_file("large_file.txt", 0.5)