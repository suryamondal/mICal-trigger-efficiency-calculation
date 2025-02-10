import os
import subprocess
import concurrent.futures
import glob
import signal
import sys

# Configuration
ROOT_DIR = "/media/surya/Surya_1/DaqMadurai/maduraiData_mICAL/SuryaFormat"
OUTPUT_DIR = "output"
EXECUTABLE = "build/alignment"
SPLIT_SIZE = 500001
MAX_WORKERS = 10
MAX_FILES = 1

# ROOT script for getting entries (embedded as a string)
ROOT_SCRIPT = """
void GetEntries(const char* filename) {
  TFile *file = TFile::Open(filename, "read");
  if (!file || file->IsZombie()) {
    std::cout << "0" << std::endl;
    return;
  }
  TTree *tree = (TTree*) file->Get("SNM");
  if (!tree) {
    std::cout << "0" << std::endl;
    return;
  }
  std::cout << tree->GetEntries() << std::endl;
  file->Close();
}
"""

def write_root_script():
    """Writes the ROOT script to a temporary file."""
    script_path = "GetEntries.C"
    with open(script_path, "w") as f:
        f.write(ROOT_SCRIPT)

def get_entries(root_file):
    """
    Get the number of entries in a ROOT file using a ROOT script.
    Modify this function based on how you determine entries.
    """
    try:
        output = subprocess.check_output(["root", "-l", "-b", "-q", f"GetEntries.C(\"{root_file}\")"])
        # Extract only numeric lines
        lines = output.decode().strip().split("\n")
        if lines:
            return int(lines[-1])
        else:
            print(f"Error: No valid entry count found for {root_file}.")
            return 0
    except Exception as e:
        print(f"Error getting entries for {root_file}: {e}")
        return 0

def process_root_file(root_file):
    """
    Processes a ROOT file by determining the number of entries
    and running the C++ executable in parallel if needed.
    """
    entries = get_entries(root_file)
    if entries == 0:
        print(f"Skipping {root_file}, could not determine entries.")
        return

    file_name = os.path.basename(root_file)
    file_no_ext = os.path.splitext(file_name)[0]
    output_prefix = os.path.join(OUTPUT_DIR, file_no_ext)

    # Create logs directory if it doesn’t exist
    log_dir = os.path.join(OUTPUT_DIR, "logs")
    os.makedirs(log_dir, exist_ok=True)

    split_count = 0
    for start in range(0, max(entries, 1), SPLIT_SIZE):
        end = min(start + SPLIT_SIZE - 1, entries - 1)
        extension = "" if entries <= SPLIT_SIZE else f"_{split_count:02d}"
        log_file = os.path.join(log_dir, f"{file_no_ext}{extension}.log")
        print(f"Processing: {start} {end}")  # Print affected range
        cmd = [EXECUTABLE, root_file, output_prefix + extension, str(start), str(end)]
        print(f"Running: {' '.join(cmd)} | Log: {log_file}")
        with open(log_file, "w") as log:
            subprocess.run(cmd, stdout=log, stderr=log)
        split_count += 1  # Increment split counter

def main():
    root_files = sorted(glob.glob(os.path.join(ROOT_DIR, "*.root")))
    write_root_script()

    with concurrent.futures.ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        future_to_file = {executor.submit(process_root_file, rf): rf for rf in root_files[0:MAX_FILES]}
        
        try:
            for future in concurrent.futures.as_completed(future_to_file):
                future.result()
        except KeyboardInterrupt:
            print("KeyboardInterrupt detected! Stopping workers...")
            executor.shutdown(wait=True, cancel_futures=True)
            sys.exit(1)

if __name__ == "__main__":
    main()
