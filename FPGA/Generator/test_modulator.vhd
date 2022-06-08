--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   12:41:17 06/07/2022
-- Design Name:   
-- Module Name:   /home/jan/Projekte/LibreVNA/FPGA/Generator/test_modulator.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Modulator
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
USE ieee.numeric_std.ALL;
 
ENTITY test_modulator IS
END test_modulator;
 
ARCHITECTURE behavior OF test_modulator IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Modulator
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         SAMPLE_FREQ_WORD : IN  std_logic_vector(15 downto 0);
         SAMPLE_DATA : IN  std_logic_vector(7 downto 0);
         SAMPLE_LATCH : IN  std_logic;
         OVERFLOW : OUT  std_logic;
         UNDERFLOW : OUT  std_logic;
         THRESHOLD_LEVEL : IN  std_logic_vector(10 downto 0);
         THRESHOLD_CROSSED : OUT  std_logic;
         FREQ_CENTER : IN  std_logic_vector(32 downto 0);
         FREQ_DEVIATION : IN  std_logic_vector(25 downto 0);
         MIN_ATTENUATION : IN  std_logic_vector(6 downto 0);
         AMPLITUDE_DEPTH : IN  std_logic_vector(6 downto 0);
         FREQUENCY : OUT  std_logic_vector(32 downto 0);
         ATTENUATOR : OUT  std_logic_vector(6 downto 0);
         NEW_OUTPUT : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal SAMPLE_FREQ_WORD : std_logic_vector(15 downto 0) := (others => '0');
   signal SAMPLE_DATA : std_logic_vector(7 downto 0) := (others => '0');
   signal SAMPLE_LATCH : std_logic := '0';
   signal THRESHOLD_LEVEL : std_logic_vector(10 downto 0) := (others => '0');
   signal FREQ_CENTER : std_logic_vector(32 downto 0) := (others => '0');
   signal FREQ_DEVIATION : std_logic_vector(25 downto 0) := (others => '0');
   signal MIN_ATTENUATION : std_logic_vector(6 downto 0) := (others => '0');
   signal AMPLITUDE_DEPTH : std_logic_vector(6 downto 0) := (others => '0');

 	--Outputs
   signal OVERFLOW : std_logic;
   signal UNDERFLOW : std_logic;
   signal THRESHOLD_CROSSED : std_logic;
   signal FREQUENCY : std_logic_vector(32 downto 0);
   signal ATTENUATOR : std_logic_vector(6 downto 0);
   signal NEW_OUTPUT : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Modulator PORT MAP (
          CLK => CLK,
          RESET => RESET,
          SAMPLE_FREQ_WORD => SAMPLE_FREQ_WORD,
          SAMPLE_DATA => SAMPLE_DATA,
          SAMPLE_LATCH => SAMPLE_LATCH,
          OVERFLOW => OVERFLOW,
          UNDERFLOW => UNDERFLOW,
          THRESHOLD_LEVEL => THRESHOLD_LEVEL,
          THRESHOLD_CROSSED => THRESHOLD_CROSSED,
          FREQ_CENTER => FREQ_CENTER,
          FREQ_DEVIATION => FREQ_DEVIATION,
          MIN_ATTENUATION => MIN_ATTENUATION,
          AMPLITUDE_DEPTH => AMPLITUDE_DEPTH,
          FREQUENCY => FREQUENCY,
          ATTENUATOR => ATTENUATOR,
          NEW_OUTPUT => NEW_OUTPUT
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
		RESET <= '1';
      -- hold reset state for 100 ns.
      wait for 100 ns;	

		SAMPLE_FREQ_WORD <= std_logic_vector(to_unsigned(32768, 16));
		THRESHOLD_LEVEL <= std_logic_vector(to_unsigned(5, 11));
		FREQ_CENTER <= std_logic_vector(to_unsigned(134217728, 33));
      FREQ_DEVIATION <= std_logic_vector(to_unsigned(16777216, 26));
      MIN_ATTENUATION <= std_logic_vector(to_unsigned(64, 7));
      AMPLITUDE_DEPTH <= std_logic_vector(to_unsigned(127, 7));
      wait for CLK_period*10;
		-- release reset
		RESET <= '0';
      -- add samples
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(0, 8));
		SAMPLE_LATCH <= '1';
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(10, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(20, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(30, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(40, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(50, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(60, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(70, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(80, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(90, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(100, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(110, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(120, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(130, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(140, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(150, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(160, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(170, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(180, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(190, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(200, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(210, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(220, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(230, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(240, 8));
		wait for CLK_period;
		SAMPLE_DATA <= std_logic_vector(to_unsigned(250, 8));
		wait for CLK_period;
		SAMPLE_LATCH <= '0';

      wait;
   end process;

END;
