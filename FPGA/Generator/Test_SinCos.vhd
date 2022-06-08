--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   09:16:15 05/14/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_SinCos.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: SinCos
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
 
ENTITY Test_SinCos IS
END Test_SinCos;
 
ARCHITECTURE behavior OF Test_SinCos IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SinCos
    PORT(
         clk : IN  std_logic;
         phase_in : IN  std_logic_vector(11 downto 0);
         cosine : OUT  std_logic_vector(15 downto 0);
         sine : OUT  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal clk : std_logic := '0';
   signal phase_in : std_logic_vector(11 downto 0) := (others => '0');

 	--Outputs
   signal cosine : std_logic_vector(15 downto 0);
   signal sine : std_logic_vector(15 downto 0);

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: SinCos PORT MAP (
          clk => clk,
          phase_in => phase_in,
          cosine => cosine,
          sine => sine
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for clk_period*10;

      -- insert stimulus here
		phase_in <= "000000000000";
		wait for clk_period*10;
		phase_in <= "000000000001";
		wait for clk_period*10;
		phase_in <= "000000000010";
		wait for clk_period*10;
		phase_in <= "000000000011";
		wait for clk_period*10;
		phase_in <= "000000000100";
      wait;
   end process;

END;
