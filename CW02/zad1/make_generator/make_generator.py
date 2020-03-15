record_length = [1, 4, 512, 1024, 4096, 8192]
record_number = ["$(R_NUM_1)", "$(R_NUM_2)"]

command = ""
for rl in record_length:
    for rn in record_number:
        command += f'\techo "-----{rn} records of {rl} characters-----" >> "wyniki.txt"\n'
        command += f"\t./program generate data {rn} {rl}\n"
        command += f'\techo "copy using system functions" >> "wyniki.txt"\n'
        command += f"\t./program start_timer copy data data_sys {rn} {rl} sys stop_timer\n"
        command += f'\techo "copy using library functions" >> "wyniki.txt"\n'
        command += f"\t./program start_timer copy data data_lib {rn} {rl} lib stop_timer\n"
        command += f'\techo "sort using system functions" >> "wyniki.txt"\n'
        command += f"\t./program start_timer sort data_sys {rn} {rl} sys stop_timer\n"
        command += f'\techo "sort using library functions" >> "wyniki.txt"\n'
        command += f"\t./program start_timer sort data_lib {rn} {rl} lib stop_timer\n"


f = open("commands.txt", "w")
f.write(command)
f.close()
