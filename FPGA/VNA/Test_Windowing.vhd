--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   18:37:51 11/06/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA2/FPGA/VNA/Test_Windowing.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Windowing
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
 
ENTITY Test_Windowing IS
END Test_Windowing;
 
ARCHITECTURE behavior OF Test_Windowing IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Windowing
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         WINDOW_TYPE : IN  std_logic_vector(1 downto 0);
         PORT1_RAW : IN  std_logic_vector(15 downto 0);
         PORT2_RAW : IN  std_logic_vector(15 downto 0);
         REF_RAW : IN  std_logic_vector(15 downto 0);
         ADC_READY : IN  std_logic;
         PORT1_WINDOWED : OUT  std_logic_vector(17 downto 0);
         PORT2_WINDOWED : OUT  std_logic_vector(17 downto 0);
         REF_WINDOWED : OUT  std_logic_vector(17 downto 0);
         WINDOWING_DONE : OUT  std_logic;
         NSAMPLES : IN  std_logic_vector(12 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal WINDOW_TYPE : std_logic_vector(1 downto 0) := (others => '0');
   signal PORT1_RAW : std_logic_vector(15 downto 0) := (others => '0');
   signal PORT2_RAW : std_logic_vector(15 downto 0) := (others => '0');
   signal REF_RAW : std_logic_vector(15 downto 0) := (others => '0');
   signal ADC_READY : std_logic := '0';
   signal NSAMPLES : std_logic_vector(12 downto 0) := (others => '0');

 	--Outputs
   signal PORT1_WINDOWED : std_logic_vector(17 downto 0);
   signal PORT2_WINDOWED : std_logic_vector(17 downto 0);
   signal REF_WINDOWED : std_logic_vector(17 downto 0);
   signal WINDOWING_DONE : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Windowing PORT MAP (
          CLK => CLK,
          RESET => RESET,
          WINDOW_TYPE => WINDOW_TYPE,
          PORT1_RAW => PORT1_RAW,
          PORT2_RAW => PORT2_RAW,
          REF_RAW => REF_RAW,
          ADC_READY => ADC_READY,
          PORT1_WINDOWED => PORT1_WINDOWED,
          PORT2_WINDOWED => PORT2_WINDOWED,
          REF_WINDOWED => REF_WINDOWED,
          WINDOWING_DONE => WINDOWING_DONE,
          NSAMPLES => NSAMPLES
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

      wait for CLK_period*10;

      -- insert stimulus here 
		WINDOW_TYPE <= "10";
		NSAMPLES <= "0000000010001";
		PORT1_RAW <= "0000000010000000";
		PORT2_RAW <= "0000000100000000";
		REF_RAW <= "0000001000000000";
		ADC_READY <= '0';
		RESET <= '1';
		wait for CLK_period;
		RESET <= '0';
		for i in 0 to 271	loop
			wait for CLK_period*111;
			ADC_READY <= '1';
			wait for CLK_period;
			ADC_READY <= '0';
		end loop;

      wait;
   end process;

END;
