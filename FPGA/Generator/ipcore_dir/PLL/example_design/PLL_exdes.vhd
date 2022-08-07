-- file: PLL_exdes.vhd
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
-- Clocking wizard example design
------------------------------------------------------------------------------
-- This example design instantiates the created clocking network, where each
--   output clock drives a counter. The high bit of each counter is ported.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

library unisim;
use unisim.vcomponents.all;

entity PLL_exdes is
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
end PLL_exdes;

architecture xilinx of PLL_exdes is

  -- Parameters for the counters
  ---------------------------------
  -- Counter width
  constant C_W        : integer := 16;


  -- When the clock goes out of lock, reset the counters
  signal   locked_int : std_logic;
  signal   reset_int  : std_logic                     := '0';

  -- Declare the clocks and counter
  signal   clk        : std_logic;
  signal   clk_int    : std_logic;
  signal   clk_n  : std_logic;
  signal   counter    : std_logic_vector(C_W-1 downto 0) := (others => '0');
  signal rst_sync : std_logic;
  signal rst_sync_int : std_logic;
  signal rst_sync_int1 : std_logic;
  signal rst_sync_int2 : std_logic;


component PLL is
port
 (-- Clock in ports
  CLK_IN1           : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  -- Status and control signals
  RESET             : in     std_logic;
  LOCKED            : out    std_logic
 );
end component;

begin
  -- Alias output to internally used signal
  LOCKED    <= locked_int;

  -- When the clock goes out of lock, reset the counters
  reset_int <= (not locked_int) or RESET or COUNTER_RESET;


 process (clk, reset_int) begin
   if (reset_int = '1') then
       rst_sync <= '1';
       rst_sync_int <= '1';
       rst_sync_int1 <= '1';
       rst_sync_int2 <= '1';
   elsif (clk 'event and clk='1') then
       rst_sync <= '0';
       rst_sync_int <= rst_sync;
       rst_sync_int1 <= rst_sync_int;
       rst_sync_int2 <= rst_sync_int1;
   end if;
 end process;


  -- Instantiation of the clocking network
  ----------------------------------------
  clknetwork : PLL
  port map
   (-- Clock in ports
    CLK_IN1            => CLK_IN1,
    -- Clock out ports
    CLK_OUT1           => clk_int,
    -- Status and control signals
    RESET              => RESET,
    LOCKED             => locked_int);

  clk_n <= not clk;
  clkout_oddr : ODDR2
  port map
   (Q  => CLK_OUT(1),
    C0 => clk,
    C1 => clk_n,
    CE => '1',
    D0 => '1',
    D1 => '0',
    R  => '0',
    S  => '0');

  -- Connect the output clocks to the design
  -------------------------------------------
  clk <= clk_int;

  -- Output clock sampling
  -------------------------------------
  process (clk, rst_sync_int2) begin
      if (rst_sync_int2 = '1') then
        counter <= (others => '0') after TCQ;
      elsif (rising_edge(clk)) then
        counter <= counter + 1 after TCQ;
      end if;
  end process;
  -- alias the high bit to the output
  COUNT <= counter(C_W-1);


end xilinx;
