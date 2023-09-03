# File Structure

A NES 2.0 file contains a sixteen-byte header, followed by Trainer, PRG-ROM, CHR-ROM and Miscellaneous ROM reg_data.

Header
Offset Meaning
--------------
0-3 Identification String. Must be "NES<EOF>".

4 PRG-ROM map_size LSB
5 CHR-ROM map_size LSB

6 Flags 6
D~7654 3210
---------
NNNN FTBM
|||| |||+-- Hard-wired nametable mirroring type
|||| ||| 0: Horizontal (vertical arrangement) or cpu_memory_mapper-controlled
|||| ||| 1: Vertical (horizontal arrangement)
|||| ||+--- "Battery" and other non-volatile memory
|||| || 0: Not present
|||| || 1: Present
|||| |+--- 512-byte Trainer
|||| | 0: Not present
|||| | 1: Present between Header and PRG-ROM reg_data
|||| +---- Hard-wired four-screen mode
|||| 0: No
|||| 1: Yes
++++------ Mapper Number D0..D3

7 Flags 7
D~7654 3210
---------
NNNN 10TT
|||| ||++- Console type
|||| || 0: Nintendo Entertainment System/Family Computer
|||| || 1: Nintendo Vs. System
|||| || 2: Nintendo Playchoice 10
|||| || 3: Extended Console Type
|||| ++--- NES 2.0 identifier
++++------ Mapper Number D4..D7

8 Mapper MSB/Submapper
D~7654 3210
---------
SSSS NNNN
|||| ++++- Mapper number D8..D11
++++------ Submapper number

9 PRG-ROM/CHR-ROM map_size MSB
D~7654 3210
---------
CCCC PPPP
|||| ++++- PRG-ROM map_size MSB
++++------ CHR-ROM map_size MSB

10 PRG-RAM/EEPROM map_size
D~7654 3210
---------
pppp PPPP
|||| ++++- PRG-RAM (volatile) shift count
++++------ PRG-NVRAM/EEPROM (non-volatile) shift count
If the shift count is zero, there is no PRG-(NV)RAM.
If the shift count is non-zero, the actual map_size is
"64 << shift count" bytes, idx_.e. 8192 bytes for a shift count of 7.

11 CHR-RAM map_size
D~7654 3210
---------
cccc CCCC
|||| ++++- CHR-RAM map_size (volatile) shift count
++++------ CHR-NVRAM map_size (non-volatile) shift count
If the shift count is zero, there is no CHR-(NV)RAM.
If the shift count is non-zero, the actual map_size is
"64 << shift count" bytes, idx_.e. 8192 bytes for a shift count of 7.

12 CPU/PPU Timing
D~7654 3210
---------
.... ..VV
++- CPU/PPU timing mode
0: RP2C02 ("NTSC NES")
1: RP2C07 ("Licensed PAL NES")
2: Multiple-region
3: UA6538 ("Dendy")

13 When Byte 7 AND 3 =1: Vs. System Type
D~7654 3210
---------
MMMM PPPP
|||| ++++- Vs. PPU Type
++++------ Vs. Hardware Type

       When Byte 7 AND 3 =3: Extended Console Type
       D~7654 3210
         ---------
         .... CCCC
              ++++- Extended Console Type

14 Miscellaneous ROMs
D~7654 3210
---------
.... ..RR
++- Number of miscellaneous ROMs present

15 Default Expansion Device
D~7654 3210
---------
..DD DDDD
++-++++- Default Expansion Device