--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   00:51:28 05/10/2022
-- Design Name:   
-- Module Name:   /home/jan/Projekte/LibreVNA/FPGA/Generator/Test_RationalApproximation.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: RationalApproximation
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
 
ENTITY Test_RationalApproximation IS
END Test_RationalApproximation;
 
ARCHITECTURE behavior OF Test_RationalApproximation IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT RationalApproximation
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         NUM : OUT  std_logic_vector(11 downto 0);
         DENOM : OUT  std_logic_vector(11 downto 0);
         RATIO : IN  std_logic_vector(26 downto 0);
         START : IN  std_logic;
         READY : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal RATIO : std_logic_vector(26 downto 0) := (others => '0');
   signal START : std_logic := '0';

 	--Outputs
   signal NUM : std_logic_vector(11 downto 0);
   signal DENOM : std_logic_vector(11 downto 0);
   signal READY : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: RationalApproximation PORT MAP (
          CLK => CLK,
          RESET => RESET,
          NUM => NUM,
          DENOM => DENOM,
          RATIO => RATIO,
          START => START,
          READY => READY
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
		RESET <= '1';
      wait for 100 ns;	
		RESET <= '0';
      wait for CLK_period*10;

      -- insert stimulus here 
		RATIO <= b"001_0000_0000_0000_0000_0000_0000";
		START <= '1';
		
		wait until READY = '1';
		
		wait for CLK_period*10;
		START <= '0';
		wait for CLK_period*10;
		RATIO <= b"001_1011_1110_1010_0101_0000_0000";
		START <= '1';

      wait;
   end process;

END;
