--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   10:46:34 05/07/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_MAX2871.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: MAX2871
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
 
ENTITY Test_MAX2871 IS
END Test_MAX2871;
 
ARCHITECTURE behavior OF Test_MAX2871 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT MAX2871
	 Generic (CLK_DIV : integer);
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         REG4 : IN  std_logic_vector(31 downto 0);
         REG3 : IN  std_logic_vector(31 downto 0);
         REG1 : IN  std_logic_vector(31 downto 0);
         REG0 : IN  std_logic_vector(31 downto 0);
         RELOAD : IN  std_logic;
         CLK_OUT : OUT  std_logic;
         MOSI : OUT  std_logic;
         LE : OUT  std_logic;
         DONE : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal REG4 : std_logic_vector(31 downto 0) := (others => '0');
   signal REG3 : std_logic_vector(31 downto 0) := (others => '0');
   signal REG1 : std_logic_vector(31 downto 0) := (others => '0');
   signal REG0 : std_logic_vector(31 downto 0) := (others => '0');
   signal RELOAD : std_logic := '0';

 	--Outputs
   signal CLK_OUT : std_logic;
   signal MOSI : std_logic;
   signal LE : std_logic;
   signal DONE : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 6.25 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: MAX2871
	GENERIC MAP(CLK_DIV => 10)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          REG4 => REG4,
          REG3 => REG3,
          REG1 => REG1,
          REG0 => REG0,
          RELOAD => RELOAD,
          CLK_OUT => CLK_OUT,
          MOSI => MOSI,
          LE => LE,
          DONE => DONE
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
		REG4 <= "11111111000000001111111100000000";
		REG3 <= "11110000111100001111000011110000";
		REG1 <= "11001100110011001100110011001100";
		REG0 <= "10101010101010101010101010101010";
		RELOAD <= '1';
		wait for CLK_period;
		RELOAD <= '0';
      wait;
   end process;

END;
