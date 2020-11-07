--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   18:40:38 05/05/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_Sampling.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Sampling
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
 
ENTITY Test_Sampling IS
END Test_Sampling;
 
ARCHITECTURE behavior OF Test_Sampling IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Sampling
	 Generic(CLK_CYCLES_PRE_DONE : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		ADC_PRESCALER : IN std_logic_vector(7 downto 0);
		PHASEINC : IN std_logic_vector(11 downto 0);
		PORT1 : IN std_logic_vector(17 downto 0);
		PORT2 : IN std_logic_vector(17 downto 0);
		REF : IN std_logic_vector(17 downto 0);
		NEW_SAMPLE : IN std_logic;
		START : IN std_logic;
		SAMPLES : IN std_logic_vector(12 downto 0);      
		ADC_START : OUT std_logic;
		DONE : OUT std_logic;
		PRE_DONE : OUT std_logic;
		PORT1_I : OUT std_logic_vector(47 downto 0);
		PORT1_Q : OUT std_logic_vector(47 downto 0);
		PORT2_I : OUT std_logic_vector(47 downto 0);
		PORT2_Q : OUT std_logic_vector(47 downto 0);
		REF_I : OUT std_logic_vector(47 downto 0);
		REF_Q : OUT std_logic_vector(47 downto 0);
		ACTIVE : OUT std_logic
		);
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal PORT1 : std_logic_vector(17 downto 0) := (others => '0');
   signal PORT2 : std_logic_vector(17 downto 0) := (others => '0');
   signal REF : std_logic_vector(17 downto 0) := (others => '0');
   signal NEW_SAMPLE : std_logic := '0';
   signal START : std_logic := '0';
   signal SAMPLES : std_logic_vector(12 downto 0) := (others => '0');
	signal ADC_PRESCALER : std_logic_vector(7 downto 0);
	signal PHASEINC : std_logic_vector(11 downto 0);
 	--Outputs
   signal ADC_START : std_logic;
   signal DONE : std_logic;
   signal PRE_DONE : std_logic;
   signal PORT1_I : std_logic_vector(47 downto 0);
   signal PORT1_Q : std_logic_vector(47 downto 0);
   signal PORT2_I : std_logic_vector(47 downto 0);
   signal PORT2_Q : std_logic_vector(47 downto 0);
   signal REF_I : std_logic_vector(47 downto 0);
   signal REF_Q : std_logic_vector(47 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 6.25 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Sampling
	Generic MAP(CLK_CYCLES_PRE_DONE => 0)
	PORT MAP (
			CLK => CLK,
			RESET => RESET,
			ADC_PRESCALER => ADC_PRESCALER,
			PHASEINC => PHASEINC,
			PORT1 => PORT1,
			PORT2 => PORT2,
			REF => REF,
			NEW_SAMPLE => NEW_SAMPLE,
			START => START,
			SAMPLES => SAMPLES,     
			ADC_START => ADC_START,
			DONE => DONE,
			PRE_DONE => PRE_DONE,
			PORT1_I => PORT1_I,
			PORT1_Q => PORT1_Q,
			PORT2_I => PORT2_I,
			PORT2_Q => PORT2_Q,
			REF_I => REF_I,
			REF_Q => REF_Q,
			ACTIVE => open
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
		ADC_PRESCALER <= "011110000";
		PHASEINC <= "010001100000";
		PORT1 <= "000001111111111111";
		PORT2 <= "000011111111111111";
		REF <= "000111111111111111";
		SAMPLES <= "0000000000001";
		START <= '1';
		while True loop
			wait until ADC_START = '1';
			START <= '0';
			wait for CLK_period * 110;
			NEW_SAMPLE <= '1';
			wait for CLK_period;
			NEW_SAMPLE <= '0';
		end loop;
      wait;

   end process;

END;
