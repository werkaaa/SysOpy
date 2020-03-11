import random
import string

file_counts = [5, 50, 100]
differences = ["small_diff", "medium_diff", "big_diff"]

f = open("commands.txt", "w")
for diff in differences:
    for file_count in file_counts:
        command = f"\t./main create_table {file_count} start_timer {file_count}_{diff}_compare compare_pairs {file_count} "

        for i in range(file_count):
            command += f"{i}a.txt:{i}b.txt "

        command += "stop_timer\n"

        f.write(command)

        command = f"\t./main create_table {file_count} compare_pairs {file_count} "

        for i in range(file_count):
            command += f"{i}a.txt:{i}b.txt "

        command += f"start_timer {file_count}_{diff}_remove "

        for i in range(file_count):
            command += "remove_block 0 "

        command += "stop_timer\n"

        f.write(command)

        command = f"\t./main create_table {file_count} start_timer {file_count}_{diff}_compare_remove "

        for i in range(file_count):
            command += f"compare_pairs 1 {i}a.txt:{i}b.txt remove_block 0 "

        command += "stop_timer\n"

        f.write(command)

f.close()
