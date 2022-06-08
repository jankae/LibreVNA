----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:47:31 05/05/2020 
-- Design Name: 
-- Module Name:    top - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity top is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           MCU_MOSI : in  STD_LOGIC;
           MCU_NSS : in  STD_LOGIC;
           MCU_INTR : out  STD_LOGIC;
           MCU_SCK : in  STD_LOGIC;
           MCU_MISO : out  STD_LOGIC;
           MCU_AUX1 : in  STD_LOGIC;
           MCU_AUX2 : in  STD_LOGIC;
           MCU_AUX3 : in  STD_LOGIC;
           PORT2_CONVSTART : out  STD_LOGIC;
           PORT2_SDO : in  STD_LOGIC;
           PORT2_SCLK : out  STD_LOGIC;
           PORT2_MIX2_EN : out  STD_LOGIC;
           PORT2_MIX1_EN : out  STD_LOGIC;
           PORT1_CONVSTART : out  STD_LOGIC;
           PORT1_SDO : in  STD_LOGIC;
           PORT1_SCLK : out  STD_LOGIC;
           PORT1_MIX2_EN : out  STD_LOGIC;
           PORT1_MIX1_EN : out  STD_LOGIC;
           LO1_MUX : in  STD_LOGIC;
           LO1_RF_EN : out  STD_LOGIC;
           LO1_LD : in  STD_LOGIC;
           LO1_CLK : out  STD_LOGIC;
           LO1_MOSI : out  STD_LOGIC;
           LO1_LE : out  STD_LOGIC;
           LO1_CE : out  STD_LOGIC;
           LEDS : out  STD_LOGIC_VECTOR (7 downto 0);
           REF_MIX2_EN : out  STD_LOGIC;
			  REF_MIX1_EN : out  STD_LOGIC;
           ATTENUATION : out  STD_LOGIC_VECTOR (6 downto 0);
           AMP_PWDN : out  STD_LOGIC;
           PORT1_SELECT : out  STD_LOGIC; -- Port 1 additional isolation switch enable
           PORT2_SELECT : out  STD_LOGIC; -- Port 2 additional isolation switch enable
			  PORT_SELECT1 : out STD_LOGIC; -- Enable source -> port 1 switch
			  PORT_SELECT2 : out STD_LOGIC; -- Enable source -> port 2 switch
           BAND_SELECT_HIGH : out  STD_LOGIC;
           BAND_SELECT_LOW : out  STD_LOGIC;
           FILT_OUT_C1 : out  STD_LOGIC;
           FILT_OUT_C2 : out  STD_LOGIC;
           FILT_IN_C1 : out  STD_LOGIC;
           FILT_IN_C2 : out  STD_LOGIC;
           SOURCE_RF_EN : out  STD_LOGIC;
           SOURCE_LD : in  STD_LOGIC;
           SOURCE_MUX : in  STD_LOGIC;
           SOURCE_CLK : out  STD_LOGIC;
           SOURCE_MOSI : out  STD_LOGIC;
           SOURCE_LE : out  STD_LOGIC;
           SOURCE_CE : out  STD_LOGIC;
           REF_CONVSTART : out  STD_LOGIC;
           REF_SDO : in  STD_LOGIC;
           REF_SCLK : out  STD_LOGIC);
end top;

