--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   16:27:40 05/05/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_MCP33131.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: MCP33131
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
 
ENTITY Test_MCP33131 IS
END Test_MCP33131;
 
ARCHITECTURE behavior OF Test_MCP33131 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT MCP33131
	 	 Generic(CLK_DIV : integer;
				CONVCYCLES : integer);
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         START : IN  std_logic;
         READY : OUT  std_logic;
         DATA : OUT  std_logic_vector(15 downto 0);
			MIN : out STD_LOGIC_VECTOR (15 downto 0);
			MAX : out STD_LOGIC_VECTOR (15 downto 0);
			RESET_MINMAX : in STD_LOGIC;
         SDO : IN  std_logic;
         CONVSTART : OUT  std_logic;
         SCLK : INOUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal START : std_logic := '0';
   signal SDO : std_logic := '0';
	signal RESET_MINMAX : STD_LOGIC := '0';

 	--Outputs
   signal READY : std_logic;
   signal DATA : std_logic_vector(15 downto 0);
   signal CONVSTART : std_logic;
   signal SCLK : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 9.765 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: MCP33131
	GENERIC MAP(CLK_DIV => 2,
					CONVCYCLES => 77)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          START => START,
          READY => READY,
          DATA => DATA,
			 RESET_MINMAX => RESET_MINMAX,
          SDO => SDO,
          CONVSTART => CONVSTART,
          SCLK => SCLK
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
		wait for CLK_period*10.5;
		RESET <= '0';
      wait for CLK_period*10;
		
      -- insert stimulus here
		while True loop
			wait for CLK_period*111;
			START <= '1';
			wait for CLK_period;
			START <= '0';
		end loop;
      wait;
   end process;

END;
