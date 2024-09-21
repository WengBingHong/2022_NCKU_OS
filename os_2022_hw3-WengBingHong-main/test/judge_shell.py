from subprocess import Popen, PIPE, run
from os.path import exists 
import sys
import os
import time

executable = './scheduler_simulator'
algo = 'FCFS'
test_txt = './test/test_shell.txt'
prompt = ""

class Part1:
    def __init__(self):
        self.accept = [False] * 5
        self.demo = []
        with open(test_txt, 'r') as f:
            self.demo = list(f.readlines())

    def test1(self, err):
        try:
            input = 'cat ' + test_txt + '\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            output = result.stdout.split('\n')[:-1]
            if err:
                print(result.stdout)

            self.accept[0] = True
            for i in range(len(self.demo)):
                if not self.demo[i].strip('\n') == output[i]:
                    self.accept[0] = False
                    break
        except:
            self.accept[0] = False

    def test2(self, err):
        try:
            p = Popen(executable)
            pid = p.pid + 2
            p.kill()

            input = 'cat ' + test_txt + ' &\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)

            output = result.stdout.split('\n')
            if output[0].find(str(pid)) < 0:
                return
            
            self.accept[1] = True
            for i in range(len(self.demo)):
                if i == 0:
                    if not output[1:][i].find(self.demo[i]):
                        self.accept[1] = False
                        break
                    continue
                if not self.demo[i].strip('\n') == output[1:][i]:
                    self.accept[1] = False
                    break
        except:
            self.accept[1] = False
        
    def test3(self, err):
        try:
            input = 'cat < ' + test_txt + '\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            output = result.stdout.split('\n')[:-1]
            self.accept[2] = True
            for i in range(len(self.demo)):
                if not self.demo[i].strip('\n') == output[i]:
                    self.accept[2] = False
                    break
        except:
            self.accept[2] = False

    def test4(self, err):
        try:
            input = 'cat ' + test_txt + ' > out\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            with open('out1', 'r') as f:
                lines = f.readlines()
                if lines == self.demo and result.stdout == prompt * 2:
                    self.accept[3] = True
        except:
            self.accept[3] = False

    def test5(self, err):
        try:
            input = 'cat ' + test_txt + ' | tail -5 | head -3 | grep os\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            output = result.stdout.split('\n')[:-1]
            index = [2, 3]
            self.accept[4] = True
            for i in range(len(index)):
                if not self.demo[index[i]].strip('\n') == output[i]:
                    self.accept[4] = False
                    break
        except:
            self.accept[4] = False
    
class Part2:
    def __init__(self):
        self.accept = [False] * 4
        self.pwd = os.getcwd()

    def cd(self, err):
        try:
            input = 'pwd\ncd ..\npwd\ncd ' + self.pwd + '\npwd\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            
            output = result.stdout.split('\n')[:-1]
            self.accept[0] = True
            if not self.pwd == output[0] and self.pwd == output[2]:
                self.accept[0] = False
                return
            if not output[0][:output[0].rfind('/')] == output[1]:
                self.accept[0] = False
                return
        except:
            self.accept[0] = False

    def echo(self, err):
        try:
            input = 'echo qwertyuiop\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            output = prompt + 'qwertyuiop\n' + prompt
            if result.stdout == output:
                self.accept[1] = True
        except:
            self.accept[1] = False

    def record(self, err):
        try:
            self.accept[2] = True

            input = 'echo qwertyuiop\necho asdfghjkl\nrecord\nreplay 2\nrecord\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            output = result.stdout.split('\n')
            if not output[0] == prompt + 'qwertyuiop' and output[1] == prompt + 'asdfghjkl':
                self.accept[2] = False
                return
            if not output[2].find('1') > 0 and output[2].find('echo qwertyuiop') > 0:
                self.accept[2] = False
                return
            if not output[3].find('2') > 0 and output[3].find('echo asdfghjkl') > 0:
                self.accept[2] = False
                return
            if not output[4].find('3') > 0 and output[4].find('record') > 0:
                self.accept[2] = False
                return
            if not output[5] == prompt + 'asdfghjkl':
                self.accept[2] = False
                return
            if not output[6].find('1') > 0 and output[6].find('echo qwertyuiop') > 0:
                self.accept[2] = False
                return
            if not output[7].find('2') > 0 and output[7].find('echo asdfghjkl') > 0:
                self.accept[2] = False
                return
            if not output[8].find('3') > 0 and output[8].find('record') > 0:
                self.accept[2] = False
                return
            if not output[9].find('4') > 0 and output[9].find('echo asdfghjkl') > 0:
                self.accept[2] = False
                return
            if not output[10].find('5') > 0 and output[10].find('record') > 0:
                self.accept[2] = False
                return
        except:
            self.accept[2] = False

    def mypid(self, err):
        try:
            self.accept[3] = True

            p = Popen(executable)
            pid = p.pid + 1
            p.kill()

            input = 'mypid -i\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            if not result.stdout.find(str(pid)) > 0:
                self.accept[3] = False
                return

            p = Popen(executable)
            pid = p.pid + 1
            p.kill()
            ppid = os.getpid()
            input = 'mypid -i\nmypid -p ' + str(pid) + '\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            if not result.stdout.find(str(ppid)) > 0:
                self.accept[3] = False
                return

            p = Popen(executable)
            pid = p.pid + 1
            p.kill()
            ppid = os.getpid()
            input = 'mypid -c ' + str(ppid) + '\nexit\n'
            result = run([executable, algo], stdout=PIPE, 
                        input=input, encoding='ascii')
            if err:
                print(result.stdout)
            if not result.stdout.find(str(pid)) > 0:
                self.accept[3] = False
                return
        except:
            self.accept[3] = False

def judge_all_tests(args):
    part1 = Part1()
    part1.test1(args[0])
    part1.test2(args[1])
    part1.test3(args[2])
    part1.test4(args[3])
    part1.test5(args[4])

    part2 = Part2()
    part2.cd(args[5])
    part2.echo(args[6])
    part2.record(args[7])
    part2.mypid(args[8])

    return part1.accept, part2.accept

def print_result(result):
    part1_command = ["external command", "background execution", "input redirection", "output redirection", "pipe"]
    part2_command = ["cd", "echo", "record and replay", "mypid"]
    command = [part1_command, part2_command]

    if not any(result[0]) or not any(result[1]):
        print("The shell is broken. \n\nProblems can occur in:")
    else:
        print("The shell works properly.")
        return
    
    for j in range(2):
        for i, res in enumerate(result[j]):
            if not res:
                print("    " + command[j][i])

if __name__ == '__main__':
    if not exists(executable):
        print("The executable file is not existed. Please compile the source code first.")
        sys.exit(0)
    
    prompt = run([executable, algo], stdout=PIPE, 
                  input='exit\n', encoding='ascii').stdout

    args = [False] * 9
    if len(sys.argv) > 2 and sys.argv[1] == "all":
        args = [True] * 9
    else:
        for i in sys.argv:
            if i == str(1.1):
                args[0] = True
            if i == str(1.2):
                args[1] = True
            if i == str(1.3):
                args[2] = True
            if i == str(1.4):
                args[3] = True
            if i == str(1.5):
                args[4] = True
            if i == str(2.1):
                args[5] = True
            if i == str(2.2):
                args[6] = True
            if i == str(2.3):
                args[7] = True
            if i == str(2.4):
                args[8] = True

    result = judge_all_tests(args)
    print_result(result)
    
    if exists('out'):
        os.remove('out')
