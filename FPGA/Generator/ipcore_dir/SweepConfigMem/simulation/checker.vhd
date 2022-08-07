
--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7_3 Core - Checker
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
-- Filename: checker.vhd
--
-- Description:
--   Checker
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

LIBRARY work;
USE work.BMG_TB_PKG.ALL;

ENTITY CHECKER IS
  GENERIC ( WRITE_WIDTH : INTEGER :=32;
            READ_WIDTH  : INTEGER :=32
  );
        
  PORT (
        CLK      : IN STD_LOGIC;
        RST      : IN STD_LOGIC;
        EN       : IN STD_LOGIC; 
        DATA_IN  : IN STD_LOGIC_VECTOR (READ_WIDTH-1 DOWNTO 0);   --OUTPUT VECTOR          
        STATUS   : OUT STD_LOGIC:= '0'
  );
END CHECKER;

ARCHITECTURE CHECKER_ARCH OF CHECKER IS
  SIGNAL EXPECTED_DATA : STD_LOGIC_VECTOR(READ_WIDTH-1 DOWNTO 0);
  SIGNAL DATA_IN_R: STD_LOGIC_VECTOR(READ_WIDTH-1 DOWNTO 0);
  SIGNAL EN_R : STD_LOGIC := '0';
  SIGNAL EN_2R : STD_LOGIC := '0';
--DATA PART CNT DEFINES THE ASPECT RATIO AND GIVES THE INFO TO THE DATA GENERATOR TO PROVIDE THE DATA EITHER IN PARTS OR COMPLETE DATA IN ONE SHOT
--IF READ_WIDTH > WRITE_WIDTH DIVROUNDUP RESULTS IN '1' AND DATA GENERATOR GIVES THE DATAOUT EQUALS TO MAX OF (WRITE_WIDTH, READ_WIDTH)
--IF READ_WIDTH < WRITE-WIDTH DIVROUNDUP RESULTS IN > '1' AND DATA GENERATOR GIVES THE DATAOUT IN TERMS OF PARTS(EG 4 PARTS WHEN WRITE_WIDTH 32 AND READ WIDTH 8)
  CONSTANT DATA_PART_CNT: INTEGER:= DIVROUNDUP(WRITE_WIDTH,READ_WIDTH);
  CONSTANT MAX_WIDTH: INTEGER:= IF_THEN_ELSE((WRITE_WIDTH>READ_WIDTH),WRITE_WIDTH,READ_WIDTH);
  SIGNAL ERR_HOLD : STD_LOGIC :='0';
  SIGNAL ERR_DET : STD_LOGIC :='0';
BEGIN
   PROCESS(CLK)
   BEGIN
     IF(RISING_EDGE(CLK)) THEN
       IF(RST= '1') THEN
     	  EN_R <= '0';
     	  EN_2R <= '0';
          DATA_IN_R <= (OTHERS=>'0');
       ELSE
	      EN_R <= EN;
	      EN_2R <= EN_R;
          DATA_IN_R <= DATA_IN;
       END IF;        
     END IF;
   END PROCESS;

   EXPECTED_DATA_GEN_INST:ENTITY work.DATA_GEN 
      GENERIC MAP ( DATA_GEN_WIDTH =>MAX_WIDTH,
                    DOUT_WIDTH     => READ_WIDTH,
         		    DATA_PART_CNT  => DATA_PART_CNT,
	                SEED           => 2
      )
      PORT MAP (
            CLK      => CLK,
			RST      => RST,
            EN       => EN_2R,
            DATA_OUT => EXPECTED_DATA
	  );

   PROCESS(CLK)
   BEGIN
      IF(RISING_EDGE(CLK)) THEN
         IF(EN_2R='1') THEN
        	 IF(EXPECTED_DATA = DATA_IN_R) THEN
	            ERR_DET<='0';
        	 ELSE
	            ERR_DET<= '1';
        	 END IF;
         END IF;
     END IF;
   END PROCESS;

   PROCESS(CLK,RST)
   BEGIN
	  IF(RST='1') THEN
		 ERR_HOLD <= '0';
      ELSIF(RISING_EDGE(CLK)) THEN
         ERR_HOLD <= ERR_HOLD  OR ERR_DET ;
      END IF;
   END PROCESS;

   STATUS <= ERR_HOLD;

END ARCHITECTURE;



