import re
import csv

log_file_path = 'E:\\Progetti\\Aziende\\Carmelo_Fiorello\\CRM3824\\CRM3824.fw\\vaso-app\\log\\COM5_2024_12_05.11.33.13.786.txt'
csv_file_path = 'E:\\Progetti\\Aziende\\Carmelo_Fiorello\\CRM3824\\CRM3824.fw\\vaso-app\\log\\thread_usage.csv'

# Updated regular expression to match the new log line format
thread_usage_pattern = re.compile(r'(\w+)\s+:\s+STACK:.*\((\d+)\s*%\);')

def parse_log_file(log_file_path):
    with open(log_file_path, 'r') as file:
        lines = file.readlines()

    thread_data = []
    current_sample = {}
    for line in lines:
        if "Thread analyze:" in line:
            # save the current sample to csv
            if current_sample:
                thread_data.append(current_sample)
                current_sample = {}
                continue
        # else parse the line
        match = thread_usage_pattern.search(line)
        if match:
            thread_name = match.group(1)
            usage_percentage = match.group(2)
            current_sample[thread_name] = usage_percentage

    # appwnd last sample
    if current_sample:
        thread_data.append(current_sample)

    return thread_data

def write_csv(thread_data, csv_file_path):
    if not thread_data:
        return

    thread_names = sorted(thread_data[0].keys())

    with open(csv_file_path, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(thread_names)
        for sample in thread_data:
            row = [sample.get(thread, '') for thread in thread_names]
            writer.writerow(row)

if __name__ == "__main__":
    thread_data = parse_log_file(log_file_path)
    write_csv(thread_data, csv_file_path)
