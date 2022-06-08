--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   15:32:41 05/15/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_SPI.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: spi_slave
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
 
ENTITY Test_SPI IS
END Test_SPI;
 
ARCHITECTURE behavior OF Test_SPI IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT spi_slave
	 GENERIC(W : integer);
    PORT(
         SPI_CLK : IN  std_logic;
         MISO : OUT  std_logic;
         MOSI : IN  std_logic;
         CS : IN  std_logic;
         BUF_OUT : OUT  std_logic_vector(W-1 downto 0);
         BUF_IN : IN  std_logic_vector(W-1 downto 0);
         CLK : IN  std_logic;
         COMPLETE : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal SPI_CLK : std_logic := '0';
   signal MOSI : std_logic := '0';
   signal CS : std_logic := '0';
   signal BUF_IN : std_logic_vector(15 downto 0) := (others => '0');
   signal CLK : std_logic := '0';

 	--Outputs
   signal MISO : std_logic;
   signal BUF_OUT : std_logic_vector(15 downto 0);
   signal COMPLETE : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
   constant SPI_CLK_period : time := 100 ns;
 
 	signal data_signal : std_logic_vector(15 downto 0);

BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: spi_slave
	GENERIC MAP(W => 16)
	PORT MAP (
          SPI_CLK => SPI_CLK,
          MISO => MISO,
          MOSI => MOSI,
          CS => CS,
          BUF_OUT => BUF_OUT,
          BUF_IN => BUF_IN,
          CLK => CLK,
          COMPLETE => COMPLETE
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
			procedure SPI(data : std_logic_vector(15 downto 0)) is
		begin
			MOSI <= data(15);
			data_signal <= data(14 downto 0) & "0";
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '1';
			wait for SPI_CLK_period/2;
			SPI_CLK <= '0';
		end procedure SPI;

   begin		
      -- hold reset state for 100 ns.
		CS <= '1';
      wait for 100 ns;	

      wait for CLK_period*10;
		BUF_IN <= "1111000010100101";
      -- insert stimulus here 
      wait for CLK_period*10;
		CS <= '0';
		SPI("0101010101010101");
		CS <= '1';
		
		wait for CLK_period*10;
		BUF_IN <= "0000111100001111";
      wait for CLK_period*10;
		CS <= '0';
		SPI("0101010101010101");
		BUF_IN <= "1010101010101010";
		wait for SPI_CLK_period/2;
		SPI("1100110011001100");
		CS <= '1';
		
      wait;
   end process;

END;
