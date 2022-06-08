--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   12:58:50 05/08/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_PLL.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: PLL
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
 
ENTITY Test_PLL IS
END Test_PLL;
 
ARCHITECTURE behavior OF Test_PLL IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT PLL
    PORT(
         CLK_IN1 : IN  std_logic;
         CLK_OUT1 : OUT  std_logic;
         RESET : IN  std_logic;
         LOCKED : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK_IN1 : std_logic := '0';
   signal RESET : std_logic := '0';

 	--Outputs
   signal CLK_OUT1 : std_logic;
   signal LOCKED : std_logic;

   -- Clock period definitions
   constant CLK_IN1_period : time := 62.5 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: PLL PORT MAP (
          CLK_IN1 => CLK_IN1,
          CLK_OUT1 => CLK_OUT1,
          RESET => RESET,
          LOCKED => LOCKED
        );

   -- Clock process definitions
   CLK_IN1_process :process
   begin
		CLK_IN1 <= '0';
		wait for CLK_IN1_period/2;
		CLK_IN1 <= '1';
		wait for CLK_IN1_period/2;
   end process;
 
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
		RESET <= '1';
      wait for 1000 ns;	
		RESET <= '0';

      wait for CLK_IN1_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