architecture Behavioral of top is
	component PLL
	port
		(-- Clock in ports
		CLK_IN1           : in     std_logic;
		-- Clock out ports
		CLK_OUT1          : out    std_logic;
		-- Status and control signals
		RESET             : in     std_logic;
		LOCKED            : out    std_logic
		);
	end component;
	
	COMPONENT ResetDelay
	GENERIC(CLK_DELAY : integer);
	PORT(
		CLK : IN std_logic;
		IN_RESET : IN std_logic;          
		OUT_RESET : OUT std_logic
		);
	END COMPONENT;

	COMPONENT MAX2871
	Generic (CLK_DIV : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		REG4 : IN std_logic_vector(31 downto 0);
		REG3 : IN std_logic_vector(31 downto 0);
		REG1 : IN std_logic_vector(31 downto 0);
		REG0 : IN std_logic_vector(31 downto 0);
		RELOAD : IN std_logic;          
		CLK_OUT : OUT std_logic;
		MOSI : OUT std_logic;
		LE : OUT std_logic;
		DONE : OUT std_logic
		);
	END COMPONENT;
	COMPONENT SPICommands
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		SCLK : IN std_logic;
		MOSI : IN std_logic;
		NSS : IN std_logic;
		SOURCE_UNLOCKED : IN std_logic;
		MOD_FIFO_UNDERFLOW : IN std_logic;
		MOD_FIFO_OVERFLOW : IN std_logic;
		MOD_FIFO_THRESHOLD_CROSSED : IN std_logic;          
		MISO : OUT std_logic;
		SOURCE_FILTER : OUT std_logic_vector(1 downto 0);
		SOURCE_POWER : out STD_LOGIC_VECTOR(1 downto 0);
		SOURCE_ATTENUATION : OUT std_logic_vector(6 downto 0);
		SOURCE_BANDSELECT : OUT std_logic;
		SOURCE_PORTSELECT : OUT std_logic;
		SOURCE_VCO_INDEX : OUT std_logic_vector(5 downto 0);
		SOURCE_VCO_MAXFREQ : OUT std_logic_vector(15 downto 0);
		SOURCE_VCO_WRITE : OUT std_logic;
		MOD_FIFO_DATA : OUT std_logic_vector(7 downto 0);
		MOD_FIFO_WRITE : OUT std_logic;
		MOD_FIFO_THRESHOLD : OUT std_logic_vector(10 downto 0);
		MOD_ENABLE : OUT std_logic;
		MOD_PHASE_INC : OUT std_logic_vector(15 downto 0);
		MOD_CENTER_FREQ : OUT std_logic_vector(32 downto 0);
		MOD_DEVIATION_FREQ : OUT std_logic_vector(25 downto 0);
		MOD_AM_DEPTH : OUT std_logic_vector(6 downto 0);
		MOD_VCO_MIN : out STD_LOGIC_VECTOR (31 downto 0);		
		AMP_SHDN : OUT std_logic;
		SOURCE_RF_EN : OUT std_logic;
		SOURCE_CE_EN : OUT std_logic;
		PORTSWITCH_EN : OUT std_logic;
		LEDS : OUT std_logic_vector(2 downto 0);
		INTERRUPT_ASSERTED : OUT std_logic
		);
	END COMPONENT;
	
	COMPONENT Synchronizer
	GENERIC(stages : integer);
	PORT(
		CLK : IN std_logic;
		SYNC_IN : IN std_logic;          
		SYNC_OUT : OUT std_logic
		);
	END COMPONENT;
	
	COMPONENT VCO_Mem
	PORT (
		clka : IN STD_LOGIC;
		wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		addra : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
		dina : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		clkb : IN STD_LOGIC;
		addrb : IN STD_LOGIC_VECTOR(5 DOWNTO 0);
		doutb : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
	);
	END COMPONENT;
	
	COMPONENT Modulator
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		ACTIVE : in STD_LOGIC;
		SAMPLE_FREQ_WORD : IN std_logic_vector(15 downto 0);
		SAMPLE_DATA : IN std_logic_vector(7 downto 0);
		SAMPLE_LATCH : IN std_logic;
		THRESHOLD_LEVEL : IN std_logic_vector(10 downto 0);
		FREQ_CENTER : IN std_logic_vector(32 downto 0);
		FREQ_DEVIATION : IN std_logic_vector(25 downto 0);
		MIN_ATTENUATION : IN std_logic_vector(6 downto 0);
		AMPLITUDE_DEPTH : IN std_logic_vector(6 downto 0);          
		OVERFLOW : OUT std_logic;
		UNDERFLOW : OUT std_logic;
		THRESHOLD_CROSSED : OUT std_logic;
		FREQUENCY : OUT std_logic_vector(32 downto 0);
		ATTENUATOR : OUT std_logic_vector(6 downto 0);
		NEW_OUTPUT : OUT std_logic
	);
	END COMPONENT;
	
	COMPONENT MAX2871_Calc
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		CALC : IN std_logic;
		FREQ : IN std_logic_vector(32 downto 0);
		VCO_MIN : IN std_logic_vector(31 downto 0);
		POWER : IN std_logic_vector(1 downto 0);
		VCO_MAX_FREQ : IN std_logic_vector(15 downto 0);          
		DONE : OUT std_logic;
		REG0 : OUT std_logic_vector(31 downto 0);
		REG1 : OUT std_logic_vector(31 downto 0);
		REG3 : OUT std_logic_vector(31 downto 0);
		REG4 : OUT std_logic_vector(31 downto 0);
		VCO_SELECT : OUT std_logic_vector(5 downto 0)
	);
	END COMPONENT;
	
	signal clk_pll : std_logic;
	signal clk_locked : std_logic;
	signal inv_clk_locked : std_logic;
	signal int_reset : std_logic;
	
	-- PLL signals
	signal source_reg_4 : std_logic_vector(31 downto 0);
	signal source_reg_3 : std_logic_vector(31 downto 0);
	signal source_reg_1 : std_logic_vector(31 downto 0);
	signal source_reg_0 : std_logic_vector(31 downto 0);
	signal reload_plls : std_logic;
	signal source_reloaded : std_logic;
	signal source_unlocked : std_logic;
	
	signal source_power : std_logic_vector(1 downto 0);
	signal source_filter : std_logic_vector(1 downto 0);
	signal band_select : std_logic;
	signal attenuator : std_logic_vector(6 downto 0);
	signal port_select : std_logic;
	signal amp_shutdown : std_logic;
	
	-- Configuration signals
	signal user_leds : std_logic_vector(2 downto 0);
	signal portswitch_en : std_logic;
	
	-- PLL/SPI internal mux
	signal fpga_select : std_logic;
	signal fpga_source_SCK : std_logic;
	signal fpga_source_MOSI : std_logic;
	signal fpga_source_LE : std_logic;
	signal fpga_LO1_SCK : std_logic;
	signal fpga_LO1_MOSI : std_logic;
	signal fpga_LO1_LE : std_logic;
	signal fpga_miso : std_logic;
	
	-- synchronized asynchronous inputs
	signal aux1_sync : std_logic;
	signal aux2_sync : std_logic;
	signal aux3_sync : std_logic;
	signal lo_ld_sync : std_logic;
	signal source_ld_sync : std_logic;
	signal nss_sync : std_logic;
	
	-- VCO table signals
	signal vco_write_index : std_logic_vector(5 downto 0);
	signal vco_read_index : std_logic_vector(5 downto 0);
	signal vco_write : std_logic;
	signal vco_write_data : std_logic_vector(15 downto 0);
	signal vco_read_data : std_logic_vector(15 downto 0);
	
	-- modulation signals
	signal mod_enable : std_logic;
	signal mod_reset : std_logic;
	signal mod_active : std_logic;
	signal mod_sample_word : std_logic_vector(15 downto 0);
	signal mod_sample_data : std_logic_vector(7 downto 0);
	signal mod_sample_latch : std_logic;
	signal mod_threshold_level : std_logic_vector(10 downto 0);
	signal mod_center : std_logic_vector(32 downto 0);
	signal mod_deviation : std_logic_vector(25 downto 0);
	signal mod_depth : std_logic_vector(6 downto 0);
	signal mod_vco_min : std_logic_vector(31 downto 0);
	signal mod_fifo_overflow : std_logic;
	signal mod_fifo_underflow : std_logic;
	signal mod_fifo_threshold_crossed : std_logic;
	
	signal mod_frequency : std_logic_vector(32 downto 0);
	signal mod_attenuator : std_logic_vector(6 downto 0);
	signal mod_new_output : std_logic;
	
	signal pll_calc_done : std_logic;
	
	signal intr : std_logic;
