--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   18:42:26 05/07/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/VNA/FPGA/VNA/Test_SPICommands.vhd
-- Project Name:  VNA
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: SPICommands
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
 
ENTITY Test_SPICommands IS
END Test_SPICommands;
 
ARCHITECTURE behavior OF Test_SPICommands IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT SPICommands
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         SCLK : IN  std_logic;
         MOSI : IN  std_logic;
         MISO : OUT  std_logic;
         NSS : IN  std_logic;
         NEW_SAMPLING_DATA : IN  std_logic;
         SAMPLING_RESULT : IN  std_logic_vector(303 downto 0);
         SOURCE_UNLOCKED : IN  std_logic;
         LO_UNLOCKED : IN  std_logic;
         MAX2871_DEF_4 : OUT  std_logic_vector(31 downto 0);
         MAX2871_DEF_3 : OUT  std_logic_vector(31 downto 0);
         MAX2871_DEF_1 : OUT  std_logic_vector(31 downto 0);
         MAX2871_DEF_0 : OUT  std_logic_vector(31 downto 0);
         SWEEP_DATA : OUT  std_logic_vector(95 downto 0);
         SWEEP_ADDRESS : OUT  std_logic_vector(12 downto 0);
         SWEEP_WRITE : OUT  std_logic_vector(0 downto 0);
         SWEEP_POINTS : OUT  std_logic_vector(12 downto 0);
         NSAMPLES : OUT  std_logic_vector(12 downto 0);
         PORT1_EN : OUT  std_logic;
         PORT2_EN : OUT  std_logic;
         REF_EN : OUT  std_logic;
         AMP_SHDN : OUT  std_logic;
         SOURCE_RF_EN : OUT  std_logic;
         LO_RF_EN : OUT  std_logic;
         LEDS : OUT  std_logic_vector(2 downto 0);
         INTERRUPT_ASSERTED : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal SCLK : std_logic := '0';
   signal MOSI : std_logic := '0';
   signal NSS : std_logic := '0';
   signal NEW_SAMPLING_DATA : std_logic := '0';
   signal SAMPLING_RESULT : std_logic_vector(303 downto 0) := (others => '0');
   signal SOURCE_UNLOCKED : std_logic := '1';
   signal LO_UNLOCKED : std_logic := '1';

 	--Outputs
   signal MISO : std_logic;
   signal MAX2871_DEF_4 : std_logic_vector(31 downto 0);
   signal MAX2871_DEF_3 : std_logic_vector(31 downto 0);
   signal MAX2871_DEF_1 : std_logic_vector(31 downto 0);
   signal MAX2871_DEF_0 : std_logic_vector(31 downto 0);
   signal SWEEP_DATA : std_logic_vector(95 downto 0);
   signal SWEEP_ADDRESS : std_logic_vector(12 downto 0);
   signal SWEEP_WRITE : std_logic_vector(0 downto 0);
   signal SWEEP_POINTS : std_logic_vector(12 downto 0);
   signal NSAMPLES : std_logic_vector(12 downto 0);
   signal PORT1_EN : std_logic;
   signal PORT2_EN : std_logic;
   signal REF_EN : std_logic;
   signal AMP_SHDN : std_logic;
   signal SOURCE_RF_EN : std_logic;
   signal LO_RF_EN : std_logic;
   signal LEDS : std_logic_vector(2 downto 0);
   signal INTERRUPT_ASSERTED : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 6.25 ns;
	constant SPI_CLK_period : time := 100 ns;
	
	signal data_signal : std_logic_vector(15 downto 0);
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: SPICommands PORT MAP (
          CLK => CLK,
          RESET => RESET,
          SCLK => SCLK,
          MOSI => MOSI,
          MISO => MISO,
          NSS => NSS,
          NEW_SAMPLING_DATA => NEW_SAMPLING_DATA,
          SAMPLING_RESULT => SAMPLING_RESULT,
          SOURCE_UNLOCKED => SOURCE_UNLOCKED,
          LO_UNLOCKED => LO_UNLOCKED,
          MAX2871_DEF_4 => MAX2871_DEF_4,
          MAX2871_DEF_3 => MAX2871_DEF_3,
          MAX2871_DEF_1 => MAX2871_DEF_1,
          MAX2871_DEF_0 => MAX2871_DEF_0,
          SWEEP_DATA => SWEEP_DATA,
          SWEEP_ADDRESS => SWEEP_ADDRESS,
          SWEEP_WRITE => SWEEP_WRITE,
          SWEEP_POINTS => SWEEP_POINTS,
          NSAMPLES => NSAMPLES,
          PORT1_EN => PORT1_EN,
          PORT2_EN => PORT2_EN,
          REF_EN => REF_EN,
          AMP_SHDN => AMP_SHDN,
          SOURCE_RF_EN => SOURCE_RF_EN,
          LO_RF_EN => LO_RF_EN,
          LEDS => LEDS,
          INTERRUPT_ASSERTED => INTERRUPT_ASSERTED
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
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
			MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SCLK <= '1';
			wait for SPI_CLK_period/2;
			SCLK <= '0';
		end procedure SPI;
	begin
      -- hold reset state for 100 ns.
		RESET <= '1';
		NSS <= '1';
      wait for 100 ns;	
		RESET <= '0';
      wait for CLK_period*10;
		NSS <= '0';
		SPI("1100000000000000");
		SPI("0000000000000000");
		NSS <= '1';
		
		wait for CLK_period*50;
      -- insert stimulus here 
		-- write number of points
		NSS <= '0';
		SPI("1000000000000001");
		SPI("1111000011110000");
		NSS <= '1';
	
		wait for CLK_period*100;
		-- Write sweep config
		NSS <= '0';
		SPI("0000000000001011");
		SPI("1111111100000000");
		SPI("1111000011110000");
		SPI("1100110011001100");
		SPI("1010101010101010");
		SPI("1101101101101101");
		SPI("1110111011101110");
		SPI("1111101111101111");
		NSS <= '1';
		
		wait for CLK_period*50;
		NEW_SAMPLING_DATA <= '1';
		wait for CLK_period;
		NEW_SAMPLING_DATA <= '0';
		wait for CLK_period*20;
		NSS <= '0';
		SPI("1100000000000000");
		NSS <= '1';
		
		wait for CLK_period*50;
		NEW_SAMPLING_DATA <= '1';
		wait for CLK_period;
		NEW_SAMPLING_DATA <= '0';
		wait for CLK_period*20;
		NSS <= '0';
		SPI("1100000000000000");
		NSS <= '1';
		
		
      wait;
   end process;

END;
