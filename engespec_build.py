import os
import subprocess
import argparse


def run_command(command, working_directory=None):
    """Run a shell command in the specified directory."""
    if working_directory:
        absolute_path = os.path.abspath(working_directory)
        print(f"Changing directory to: {absolute_path}")
        os.chdir(absolute_path)
        print(f"Current working directory: {os.getcwd()}")
    print(f"Running: {command}")
    result = subprocess.run(
        command,
        shell=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    if result.returncode == 0:
        print(result.stdout)
    else:
        print(f"Error running '{command}':")
        print(result.stderr)
        exit(1)


def main(clean):
    # Step 5: Build the libraries
    print("Building sort routine")

    current_path = os.getcwd()
    temp = current_path.split("EngeSpec")
    if len(temp) == 1:
        print("Error: Could not find EngeSpec root directory")
        exit(1)
    os.chdir(temp[0] + "EngeSpec")

    lib_directory = os.path.abspath("lib")

    run_command("cmake .", lib_directory)
    if clean:
        run_command("make clean", lib_directory)
    run_command("make", lib_directory)

    source_file = os.path.join(lib_directory, "libLenaSort.so")
    dest_file = os.path.join(current_path, "EngeSort.so")

    # Ensure the source file exists
    if not os.path.exists(source_file):
        print(f"Error: {source_file} not found.")
        exit(1)

    # Remove existing symbolic link or file if it exists
    if os.path.exists(dest_file):
        print("Removing existing symbolic link")
        os.remove(dest_file)

    # Create symbolic link
    try:
        os.symlink(source_file, dest_file)
        print(f"Created symbolic link {dest_file} -> {source_file}")
    except OSError as e:
        print(f"Error: creating symbolic link: {e}")
        exit(1)

    print("Build done!")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Build and set up EngeSpec.")
    parser.add_argument(
        "--clean", action="store_true", help="Run 'make clean' before building"
    )
    args = parser.parse_args()

    main(clean=args.clean)