begin

	-- Reference CLK LED
	LEDS(0) <= user_leds(2);
	-- Lock status of PLLs
	LEDS(1) <= clk_locked;
	LEDS(2) <= SOURCE_LD;
	LEDS(3) <= LO1_LD;
	-- Sweep and active port
	PORT_SELECT2 <= not port_select and not amp_shutdown;
	PORT2_SELECT <= not port_select and not amp_shutdown;
	PORT_SELECT1 <= port_select and not amp_shutdown;
	PORT1_SELECT <= port_select and not amp_shutdown;
	BAND_SELECT_HIGH <= not band_select;
	BAND_SELECT_LOW <= band_select;
	AMP_PWDN <= amp_shutdown;
	
	ATTENUATION <= attenuator when mod_reset = '1' else mod_attenuator;
	
	-- unused signals, ADCs not used
	PORT1_MIX2_EN <= '0';
	PORT1_MIX1_EN <= '1';
	PORT2_MIX2_EN <= '0';
	PORT2_MIX1_EN <= '1';
	REF_MIX2_EN <= '0';
	REF_MIX1_EN <= '1';
	PORT1_CONVSTART <= '0';
	PORT1_SCLK <= '0';
	PORT2_CONVSTART <= '0';
	PORT2_SCLK <= '0';
	REF_CONVSTART <= '0';
	REF_SCLK <= '0';
	LO1_RF_EN <= '0';
	LO1_CE <= '0';
	
	LEDS(4) <= not (not port_select and not amp_shutdown);
	LEDS(5) <= not (port_select and not amp_shutdown);
	-- Uncommitted LEDs
	LEDS(7 downto 6) <= user_leds(1 downto 0);	
	MCU_INTR <= intr;
	
	MainCLK : PLL
	port map(
		-- Clock in ports
		CLK_IN1 => CLK,
		-- Clock out ports
		CLK_OUT1 => clk_pll,
		-- Status and control signals
		RESET  => RESET,
		LOCKED => clk_locked
	);
	
	inv_clk_locked <= not clk_locked and not RESET;
	
	Inst_ResetDelay: ResetDelay
	GENERIC MAP(CLK_DELAY => 100)
	PORT MAP(
		CLK => clk_pll,
		IN_RESET => inv_clk_locked,
		OUT_RESET => int_reset
	);
	
	Sync_AUX1 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk_pll,
		SYNC_IN => MCU_AUX1,
		SYNC_OUT => aux1_sync
	);
	Sync_AUX2 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk_pll,
		SYNC_IN => MCU_AUX2,
		SYNC_OUT => aux2_sync
	);
	Sync_AUX3 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk_pll,
		SYNC_IN => MCU_AUX3,
		SYNC_OUT => aux3_sync
	);
