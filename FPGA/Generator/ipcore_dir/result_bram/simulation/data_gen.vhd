
--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7_3 Core - Data Generator
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
-- Filename: data_gen.vhd
--
-- Description:
--   Data Generator
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

ENTITY DATA_GEN IS
  GENERIC ( DATA_GEN_WIDTH : INTEGER := 32;
            DOUT_WIDTH     : INTEGER := 32;
            DATA_PART_CNT  : INTEGER := 1;
            SEED           : INTEGER := 2
  );
        
  PORT (
        CLK      : IN STD_LOGIC;
        RST      : IN STD_LOGIC;
        EN       : IN STD_LOGIC; 
        DATA_OUT : OUT STD_LOGIC_VECTOR (DOUT_WIDTH-1 DOWNTO 0)   --OUTPUT VECTOR           
  );
END DATA_GEN;

ARCHITECTURE DATA_GEN_ARCH OF DATA_GEN IS
  CONSTANT LOOP_COUNT     : INTEGER   := DIVROUNDUP(DATA_GEN_WIDTH,8);
  SIGNAL   RAND_DATA      : STD_LOGIC_VECTOR(8*LOOP_COUNT-1 DOWNTO 0);
  SIGNAL   LOCAL_DATA_OUT : STD_LOGIC_VECTOR(DATA_GEN_WIDTH-1 DOWNTO 0);
  SIGNAL   LOCAL_CNT      : INTEGER   :=1;
  SIGNAL   DATA_GEN_I     : STD_LOGIC :='0';
BEGIN

  LOCAL_DATA_OUT <= RAND_DATA(DATA_GEN_WIDTH-1 DOWNTO 0);
  DATA_OUT       <= LOCAL_DATA_OUT(((DOUT_WIDTH*LOCAL_CNT)-1) DOWNTO ((DOUT_WIDTH*LOCAL_CNT)-DOUT_WIDTH));
  DATA_GEN_I     <= '0' WHEN (LOCAL_CNT < DATA_PART_CNT) ELSE EN;

  PROCESS(CLK)
  BEGIN
     IF(RISING_EDGE (CLK)) THEN
        IF(EN ='1' AND (DATA_PART_CNT =1)) THEN
           LOCAL_CNT <=1;
        ELSIF(EN='1' AND (DATA_PART_CNT>1)) THEN
           IF(LOCAL_CNT = 1) THEN
              LOCAL_CNT <= LOCAL_CNT+1;
           ELSIF(LOCAL_CNT < DATA_PART_CNT) THEN
              LOCAL_CNT <= LOCAL_CNT+1;
           ELSE
              LOCAL_CNT <= 1;
           END IF;
        ELSE
          LOCAL_CNT <= 1;
        END IF;
     END IF;
  END PROCESS;

  RAND_GEN:FOR N IN LOOP_COUNT-1 DOWNTO 0 GENERATE
    RAND_GEN_INST:ENTITY work.RANDOM
       GENERIC MAP(
           WIDTH => 8,
           SEED => (SEED+N) 
       )
       PORT MAP(
          CLK => CLK,
	      RST => RST,
          EN  => DATA_GEN_I,
          RANDOM_NUM => RAND_DATA(8*(N+1)-1 DOWNTO 8*N)
       );
  END GENERATE RAND_GEN;

END ARCHITECTURE;

