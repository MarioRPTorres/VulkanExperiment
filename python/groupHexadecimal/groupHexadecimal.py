
import sys
import os 
import subprocess

shaderCodePath = './shaderCode.spv'


def main():
    argc = len(sys.argv)
    argv = sys.argv
    if (argc < 2):
        print("Invalid arguments\nExpected format: python groupHexadecimal.py [path/to/shaderSourceFile] [step] [lineStep]\n")
        return 
    
    if (argc == 3):
        step = int(argv[2])
        lineStep = 8
    elif (argc == 4):
        step = int(argv[2])
        lineStep = int(argv[3])
    else:
        step = 4
        lineStep = 8
    
    sourceCodePath = sys.argv[1]
    #os.system('./glslc.exe ' + sourceCodePath + ' -o ' + shaderCodePath)
    cmd = 'glslc.exe ' + sourceCodePath + ' -o ' + shaderCodePath 
    print(cmd)
    proc = subprocess.Popen(cmd, 
    shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while proc.poll() is None:
          print(proc.stdout.readline()) #give output from your execution/your own message
    #self.commandResult = proc.wait() #catch return code 
    
    with open(shaderCodePath,"rb") as f:
        out = f.read()
    
    sOut = []
    for o in range(len(out)):
        sOut +=  [f'{out[o]:02x}']
    
    with open("out.txt", "w") as fo:
        uint = []
        for i in range(0,len(sOut),step):
            uint += ["0x" + ''.join(sOut[i:i+step][::-1])]
        
        for i in range(0,len(uint),lineStep):
            line = ", ".join(uint[i:i+lineStep])
            if (i+lineStep < len(uint)):
                line = line + ",\n"
            fo.write(line)
    #os.remove(shaderCodePath)

if __name__=="__main__":
    main()