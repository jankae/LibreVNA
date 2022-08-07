-- file: DFT_CLK_tb.vhd
-- 
-- (c) Copyright 2008 - 2011 Xilinx, Inc. All rights reserved.
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
-- 

------------------------------------------------------------------------------
-- Clocking wizard demonstration testbench
------------------------------------------------------------------------------
-- This demonstration testbench instantiates the example design for the 
--   clocking wizard. Input clocks are toggled, which cause the clocking
--   network to lock and the counters to increment.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;

library std;
use std.textio.all;

library work;
use work.all;

entity DFT_CLK_tb is
end DFT_CLK_tb;

architecture test of DFT_CLK_tb is

  -- Clock to Q delay of 100 ps
  constant TCQ               : time := 100 ps;
  -- timescale is 1ps
  constant ONE_NS      : time := 1 ns;
  -- how many cycles to run
  constant COUNT_PHASE : integer := 1024 + 1;


  -- we'll be using the period in many locations
  constant PER1        : time := 62.5 ns;


  -- Declare the input clock signals
  signal CLK_IN1       : std_logic := '1';
  -- The high bit of the sampling counter
  signal COUNT         : std_logic;
  -- Status and control signals
  signal RESET         : std_logic := '0';
  signal LOCKED        : std_logic;
  signal COUNTER_RESET : std_logic := '0';
--  signal defined to stop mti simulation without severity failure in the report
  signal end_of_sim : std_logic := '0';
  signal CLK_OUT : std_logic_vector(1 downto 1);
--Freq Check using the M & D values setting and actual Frequency generated

component DFT_CLK_exdes
generic (
  TCQ               : in time := 100 ps);
port
 (-- Clock in ports
  CLK_IN1           : in  std_logic;
  -- Reset that only drives logic in example design
  COUNTER_RESET     : in  std_logic;
  CLK_OUT           : out std_logic_vector(1 downto 1) ;
  -- High bits of counters driven by clocks
  COUNT             : out std_logic;
  -- Status and control signals
  RESET             : in  std_logic;
  LOCKED            : out std_logic
 );
end component;

begin

  -- Input clock generation
  --------------------------------------
  process begin
    CLK_IN1 <= not CLK_IN1; wait for (PER1/2);
  end process;

  -- Test sequence
  process 

    procedure simtimeprint is
      variable outline : line;
    begin
      write(outline, string'("## SYSTEM_CYCLE_COUNTER "));
      write(outline, NOW/PER1);
      write(outline, string'(" ns"));
      writeline(output,outline);
    end simtimeprint;

    procedure simfreqprint (period : time; clk_num : integer) is
       variable outputline : LINE;
       variable str1 : string(1 to 16);
       variable str2 : integer;
       variable str3 : string(1 to 2);
       variable str4 : integer;
       variable str5 : string(1 to 4);
    begin
       str1 := "Freq of CLK_OUT(";
       str2 :=  clk_num;
       str3 :=  ") ";
       str4 :=  1000000 ps/period ;
       str5 :=  " MHz" ;
       write(outputline, str1 );
       write(outputline, str2);
       write(outputline, str3);
       write(outputline, str4);
       write(outputline, str5);
       writeline(output, outputline);
    end simfreqprint;

  begin
    RESET      <= '1';
    wait for (PER1*6);
    RESET      <= '0';
    wait until LOCKED = '1';
    COUNTER_RESET <= '1';
    wait for (PER1*20);
    COUNTER_RESET <= '0';
    wait for (PER1*COUNT_PHASE);


    simtimeprint;
    end_of_sim <= '1';
    wait for 1 ps;
    report "Simulation Stopped." severity failure;
    wait;
  end process;

  -- Instantiation of the example design containing the clock
  --    network and sampling counters
  -----------------------------------------------------------
  dut : DFT_CLK_exdes
  generic map (
    TCQ                => TCQ)
  port map
   (-- Clock in ports
    CLK_IN1            => CLK_IN1,
    -- Reset for logic in example design
    COUNTER_RESET      => COUNTER_RESET,
    CLK_OUT            => CLK_OUT,
    -- High bits of the counters
    COUNT              => COUNT,
    -- Status and control signals
    RESET              => RESET,
    LOCKED             => LOCKED);

-- Freq Check 

end test;
