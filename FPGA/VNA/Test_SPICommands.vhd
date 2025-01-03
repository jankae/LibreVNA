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
         CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           SCLK : in  STD_LOGIC;
           MOSI : in  STD_LOGIC;
           MISO : out  STD_LOGIC;
           NSS : in  STD_LOGIC;
           NEW_SAMPLING_DATA : in  STD_LOGIC;
           SAMPLING_RESULT : in  STD_LOGIC_VECTOR (303 downto 0);
			  ADC_MINMAX : in STD_LOGIC_VECTOR(95 downto 0);
           SOURCE_UNLOCKED : in  STD_LOGIC;
           LO_UNLOCKED : in  STD_LOGIC;
           MAX2871_DEF_4 : out  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_3 : out  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_1 : out  STD_LOGIC_VECTOR (31 downto 0);
           MAX2871_DEF_0 : out  STD_LOGIC_VECTOR (31 downto 0);
           SWEEP_DATA : out  STD_LOGIC_VECTOR (95 downto 0);
           SWEEP_ADDRESS : out  STD_LOGIC_VECTOR (12 downto 0);
           SWEEP_WRITE : out  STD_LOGIC_VECTOR (0 downto 0);
           SWEEP_POINTS : out  STD_LOGIC_VECTOR (12 downto 0);
           NSAMPLES : out  STD_LOGIC_VECTOR (12 downto 0);
			  STAGES : out STD_LOGIC_VECTOR (2 downto 0);
			  SETTLING_TIME : out STD_LOGIC_VECTOR (19 downto 0);
			  SYNC_ENABLED : out STD_LOGIC;
			  SYNC_MASTER : out STD_LOGIC;
			  PORT1_STAGE : out STD_LOGIC_VECTOR (2 downto 0);
			  PORT2_STAGE : out STD_LOGIC_VECTOR (2 downto 0);
			  PORT1_EN : out STD_LOGIC;
			  PORT2_EN : out STD_LOGIC;
			  REF_EN : out STD_LOGIC;
			  AMP_SHDN : out STD_LOGIC;
			  SOURCE_RF_EN : out STD_LOGIC;
			  LO_RF_EN : out STD_LOGIC;
			  SOURCE_CE_EN : out STD_LOGIC;
			  LO_CE_EN : out STD_LOGIC;
			  PORTSWITCH_EN : out STD_LOGIC;
			  LEDS : out STD_LOGIC_VECTOR(2 downto 0);
			  WINDOW_SETTING : out STD_LOGIC_VECTOR(1 downto 0);
			  ADC_PRESCALER : out STD_LOGIC_VECTOR(7 downto 0);
			  ADC_PHASEINC : out STD_LOGIC_VECTOR(11 downto 0);
			  INTERRUPT_ASSERTED : out STD_LOGIC;
			  RESET_MINMAX : out STD_LOGIC;
			  SWEEP_HALTED : in STD_LOGIC;
			  SWEEP_RESUME : out STD_LOGIC;
			  
			  -- hardware overwrite signals
			  SPI_OVERWRITE_ENABLED : out STD_LOGIC;
			  SPI_OVERWRITE_DATA : out STD_LOGIC_VECTOR(14 downto 0);
			  
			  -- DFT signals
			  DFT_BIN1_PHASEINC : out  STD_LOGIC_VECTOR (15 downto 0);
           DFT_DIFFBIN_PHASEINC : out  STD_LOGIC_VECTOR (15 downto 0);
			  DFT_RESULT_READY : in  STD_LOGIC;
           DFT_OUTPUT : in  STD_LOGIC_VECTOR (191 downto 0);
           DFT_NEXT_OUTPUT : out  STD_LOGIC;
			  DFT_ENABLE : out STD_LOGIC;
			  			  
			  DEBUG_STATUS : in STD_LOGIC_VECTOR(10 downto 0)
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
   constant CLK_period : time := 9.765625 ns;
	constant SPI_CLK_period : time := 23.52941176 ns;
	
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
			 ADC_MINMAX => (others => '0'),
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
          INTERRUPT_ASSERTED => INTERRUPT_ASSERTED,
			 SWEEP_HALTED => '0',
			 DFT_RESULT_READY => '0',
			 DFT_OUTPUT => (others => '0'),
			 DEBUG_STATUS => (others => '0')
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
		-- read static test register
		NSS <= '0';
		SPI("0100000000000000");
		SPI("0000000000000000");
		NSS <= '1';
		
		wait for CLK_period*50;
		-- write register 3 = 0xFFFF (enable all periphery)
		NSS <= '0';
		SPI("1000000000000011");
		SPI("1111111111111111");
		NSS <= '1';
		
		wait for CLK_period*50;
		-- set sampling result and read first 4 words
		SAMPLING_RESULT(63 downto 0) <= "1111000011110000101010101010101001010101010101010000111100001111";
		NSS <= '0';
		SPI("1100000000000000");
		SPI("0000000000000000");
		SPI("0000000000000000");
		SPI("0000000000000000");
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
