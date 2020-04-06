from subprocess import call, check_output, CalledProcessError, STDOUT
import shlex

call(["gcc", "-O3", "-fopenmp",  "-msse4", "conv-harness_David.c", "-o", "conv-harness_David"])
call(["gcc", "-O3", "-fopenmp",  "-msse4", "conv-harness_SSE.c", "-o", "conv-harness_SSE"])
call(["gcc", "-O3", "-fopenmp",  "-msse4", "conv-harness_OpenMP_Without_If.c", "-o", "conv-harness_OpenMP_Without_If"])
call(["echo", "Finished compiling."])

call(["echo", "Running tests..."])

files = ["./conv-harness_David ", "./conv-harness_SSE ", "./conv-harness_OpenMP_Without_If "]


for file in files :
    command1 = file + "16 16 1 32 32 20"
    command1 = shlex.split(command1)
    try:
        output = check_output(command1, stderr=STDOUT).decode()
        success = True 
    except CalledProcessError as e:
        output = e.output.decode()
        success = False
    print(file + ": " + output)





