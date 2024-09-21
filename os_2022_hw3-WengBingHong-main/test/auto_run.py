from subprocess import Popen, PIPE, run
from os.path import exists
import sys

executable = './scheduler_simulator'
prompt = ""

def read_test_case(test_case):
    input = ""
    with open(test_case, 'r') as f:
        lines = f.readlines()
        for line in lines:
            input = input + line
    return input


if __name__ == '__main__':
    if not exists(executable):
        print("The executable file is not existed. Please compile the source code first.")
        sys.exit(0)
    
    if len(sys.argv) < 3:
        print("Too few arguments.")
        sys.exit(0)
    scheduling_algo = sys.argv[1]
    test_case = sys.argv[2]
    if not exists(test_case):
        print("The test case is not existed.")
        sys.exit(0)

    prompt = run([executable, 'FCFS'], stdout=PIPE, 
                  input='exit\n', encoding='ascii').stdout

    input = read_test_case(test_case)
    if scheduling_algo == "all":
        result = run([executable, 'FCFS'], stdout=PIPE, 
                  input=input, encoding='ascii')
        f = open(test_case.split('.')[0] + "_FCFS.txt", 'w')
        f.write(result.stdout.replace(prompt , ""))
        f.close()

        result = run([executable, 'RR'], stdout=PIPE, 
                  input=input, encoding='ascii')
        f = open(test_case.split('.')[0] + "_RR.txt", 'w')
        f.write(result.stdout.replace(prompt , ""))
        f.close()

        result = run([executable, 'PP'], stdout=PIPE, 
                  input=input, encoding='ascii')
        f = open(test_case.split('.')[0] + "_PP.txt", 'w')
        f.write(result.stdout.replace(prompt , ""))
        f.close()
    else:
        result = run([executable, scheduling_algo], stdout=PIPE, 
                  input=input, encoding='ascii')
        f = open(test_case.split('.')[0] + "_" + scheduling_algo + ".txt", 'w')
        f.write(result.stdout.replace(prompt , ""))
        f.close()
