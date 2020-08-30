--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   14:55:06 05/10/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_Sync.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: SwitchingSync
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
 
ENTITY Test_Sync IS
END Test_Sync;
 
ARCHITECTURE behavior OF Test_Sync IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SwitchingSync
	 Generic (CLK_DIV : integer);
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         SETTING : IN  std_logic_vector(1 downto 0);
         SYNC_OUT : OUT  std_logic;
         SYNC_PULSE_IN : IN  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal SETTING : std_logic_vector(1 downto 0) := (others => '0');
   signal SYNC_PULSE_IN : std_logic := '0';

 	--Outputs
   signal SYNC_OUT : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 6.25 ns;
	constant SYNC_PULSE_period : time := 1031.25 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: SwitchingSync
	GENERIC MAP (CLK_DIV => 160)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          SETTING => SETTING,
          SYNC_OUT => SYNC_OUT,
          SYNC_PULSE_IN => SYNC_PULSE_IN
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
	
	SYNC_process :process
   begin
		SYNC_PULSE_IN <= '1';
		wait for CLK_period;
		SYNC_PULSE_IN <= '0';
		wait for SYNC_PULSE_period - CLK_period;
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
		SETTING <= "00";
		wait for CLK_period*1600;
		SETTING <= "01";
		wait for CLK_period*1600;
		SETTING <= "10";
		wait for CLK_period*1600;
      wait;
   end process;

END;
