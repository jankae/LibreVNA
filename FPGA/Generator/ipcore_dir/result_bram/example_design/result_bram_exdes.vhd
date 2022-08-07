 
 
 




--------------------------------------------------------------------------------
--
-- BLK MEM GEN v7.1 Core - Top-level core wrapper
--
--------------------------------------------------------------------------------
--
-- (c) Copyright 2006-2010 Xilinx, Inc. All rights reserved.
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
-- Filename: result_bram_exdes.vhd
--
-- Description:
--   This is the actual BMG core wrapper.
--
--------------------------------------------------------------------------------
-- Author: IP Solutions Division
--
-- History: August 31, 2005 - First Release
--------------------------------------------------------------------------------
--
--------------------------------------------------------------------------------
-- Library Declarations
--------------------------------------------------------------------------------

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_ARITH.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;

LIBRARY UNISIM;
USE UNISIM.VCOMPONENTS.ALL;

--------------------------------------------------------------------------------
-- Entity Declaration
--------------------------------------------------------------------------------
ENTITY result_bram_exdes IS
  PORT (
      --Inputs - Port A
  
    WEA            : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    ADDRA          : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
  
    DINA           : IN STD_LOGIC_VECTOR(191 DOWNTO 0);
  
    CLKA       : IN STD_LOGIC;

  
      --Inputs - Port B
    ADDRB          : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    DOUTB          : OUT STD_LOGIC_VECTOR(191 DOWNTO 0);
    CLKB           : IN STD_LOGIC

  );

END result_bram_exdes;


ARCHITECTURE xilinx OF result_bram_exdes IS

  COMPONENT BUFG IS
  PORT (
     I      : IN STD_ULOGIC;
     O      : OUT STD_ULOGIC
  );
  END COMPONENT;

  COMPONENT result_bram IS
  PORT (
      --Port A
  
    WEA        : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    ADDRA      : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
  
    DINA       : IN STD_LOGIC_VECTOR(191 DOWNTO 0);

  
    CLKA       : IN STD_LOGIC;

  
      --Port B
    ADDRB      : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    DOUTB      : OUT STD_LOGIC_VECTOR(191 DOWNTO 0);
    CLKB       : IN STD_LOGIC


  );
  END COMPONENT;

  SIGNAL CLKA_buf     : STD_LOGIC;
  SIGNAL CLKB_buf     : STD_LOGIC;
  SIGNAL S_ACLK_buf   : STD_LOGIC;

BEGIN

  bufg_A : BUFG
    PORT MAP (
     I => CLKA,
     O => CLKA_buf
     );

  bufg_B : BUFG
    PORT MAP (
     I => CLKB,
     O => CLKB_buf
     );


  bmg0 : result_bram
    PORT MAP (
      --Port A
  
      WEA        => WEA,
      ADDRA      => ADDRA,
  
      DINA       => DINA,

      CLKA       => CLKA_buf,

  
      --Port B
      ADDRB      => ADDRB,
      DOUTB      => DOUTB,
      CLKB       => CLKB_buf

    );

END xilinx;
