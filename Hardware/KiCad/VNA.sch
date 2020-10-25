EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 11
Title "VNA2"
Date "2020-10-25"
Rev "0.1"
Comp "Design by Jan Käberich (jankae)"
Comment1 "Ported from Eagle to KiCad by B.VERNOUX Oct 2020"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 1640 2370 500  150 
U 5F7F4FB8
F0 "VNA_1" 50
F1 "VNA_1.sch" 50
$EndSheet
$Sheet
S 3640 2370 500  150 
U 5F7F56B2
F0 "CLK Generation" 50
F1 "CLK_Generation.sch" 50
$EndSheet
$Sheet
S 5640 2370 500  150 
U 5F7F5A73
F0 "Source" 50
F1 "Source.sch" 50
$EndSheet
$Sheet
S 7640 2370 500  150 
U 5F7F60D7
F0 "VNA_4" 50
F1 "VNA_4.sch" 50
$EndSheet
$Sheet
S 9640 2370 500  150 
U 5F7F67E0
F0 "1. LO" 50
F1 "1._LO.sch" 50
$EndSheet
$Sheet
S 1640 4370 500  150 
U 5F7F6B65
F0 "Port 1" 50
F1 "Port_1.sch" 50
$EndSheet
$Sheet
S 3640 4370 500  150 
U 5F7F7313
F0 "Port 2" 50
F1 "Port_2.sch" 50
$EndSheet
$Sheet
S 5640 4370 500  150 
U 5F7F7A67
F0 "Ref Signal" 50
F1 "Ref_Signal.sch" 50
$EndSheet
$Sheet
S 7640 4370 500  150 
U 5F7F8071
F0 "VNA_9" 50
F1 "VNA_9.sch" 50
$EndSheet
$Sheet
S 9640 4370 500  150 
U 5F7F866C
F0 "VNA_10" 50
F1 "VNA_10.sch" 50
$EndSheet
$Comp
L power:GND #PWR0101
U 1 1 5FC5E1C2
P 1930 5910
F 0 "#PWR0101" H 1930 5660 50  0001 C CNN
F 1 "GND" H 1935 5737 50  0000 C CNN
F 2 "" H 1930 5910 50  0001 C CNN
F 3 "" H 1930 5910 50  0001 C CNN
	1    1930 5910
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0114
U 1 1 5FC5E4FB
P 1930 5800
F 0 "#FLG0114" H 1930 5875 50  0001 C CNN
F 1 "PWR_FLAG" H 1930 5973 50  0000 C CNN
F 2 "" H 1930 5800 50  0001 C CNN
F 3 "~" H 1930 5800 50  0001 C CNN
	1    1930 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1930 5910 1930 5800
$Comp
L VNA:Shield Shield_Top1
U 1 1 5F98D376
P 1945 6415
F 0 "Shield_Top1" H 2123 6336 50  0000 L CNN
F 1 "Shield" H 2123 6245 50  0000 L CNN
F 2 "VNA:Shield_Top" H 1945 6415 50  0001 C CNN
F 3 "" H 1945 6415 50  0001 C CNN
	1    1945 6415
	1    0    0    -1  
$EndComp
$Comp
L VNA:Shield Shield_Bottom1
U 1 1 5F98D84A
P 2905 6410
F 0 "Shield_Bottom1" H 3083 6331 50  0000 L CNN
F 1 "Shield" H 3083 6240 50  0000 L CNN
F 2 "VNA:Shield_Bottom" H 2905 6410 50  0001 C CNN
F 3 "" H 2905 6410 50  0001 C CNN
	1    2905 6410
	1    0    0    -1  
$EndComp
$EndSCHEMATC
