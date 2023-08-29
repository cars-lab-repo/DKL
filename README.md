# Dual-Key-Lock </br> 
This repository contains resources related to a Dual Key logic locking scheme. </br>
Jordan Maynard & Amin Rezaei </br>

# Contents
* `paper`: published paper based on the source code </br>
* `src`: source code, executable, and benchmarks </br>
* `src/encrypted`: locked benchmarks - output location of the tool </br>
* `src/modules` : benchmark files containing important structures of DK Lock </br>

# Compiling
1. g++ version 8.1.0 & Windows 10/11 is used to compile. To download g++, follow [this link](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z/download).
2. Run `cd src` to navigate to the src folder.
3. Run `g++ dkl.cpp circuit.cpp` in terminal to compile and create executable file.
4. Run `./a.exe` to use script. </br>
- NOTE: A pre-compiled executable is included (dkl.exe) but it is recommended the user compile the code on their own.

# Usage
1. Run `./a.exe`.
2. Input benchmark file name, omitting ".bench". *Example: `iscas89/s27` NOT ~~`iscas89/s27.bench`~~*
3. Choose between no scan exposed, sequential scan exposed, or combinational scan exposed.
4. Choose key size and key values.
5. Choose number of cycles before activation.
- NOTE: If the input benchmark is inside a folder, a folder of the same name must exist in the /encrypted/ folder for the script to create an output. 
- *Example: Input file `iscas89/s27` -> Output file `encrypted/iscas89/s27_enc10.bench`*

# Citation
```
@INPROCEEDINGS{DK-Lock,
  author={Maynard, Jordan and Rezaei, Amin},
  booktitle={2023 24th International Symposium on Quality Electronic Design (ISQED)}, 
  title={DK Lock: Dual Key Logic Locking Against Oracle-Guided Attacks}, 
  year={2023},
  volume={},
  number={},
  pages={1-7},
  doi={10.1109/ISQED57927.2023.10129368}
  }
