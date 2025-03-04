import os
import subprocess
import concurrent.futures
import glob
import signal
import sys
import re

# Configuration
ROOT_DIR = "/media/surya/Surya_1/DaqMadurai/maduraiData_mICAL/SuryaFormat"
FILE_REGEX = "SNM_RPC*.root"
OUTPUT_DIR = "output"
# EXECUTABLE = "build/time-alignment"
EXECUTABLE = "build/grouping-and-efficiency"
# OUTPUT_DIR = "input/corry-input"
# EXECUTABLE = "build/createTTreeForCorry"
SPLIT_SIZE = 10000
MAX_WORKERS = 4
MAX_FILES = 1

class EventCounter:
    def __init__(self):
        self.count = 0
    def increment(self, count):
        self.count += count
    def entries(self):
        return self.count

counter = EventCounter()

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

def execute_job(cmd):
    print(f"Running: {' '.join(cmd[0:-1])} | Log: {cmd[-1]}")
    with open(cmd[-1], "w") as log:
        subprocess.run(cmd[0:-1], stdout=log, stderr=log)

def process_root_file(root_file):
    """
    Processes a ROOT file by determining the number of entries
    and running the C++ executable in parallel if needed.
    """
    entries = get_entries(root_file)
    if entries == 0:
        print(f"Skipping {root_file}, could not determine entries.")
        return []

    file_name = os.path.basename(root_file)
    file_no_ext = os.path.splitext(file_name)[0]
    output_prefix = os.path.join(OUTPUT_DIR, file_no_ext)

    # Create logs directory if it doesn’t exist
    log_dir = os.path.join(OUTPUT_DIR, "logs")
    os.makedirs(log_dir, exist_ok=True)

    # Check if any matching output file already exists
    existing_outputs = glob.glob(f"{output_prefix}*")
    if existing_outputs:
        print(f"Skipping {root_file}, output files already exist: {existing_outputs}")
        return []

    cmdList = []

    split_count = 0
    for start in range(0, max(entries, 1), SPLIT_SIZE):
        end = min(start + SPLIT_SIZE - 1, entries - 1)
        suffix = "" if entries <= SPLIT_SIZE else f"_{split_count:04d}"
        log_file = os.path.join(log_dir, f"{file_no_ext}{suffix}.log")
        print(f"Processing: {start} {end}")  # Print affected range
        cmd = [EXECUTABLE, root_file, output_prefix + suffix,
               str(start), str(end), str(counter.entries()),
               log_file]
        cmdList += [cmd]
        split_count += 1  # Increment split counter

        counter.increment(end - start + 1)

    return cmdList

def submit_jobs():
    root_files = sorted(glob.glob(os.path.join(ROOT_DIR, FILE_REGEX)))
    write_root_script()
    cmdLists = []
    for rf in root_files[0:MAX_FILES]:
        cmdLists += process_root_file(rf)
    with concurrent.futures.ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        # future_to_file = {executor.submit(process_root_file, rf): rf for rf in root_files[0:MAX_FILES]}
        future_to_file = {executor.submit(execute_job, rf): rf for rf in cmdLists}
        try:
            for future in concurrent.futures.as_completed(future_to_file):
                future.result()
        except KeyboardInterrupt:
            print("KeyboardInterrupt detected! Stopping workers...")
            executor.shutdown(wait=True, cancel_futures=True)
            sys.exit(1)

def merge_files_by_date():
    """Merges all output files from the same day into one inside output/merged/."""
    merged_dir = os.path.join(OUTPUT_DIR, "merged")
    os.makedirs(merged_dir, exist_ok=True)

    # Find all unique (prefix, date, suffix) groups
    file_pattern = os.path.join(OUTPUT_DIR, "*_20??????_*")  # Matches *_YYYYMMDD_*
    output_files = sorted(glob.glob(file_pattern))
    file_groups = {}

    for f in output_files:
        match = re.search(r"(.+)_20(\d{6})_\d{6}_(\d+)\.(.+)", os.path.basename(f))
        if match:
            prefix, date, _, suffix = match.groups()
            key = (prefix, date, suffix)  # Group by prefix, date, and suffix
            file_groups.setdefault(key, []).append(f)

    # Merge each group in parallel
    def merge_group(key, files):
        prefix, date, suffix = key
        merged_file = os.path.join(merged_dir, f"{prefix}_20{date}.{suffix}")
        log_file = os.path.join(merged_dir, f"{prefix}_20{date}.{suffix}.log")

        if os.path.exists(merged_file):
            print(f"Skipping merge for {merged_file}, already exists.")
            return

        print(f"Merging {len(files)} files into {merged_file}... | Log: {log_file}")
        cmd = ["hadd", "-f", merged_file] + files
        print(f"Running: {' '.join(cmd)} | Log: {log_file}")
        with open(log_file, "w") as log:
            cmd = ["hadd", "-f", merged_file] + files
            subprocess.run(cmd, stdout=log, stderr=log)

    with concurrent.futures.ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = {executor.submit(merge_group, key, files): key for key, files in file_groups.items()}
        for future in concurrent.futures.as_completed(futures):
            future.result()  # Ensure all merging completes


def main():

    submit_jobs();
    # merge_files_by_date()

if __name__ == "__main__":
    main()
