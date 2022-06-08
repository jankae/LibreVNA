--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   13:39:25 09/16/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA2/FPGA/VNA/Test_Window.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: window
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
 
ENTITY Test_Window IS
END Test_Window;
 
ARCHITECTURE behavior OF Test_Window IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT window
    PORT(
         CLK : IN  std_logic;
         INDEX : IN  std_logic_vector(6 downto 0);
         WINDOW_TYPE : IN  std_logic_vector(1 downto 0);
         VALUE : OUT  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal INDEX : std_logic_vector(6 downto 0) := (others => '0');
   signal WINDOW_TYPE : std_logic_vector(1 downto 0) := (others => '0');

 	--Outputs
   signal VALUE : std_logic_vector(15 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: window PORT MAP (
          CLK => CLK,
          INDEX => INDEX,
          WINDOW_TYPE => WINDOW_TYPE,
          VALUE => VALUE
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
		WINDOW_TYPE <= "00";
		INDEX <= "0000000";
		wait for CLK_period*10;

		WINDOW_TYPE <= "10";
      -- insert stimulus here 
		wait for CLK_period*10;
		INDEX <= "0000001";
		wait for CLK_period*10;
		INDEX <= "0000010";
		wait for CLK_period*10;
		INDEX <= "0000011";
		wait for CLK_period*10;
		INDEX <= "0000100";

      wait;
   end process;

END;
