
--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7_3 Core - Random Number Generator
--
--------------------------------------------------------------------------------
--
-- (c) Copyright 2006_3010 Xilinx, Inc. All rights reserved.
--
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
--
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
--
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
--
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.

--------------------------------------------------------------------------------
--
-- Filename: random.vhd
--
-- Description:
--   Random Generator
--
--------------------------------------------------------------------------------
-- Author: IP Solutions Division
--
-- History: Sep 12, 2011 - First Release
--------------------------------------------------------------------------------
--
--------------------------------------------------------------------------------
-- Library Declarations
--------------------------------------------------------------------------------




LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;


ENTITY RANDOM IS
  GENERIC ( WIDTH : INTEGER :=  32;
 	        SEED  : INTEGER :=2 
  );
        
  PORT (
     CLK        : IN STD_LOGIC;
     RST        : IN STD_LOGIC;
     EN         : IN STD_LOGIC;
     RANDOM_NUM : OUT STD_LOGIC_VECTOR (WIDTH-1 DOWNTO 0)   --OUTPUT VECTOR           
  );
END RANDOM;

ARCHITECTURE BEHAVIORAL OF RANDOM IS
BEGIN
  PROCESS(CLK)
    VARIABLE RAND_TEMP : STD_LOGIC_VECTOR(WIDTH-1 DOWNTO 0):=CONV_STD_LOGIC_VECTOR(SEED,WIDTH);
    VARIABLE TEMP      : STD_LOGIC := '0';
  BEGIN
    IF(RISING_EDGE(CLK)) THEN
       IF(RST='1') THEN
          RAND_TEMP  := CONV_STD_LOGIC_VECTOR(SEED,WIDTH);
	   ELSE
          IF(EN = '1') THEN
             TEMP                        := RAND_TEMP(WIDTH-1) XOR RAND_TEMP(WIDTH-2);
             RAND_TEMP(WIDTH-1 DOWNTO 1) := RAND_TEMP(WIDTH-2 DOWNTO 0);
             RAND_TEMP(0)                := TEMP;
          END IF;
       END IF;
    END IF;
    RANDOM_NUM <= RAND_TEMP;
  END PROCESS;
END ARCHITECTURE;
