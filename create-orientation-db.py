import sqlite3
import re

# Database setup
db_name = "calibration.db"
conn = sqlite3.connect(db_name)
cursor = conn.cursor()

cursor.execute("""
CREATE TABLE IF NOT EXISTS Position (
    Module INTEGER,
    Row INTEGER,
    Column INTEGER,
    Layer INTEGER,
    detector_type_x INTEGER,
    detector_type_y INTEGER,
    position_x REAL,
    position_y REAL,
    position_z REAL,
    orientation_x REAL,
    orientation_y REAL,
    orientation_z REAL
)
""")
conn.commit()

# Parsing function
def parse_rpc_section(section, filename):
    layer = int(re.search(r"RPC(\d+)", section).group(1))
    detector_types = re.findall(r"corry-det-(\d)(\d)\.geo", filename)
    detector_type_1, detector_type_2 = map(int, detector_types[0]) if detector_types else (0, 0)
    
    position_match = re.search(r"position = ([-\d\.]+)([a-z]+),([-\d\.]+)([a-z]+),([-\d\.]+)([a-z]+)", section)
    orientation_match = re.search(r"orientation = ([-\d\.]+)deg,([-\d\.]+)deg,([-\d\.]+)deg", section)
    
    if not position_match or not orientation_match:
        return None
    
    def convert_to_m(value, unit):
        return float(value) / 10000000 if unit == "um" else float(value) / 1000
    
    position_x = convert_to_m(position_match.group(1), position_match.group(2))
    position_y = convert_to_m(position_match.group(3), position_match.group(4))
    position_z = convert_to_m(position_match.group(5), position_match.group(6))
    
    orientation_x = float(orientation_match.group(1))
    orientation_y = float(orientation_match.group(2))
    orientation_z = float(orientation_match.group(3))
    
    return (0, 0, 0, layer, detector_type_1, detector_type_2, position_x, position_y, position_z, orientation_x, orientation_y, orientation_z)

# Read file

parsed_data = []

for filetag in ["00", "01", "10", "11"]:

    filename = f"corry-inputs/corry-det-{filetag}.geo"
    with open(filename, "r") as file:
        content = file.read()

    # Process each RPC section
    sections = re.findall(r"\[RPC\d+\][^\[]+", content)
    tmp_data = [parse_rpc_section(section, filename) for section in sections]
    parsed_data += [data for data in tmp_data if data is not None]

# Insert into database
cursor.executemany("""
INSERT INTO position (Module, Row, Column, Layer, detector_type_x, detector_type_y, position_x, position_y, position_z, orientation_x, orientation_y, orientation_z)
VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
""", parsed_data)
conn.commit()
conn.close()

print("Data successfully inserted into the database.")
