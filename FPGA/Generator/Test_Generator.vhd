--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   16:08:07 06/08/2022
-- Design Name:   
-- Module Name:   /home/jan/Projekte/LibreVNA/FPGA/Generator/Test_Generator.vhd
-- Project Name:  Generator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: top
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
 
ENTITY Test_Generator IS
END Test_Generator;
 
ARCHITECTURE behavior OF Test_Generator IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT top
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         MCU_MOSI : IN  std_logic;
         MCU_NSS : IN  std_logic;
         MCU_INTR : OUT  std_logic;
         MCU_SCK : IN  std_logic;
         MCU_MISO : OUT  std_logic;
         MCU_AUX1 : IN  std_logic;
         MCU_AUX2 : IN  std_logic;
         MCU_AUX3 : IN  std_logic;
         PORT2_CONVSTART : OUT  std_logic;
         PORT2_SDO : IN  std_logic;
         PORT2_SCLK : OUT  std_logic;
         PORT2_MIX2_EN : OUT  std_logic;
         PORT2_MIX1_EN : OUT  std_logic;
         PORT1_CONVSTART : OUT  std_logic;
         PORT1_SDO : IN  std_logic;
         PORT1_SCLK : OUT  std_logic;
         PORT1_MIX2_EN : OUT  std_logic;
         PORT1_MIX1_EN : OUT  std_logic;
         LO1_MUX : IN  std_logic;
         LO1_RF_EN : OUT  std_logic;
         LO1_LD : IN  std_logic;
         LO1_CLK : OUT  std_logic;
         LO1_MOSI : OUT  std_logic;
         LO1_LE : OUT  std_logic;
         LO1_CE : OUT  std_logic;
         LEDS : OUT  std_logic_vector(7 downto 0);
         REF_MIX2_EN : OUT  std_logic;
         REF_MIX1_EN : OUT  std_logic;
         ATTENUATION : OUT  std_logic_vector(6 downto 0);
         AMP_PWDN : OUT  std_logic;
         PORT1_SELECT : OUT  std_logic;
         PORT2_SELECT : OUT  std_logic;
         PORT_SELECT1 : OUT  std_logic;
         PORT_SELECT2 : OUT  std_logic;
         BAND_SELECT_HIGH : OUT  std_logic;
         BAND_SELECT_LOW : OUT  std_logic;
         FILT_OUT_C1 : OUT  std_logic;
         FILT_OUT_C2 : OUT  std_logic;
         FILT_IN_C1 : OUT  std_logic;
         FILT_IN_C2 : OUT  std_logic;
         SOURCE_RF_EN : OUT  std_logic;
         SOURCE_LD : IN  std_logic;
         SOURCE_MUX : IN  std_logic;
         SOURCE_CLK : OUT  std_logic;
         SOURCE_MOSI : OUT  std_logic;
         SOURCE_LE : OUT  std_logic;
         SOURCE_CE : OUT  std_logic;
         REF_CONVSTART : OUT  std_logic;
         REF_SDO : IN  std_logic;
         REF_SCLK : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal MCU_MOSI : std_logic := '0';
   signal MCU_NSS : std_logic := '1';
   signal MCU_SCK : std_logic := '0';
   signal MCU_AUX1 : std_logic := '0';
   signal MCU_AUX2 : std_logic := '0';
   signal MCU_AUX3 : std_logic := '1';
   signal PORT2_SDO : std_logic := '0';
   signal PORT1_SDO : std_logic := '0';
   signal LO1_MUX : std_logic := '0';
   signal LO1_LD : std_logic := '1';
   signal SOURCE_LD : std_logic := '1';
   signal SOURCE_MUX : std_logic := '0';
   signal REF_SDO : std_logic := '0';

 	--Outputs
   signal MCU_INTR : std_logic;
   signal MCU_MISO : std_logic;
   signal PORT2_CONVSTART : std_logic;
   signal PORT2_SCLK : std_logic;
   signal PORT2_MIX2_EN : std_logic;
   signal PORT2_MIX1_EN : std_logic;
   signal PORT1_CONVSTART : std_logic;
   signal PORT1_SCLK : std_logic;
   signal PORT1_MIX2_EN : std_logic;
   signal PORT1_MIX1_EN : std_logic;
   signal LO1_RF_EN : std_logic;
   signal LO1_CLK : std_logic;
   signal LO1_MOSI : std_logic;
   signal LO1_LE : std_logic;
   signal LO1_CE : std_logic;
   signal LEDS : std_logic_vector(7 downto 0);
   signal REF_MIX2_EN : std_logic;
   signal REF_MIX1_EN : std_logic;
   signal ATTENUATION : std_logic_vector(6 downto 0);
   signal AMP_PWDN : std_logic;
   signal PORT1_SELECT : std_logic;
   signal PORT2_SELECT : std_logic;
   signal PORT_SELECT1 : std_logic;
   signal PORT_SELECT2 : std_logic;
   signal BAND_SELECT_HIGH : std_logic;
   signal BAND_SELECT_LOW : std_logic;
   signal FILT_OUT_C1 : std_logic;
   signal FILT_OUT_C2 : std_logic;
   signal FILT_IN_C1 : std_logic;
   signal FILT_IN_C2 : std_logic;
   signal SOURCE_RF_EN : std_logic;
   signal SOURCE_CLK : std_logic;
   signal SOURCE_MOSI : std_logic;
   signal SOURCE_LE : std_logic;
   signal SOURCE_CE : std_logic;
   signal REF_CONVSTART : std_logic;
   signal REF_SCLK : std_logic;

	signal data_signal : std_logic_vector(15 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 62.5 ns;
	constant SPI_CLK_period : time := 50 ns;
	
	constant center_freq : std_logic_vector (32 downto 0) := std_logic_vector(to_unsigned(129055507, 33));
	constant deviation_freq : std_logic_vector (25 downto 0) := std_logic_vector(to_unsigned(25811102, 26));
 	constant minimum_vco_freq : std_logic_vector (31 downto 0) := "11100110110001001110110001001111";
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: top PORT MAP (
          CLK => CLK,
          RESET => RESET,
          MCU_MOSI => MCU_MOSI,
          MCU_NSS => MCU_NSS,
          MCU_INTR => MCU_INTR,
          MCU_SCK => MCU_SCK,
          MCU_MISO => MCU_MISO,
          MCU_AUX1 => MCU_AUX1,
          MCU_AUX2 => MCU_AUX2,
          MCU_AUX3 => MCU_AUX3,
          PORT2_CONVSTART => PORT2_CONVSTART,
          PORT2_SDO => PORT2_SDO,
          PORT2_SCLK => PORT2_SCLK,
          PORT2_MIX2_EN => PORT2_MIX2_EN,
          PORT2_MIX1_EN => PORT2_MIX1_EN,
          PORT1_CONVSTART => PORT1_CONVSTART,
          PORT1_SDO => PORT1_SDO,
          PORT1_SCLK => PORT1_SCLK,
          PORT1_MIX2_EN => PORT1_MIX2_EN,
          PORT1_MIX1_EN => PORT1_MIX1_EN,
          LO1_MUX => LO1_MUX,
          LO1_RF_EN => LO1_RF_EN,
          LO1_LD => LO1_LD,
          LO1_CLK => LO1_CLK,
          LO1_MOSI => LO1_MOSI,
          LO1_LE => LO1_LE,
          LO1_CE => LO1_CE,
          LEDS => LEDS,
          REF_MIX2_EN => REF_MIX2_EN,
          REF_MIX1_EN => REF_MIX1_EN,
          ATTENUATION => ATTENUATION,
          AMP_PWDN => AMP_PWDN,
          PORT1_SELECT => PORT1_SELECT,
          PORT2_SELECT => PORT2_SELECT,
          PORT_SELECT1 => PORT_SELECT1,
          PORT_SELECT2 => PORT_SELECT2,
          BAND_SELECT_HIGH => BAND_SELECT_HIGH,
          BAND_SELECT_LOW => BAND_SELECT_LOW,
          FILT_OUT_C1 => FILT_OUT_C1,
          FILT_OUT_C2 => FILT_OUT_C2,
          FILT_IN_C1 => FILT_IN_C1,
          FILT_IN_C2 => FILT_IN_C2,
          SOURCE_RF_EN => SOURCE_RF_EN,
          SOURCE_LD => SOURCE_LD,
          SOURCE_MUX => SOURCE_MUX,
          SOURCE_CLK => SOURCE_CLK,
          SOURCE_MOSI => SOURCE_MOSI,
          SOURCE_LE => SOURCE_LE,
          SOURCE_CE => SOURCE_CE,
          REF_CONVSTART => REF_CONVSTART,
          REF_SDO => REF_SDO,
          REF_SCLK => REF_SCLK
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
			MCU_MOSI <= data(15);
			data_signal <= data(14 downto 0) & "0";
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
			MCU_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '1';
			wait for SPI_CLK_period/2;
			MCU_SCK <= '0';
		end procedure SPI;
   begin		
		RESET <= '1';
      -- hold reset state for 100 ns.
      wait for 1000 ns;	

		MCU_AUX3 <= '1'; -- stop modulation
		RESET <= '0';

      wait for CLK_period*10;

      -- insert stimulus here
		-- configure VCO table (only use first four entries)
		-- VCO 0 is good up to 3GHz
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000000000000");
		SPI(std_logic_vector(to_unsigned(29538, 16)));
		MCU_NSS <= '1';
		-- VCO 1 is good up to 4GHz
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000000000001");
		SPI(std_logic_vector(to_unsigned(39385, 16)));
		MCU_NSS <= '1';
		-- VCO 2 is good up to 5GHz
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000000000010");
		SPI(std_logic_vector(to_unsigned(49231, 16)));
		MCU_NSS <= '1';
		-- VCO 3 is good up to 6GHz
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000000000011");
		SPI(std_logic_vector(to_unsigned(59077, 16)));
		MCU_NSS <= '1';
		
		-- configure modulation
		-- using FM/AM combination:
		-- center 100 GHz, 20 MHz deviation
		-- -3dB base attenuation, 100% depth
		
		-- set band/port/filter/base attenuation, enable PLL/AMP
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("1000000000000001");
		SPI("1111000110000110");
		MCU_NSS <= '1';

		-- enable modulation
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("1000000000000010");
		SPI("0000000000000001");
		MCU_NSS <= '1';
		
		-- set 20kHz modulation rate
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("1000000000000011");
		SPI(std_logic_vector(to_unsigned(26214, 16)));
		MCU_NSS <= '1';
		
		-- set FIFO threshold to 100
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("1000000000000100");
		SPI(std_logic_vector(to_unsigned(100, 16)));
		MCU_NSS <= '1';
		
		-- setup modulation lookup-table
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		SPI("1111000011110000");
		MCU_NSS <= '1';
				wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0000000001010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		SPI("0101010101010101");
		MCU_NSS <= '1';
		
		wait for CLK_period*10;
		MCU_NSS <= '0';
		SPI("0010000000000000");
		for i in 0 to 63 loop
			-- write sample FIFO
			wait for CLK_period*10;
			SPI("1111000001010101");
		end loop;
		MCU_NSS <= '0';
		
		wait for CLK_period*10;
		-- start the modulation
		MCU_AUX3 <= '0';
		
      wait;		
   end process;

END;
