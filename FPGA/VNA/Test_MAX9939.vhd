--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   21:19:15 05/29/2021
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA2/FPGA/VNA/Test_MAX9939.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: MAX9939
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY Test_MAX9939 IS
END Test_MAX9939;
 
ARCHITECTURE behavior OF Test_MAX9939 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT MAX9939
    GENERIC(CLK_DIV : integer);
	 PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         GAIN : IN  std_logic_vector(3 downto 0);
         READY : OUT  std_logic;
         CS : OUT  std_logic;
         SCLK : OUT  std_logic;
         DIN : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '1';
   signal GAIN : std_logic_vector(3 downto 0) := (others => '0');

 	--Outputs
   signal READY : std_logic;
   signal CS : std_logic;
   signal SCLK : std_logic;
   signal DIN : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: MAX9939
		GENERIC MAP(CLK_DIV => 10)
		PORT MAP (
          CLK => CLK,
          RESET => RESET,
          GAIN => GAIN,
          READY => READY,
          CS => CS,
          SCLK => SCLK,
          DIN => DIN
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;
		RESET <= '0';

      wait for CLK_period*10;

      -- insert stimulus here 
		wait for CLK_period*200;
		GAIN <= "1010";

      wait;
   end process;

END;