--	Sync_LO_LD : Synchronizer
--	GENERIC MAP(stages => 2)
--	PORT MAP(
--		CLK => clk_pll,
--		SYNC_IN => LO1_LD,
--		SYNC_OUT => lo_ld_sync
--	);
	Sync_SOURCE_LD : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk_pll,
		SYNC_IN => SOURCE_LD,
		SYNC_OUT => source_ld_sync
	);	
	Sync_NSS : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk_pll,
		SYNC_IN => MCU_NSS,
		SYNC_OUT => nss_sync
	);	

	Source: MAX2871
	GENERIC MAP(CLK_DIV => 10)
	PORT MAP(
		CLK => clk_pll,
		RESET => int_reset,
		REG4 => source_reg_4,
		REG3 => source_reg_3,
		REG1 => source_reg_1,
		REG0 => source_reg_0,
		RELOAD => mod_new_output,
		CLK_OUT => fpga_source_SCK,
		MOSI => fpga_source_MOSI,
		LE => fpga_source_LE,
		DONE => source_reloaded
	);

	-- Source filter mapping
	FILT_IN_C1 <= '0' when source_filter = "00" or source_filter = "10" else '1';
	FILT_IN_C2 <= '0' when source_filter = "11" or source_filter = "10" else '1';
	FILT_OUT_C1 <= '0' when source_filter = "00" or source_filter = "10" else '1';
	FILT_OUT_C2 <= '0' when source_filter = "00" or source_filter = "01" else '1';
	
	-- PLL/SPI mux
	-- only select FPGA SPI slave when both AUX1 and AUX2 are low
	fpga_select <= nss_sync when aux1_sync = '0' and aux2_sync = '0' else '1';
	-- direct connection between MCU and SOURCE when AUX1 is high
	SOURCE_CLK <= MCU_SCK when aux1_sync = '1' else fpga_source_SCK;
	SOURCE_MOSI <= MCU_MOSI when aux1_sync = '1' else fpga_source_MOSI;
	SOURCE_LE <= MCU_NSS when aux1_sync = '1' else fpga_source_LE;
	-- direct connection between MCU and LO1 when AUX2 is high
	LO1_CLK <= MCU_SCK when aux2_sync = '1' else '0';
	LO1_MOSI <= MCU_MOSI when aux2_sync = '1' else '0';
	LO1_LE <= MCU_NSS when aux2_sync = '1' else '0';
	-- select MISO source
	MCU_MISO <= SOURCE_MUX when aux1_sync = '1' else
					LO1_MUX when aux2_sync = '1' else
					fpga_miso when MCU_NSS = '0' else
					'Z';

	source_unlocked <= not source_ld_sync;

	VCOMap : VCO_Mem
	PORT MAP (
		clka => clk_pll,
		wea(0) => vco_write,
		addra => vco_write_index,
		dina => vco_write_data,
		clkb => clk_pll,
		addrb => vco_read_index,
		doutb => vco_read_data
	);

	SPI: SPICommands PORT MAP(
		CLK => clk_pll,
		RESET => int_reset,
		SCLK => MCU_SCK,
		MOSI => MCU_MOSI,
		MISO => fpga_miso,
		NSS => fpga_select,
		SOURCE_FILTER => source_filter,
		SOURCE_POWER => source_power,
		SOURCE_ATTENUATION => attenuator,
		SOURCE_BANDSELECT => band_select,
		SOURCE_PORTSELECT => port_select,
		SOURCE_UNLOCKED => source_unlocked,
		SOURCE_VCO_INDEX => vco_write_index,
		SOURCE_VCO_MAXFREQ => vco_write_data,
		SOURCE_VCO_WRITE => vco_write,
		MOD_FIFO_DATA => mod_sample_data,
		MOD_FIFO_WRITE => mod_sample_latch,
		MOD_FIFO_UNDERFLOW => mod_fifo_underflow,
		MOD_FIFO_OVERFLOW => mod_fifo_overflow,
		MOD_FIFO_THRESHOLD_CROSSED => mod_fifo_threshold_crossed,
		MOD_FIFO_THRESHOLD => mod_threshold_level,
		MOD_ENABLE => mod_enable,
		MOD_PHASE_INC => mod_sample_word,
		MOD_CENTER_FREQ => mod_center,
		MOD_DEVIATION_FREQ => mod_deviation,
		MOD_AM_DEPTH => mod_depth,
		MOD_VCO_MIN => mod_vco_min,
		AMP_SHDN => amp_shutdown,
		SOURCE_RF_EN => SOURCE_RF_EN,
		SOURCE_CE_EN => SOURCE_CE,
		PORTSWITCH_EN => portswitch_en,
		LEDS => user_leds,
		INTERRUPT_ASSERTED => intr
	);
	
	mod_reset <= not mod_enable;
	mod_active <= not aux3_sync;
	
	Modulation: Modulator PORT MAP(
		CLK => clk_pll,
		RESET => mod_reset,
		ACTIVE => mod_active,
		SAMPLE_FREQ_WORD => mod_sample_word,
		SAMPLE_DATA => mod_sample_data,
		SAMPLE_LATCH => mod_sample_latch,
		OVERFLOW => mod_fifo_overflow,
		UNDERFLOW => mod_fifo_underflow,
		THRESHOLD_LEVEL => mod_threshold_level,
		THRESHOLD_CROSSED => mod_fifo_threshold_crossed,
		FREQ_CENTER => mod_center,
		FREQ_DEVIATION => mod_deviation,
		MIN_ATTENUATION => attenuator,
		AMPLITUDE_DEPTH => mod_depth,
		FREQUENCY => mod_frequency,
		ATTENUATOR => mod_attenuator,
		NEW_OUTPUT => mod_new_output
	);

	PLL_Calc: MAX2871_Calc PORT MAP(
		CLK => clk_pll,
		RESET => int_reset,
		CALC => mod_new_output,
		FREQ => mod_frequency,
		VCO_MIN => mod_vco_min,
		DONE => pll_calc_done,
		REG0 => source_reg_0,
		REG1 => source_reg_1,
		REG3 => source_reg_3,
		REG4 => source_reg_4,
		POWER => source_power,
		VCO_SELECT => vco_read_index,
		VCO_MAX_FREQ => vco_read_data
	);

end Behavioral;

