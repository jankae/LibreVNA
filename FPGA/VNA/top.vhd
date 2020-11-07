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

	COMPONENT Sweep
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		NPOINTS : IN std_logic_vector(12 downto 0);
		CONFIG_DATA : IN std_logic_vector(95 downto 0);
		USER_NSAMPLES : in STD_LOGIC_VECTOR (12 downto 0);
		NSAMPLES : out STD_LOGIC_VECTOR (12 downto 0);
		SAMPLING_BUSY : in STD_LOGIC;
		SAMPLING_DONE : IN std_logic;
		MAX2871_DEF_4 : IN std_logic_vector(31 downto 0);
		MAX2871_DEF_3 : IN std_logic_vector(31 downto 0);
		MAX2871_DEF_1 : IN std_logic_vector(31 downto 0);
		MAX2871_DEF_0 : IN std_logic_vector(31 downto 0);
		PLL_RELOAD_DONE : IN std_logic;
		PLL_LOCKED : IN std_logic; 
		CONFIG_ADDRESS : OUT std_logic_vector(12 downto 0);
		START_SAMPLING : OUT std_logic;
		PORT_SELECT : OUT std_logic;
		BAND_SELECT : out STD_LOGIC;
		SOURCE_REG_4 : OUT std_logic_vector(31 downto 0);
		SOURCE_REG_3 : OUT std_logic_vector(31 downto 0);
		SOURCE_REG_1 : OUT std_logic_vector(31 downto 0);
		SOURCE_REG_0 : OUT std_logic_vector(31 downto 0);
		LO_REG_4 : OUT std_logic_vector(31 downto 0);
		LO_REG_3 : OUT std_logic_vector(31 downto 0);
		LO_REG_1 : OUT std_logic_vector(31 downto 0);
		LO_REG_0 : OUT std_logic_vector(31 downto 0);
		RELOAD_PLL_REGS : OUT std_logic;
		SWEEP_HALTED : out STD_LOGIC;
		SWEEP_RESUME : in STD_LOGIC;
		ATTENUATOR : OUT std_logic_vector(6 downto 0);
		SOURCE_FILTER : OUT std_logic_vector(1 downto 0);
		EXCITE_PORT1 : in STD_LOGIC;
		EXCITE_PORT2 : in STD_LOGIC;
		RESULT_INDEX : out STD_LOGIC_VECTOR (15 downto 0);
		DEBUG_STATUS : out STD_LOGIC_VECTOR (10 downto 0)
		);
	END COMPONENT;
	
	COMPONENT Windowing
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		WINDOW_TYPE : IN std_logic_vector(1 downto 0);
		PORT1_RAW : IN std_logic_vector(15 downto 0);
		PORT2_RAW : IN std_logic_vector(15 downto 0);
		REF_RAW : IN std_logic_vector(15 downto 0);
		ADC_READY : IN std_logic;
		NSAMPLES : IN std_logic_vector(12 downto 0);          
		PORT1_WINDOWED : OUT std_logic_vector(17 downto 0);
		PORT2_WINDOWED : OUT std_logic_vector(17 downto 0);
		REF_WINDOWED : OUT std_logic_vector(17 downto 0);
		WINDOWING_DONE : OUT std_logic
		);
	END COMPONENT;
	
	COMPONENT Sampling
	Generic(CLK_CYCLES_PRE_DONE : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		ADC_PRESCALER : in STD_LOGIC_VECTOR(7 downto 0);
		PHASEINC : in STD_LOGIC_VECTOR(11 downto 0);
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
	COMPONENT MCP33131
	Generic(CLK_DIV : integer;
			CONVCYCLES : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		START : IN std_logic;
		SDO : IN std_logic;          
		READY : OUT std_logic;
		DATA : OUT std_logic_vector(15 downto 0);
		MIN : out STD_LOGIC_VECTOR (15 downto 0);
		MAX : out STD_LOGIC_VECTOR (15 downto 0);
		RESET_MINMAX : in STD_LOGIC;
		CONVSTART : OUT std_logic;
		SCLK : OUT std_logic
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
		NEW_SAMPLING_DATA : IN std_logic;
		SAMPLING_RESULT : IN std_logic_vector(303 downto 0);
		ADC_MINMAX : in STD_LOGIC_VECTOR(95 downto 0);
		SOURCE_UNLOCKED : IN std_logic;
		LO_UNLOCKED : IN std_logic;          
		MISO : OUT std_logic;
		MAX2871_DEF_4 : OUT std_logic_vector(31 downto 0);
		MAX2871_DEF_3 : OUT std_logic_vector(31 downto 0);
		MAX2871_DEF_1 : OUT std_logic_vector(31 downto 0);
		MAX2871_DEF_0 : OUT std_logic_vector(31 downto 0);
		SWEEP_DATA : OUT std_logic_vector(95 downto 0);
		SWEEP_ADDRESS : OUT std_logic_vector(12 downto 0);
		SWEEP_WRITE : OUT std_logic_vector(0 to 0);
		SWEEP_POINTS : OUT std_logic_vector(12 downto 0);
		NSAMPLES : OUT std_logic_vector(12 downto 0);
		EXCITE_PORT1 : out STD_LOGIC;
		EXCITE_PORT2 : out STD_LOGIC;
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
		INTERRUPT_ASSERTED : OUT std_logic;
		RESET_MINMAX : out STD_LOGIC;
		SWEEP_HALTED : in STD_LOGIC;
		SWEEP_RESUME : out STD_LOGIC;
		DFT_BIN1_PHASEINC : out  STD_LOGIC_VECTOR (15 downto 0);
		DFT_DIFFBIN_PHASEINC : out  STD_LOGIC_VECTOR (15 downto 0);
		DFT_RESULT_READY : in  STD_LOGIC;
		DFT_OUTPUT : in  STD_LOGIC_VECTOR (191 downto 0);
		DFT_NEXT_OUTPUT : out  STD_LOGIC;
		DFT_ENABLE : out STD_LOGIC;
		DEBUG_STATUS : in STD_LOGIC_VECTOR (10 downto 0)
		);
	END COMPONENT;
	
	COMPONENT DFT
	Generic (BINS : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		PORT1 : IN std_logic_vector(17 downto 0);
		PORT2 : IN std_logic_vector(17 downto 0);
		NEW_SAMPLE : IN std_logic;
		NSAMPLES : IN std_logic_vector(12 downto 0);
		BIN1_PHASEINC : IN std_logic_vector(15 downto 0);
		DIFFBIN_PHASEINC : IN std_logic_vector(15 downto 0);
		NEXT_OUTPUT : IN std_logic;          
		RESULT_READY : OUT std_logic;
		OUTPUT : OUT std_logic_vector(191 downto 0)
		);
	END COMPONENT;
	
	COMPONENT SweepConfigMem
	PORT (
		clka : IN STD_LOGIC;
		ena : IN STD_LOGIC;
		wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		addra : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
		dina : IN STD_LOGIC_VECTOR(95 DOWNTO 0);
		clkb : IN STD_LOGIC;
		addrb : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
		doutb : OUT STD_LOGIC_VECTOR(95 DOWNTO 0)
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
	
	signal clk160 : std_logic;
	signal clk_locked : std_logic;
	signal inv_clk_locked : std_logic;
	signal int_reset : std_logic;
	
	-- PLL signals
	signal source_reg_4 : std_logic_vector(31 downto 0);
	signal source_reg_3 : std_logic_vector(31 downto 0);
	signal source_reg_1 : std_logic_vector(31 downto 0);
	signal source_reg_0 : std_logic_vector(31 downto 0);
	signal lo_reg_4 : std_logic_vector(31 downto 0);
	signal lo_reg_3 : std_logic_vector(31 downto 0);
	signal lo_reg_1 : std_logic_vector(31 downto 0);
	signal lo_reg_0 : std_logic_vector(31 downto 0);
	signal reload_plls : std_logic;
	signal source_reloaded : std_logic;
	signal lo_reloaded : std_logic;
	signal plls_reloaded : std_logic;
	signal plls_locked : std_logic;
	signal source_unlocked : std_logic;
	signal lo_unlocked : std_logic;
	
	-- ADC signals
	signal adc_trigger_sample : std_logic;
	signal adc_port1_ready : std_logic;
	signal adc_port1_data : std_logic_vector(15 downto 0);
	signal adc_port2_data : std_logic_vector(15 downto 0);
	signal adc_ref_data : std_logic_vector(15 downto 0);
	signal adc_minmax : std_logic_vector(95 downto 0);
	signal adc_reset_minmax : std_logic;
	
	signal port1_windowed : std_logic_vector(17 downto 0);
	signal port2_windowed : std_logic_vector(17 downto 0);
	signal ref_windowed : std_logic_vector(17 downto 0);
	signal windowing_ready : std_logic;
	
	-- Sampling signals
	signal sampling_busy : std_logic;
	signal sampling_done : std_logic;
	signal sampling_start : std_logic;
	signal sampling_samples : std_logic_vector(12 downto 0);
	signal sampling_user_samples : std_logic_vector(12 downto 0);
	signal sampling_result : std_logic_vector(303 downto 0);
	signal sampling_window : std_logic_vector(1 downto 0);
	signal sampling_prescaler : std_logic_vector(7 downto 0);
	signal sampling_phaseinc : std_logic_vector(11 downto 0);
	
	-- Sweep signals
	signal sweep_points : std_logic_vector(12 downto 0);
	signal sweep_config_data : std_logic_vector(95 downto 0);
	signal sweep_config_address : std_logic_vector(12 downto 0);
	signal source_filter : std_logic_vector(1 downto 0);
	signal sweep_port_select : std_logic;
	
	signal sweep_config_write_address : std_logic_vector(12 downto 0);
	signal sweep_config_write_data : std_logic_vector(95 downto 0);
	signal sweep_config_write : std_logic_vector(0 downto 0);
	
	signal sweep_reset : std_logic;
	signal sweep_halted : std_logic;
	signal sweep_resume : std_logic;
	
	signal sweep_excite_port1 : std_logic;
	signal sweep_excite_port2 : std_logic;
	
	signal sweep_band : std_logic;
	
	-- Configuration signals
	signal settling_time : std_logic_vector(15 downto 0);
	signal def_reg_4 : std_logic_vector(31 downto 0);
	signal def_reg_3 : std_logic_vector(31 downto 0);
	signal def_reg_1 : std_logic_vector(31 downto 0);
	signal def_reg_0 : std_logic_vector(31 downto 0);
	signal user_leds : std_logic_vector(2 downto 0);
	signal port1mix_en : std_logic;
	signal port2mix_en : std_logic;
	signal refmix_en : std_logic;	
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
	
	signal debug : std_logic_vector(10 downto 0);
	signal intr : std_logic;
	
	-- DFT signals
	signal dft_bin1_phaseinc : std_logic_vector (15 downto 0);
	signal dft_diffbin_phaseinc : std_logic_vector (15 downto 0);
	signal dft_ready : std_logic;
	signal dft_output : std_logic_vector (191 downto 0);
	signal dft_next_output : std_logic;
	signal dft_enable : std_logic;
	signal dft_reset : std_logic;
begin

	-- Reference CLK LED
	LEDS(0) <= user_leds(2);
	-- Lock status of PLLs
	LEDS(1) <= clk_locked;
	LEDS(2) <= SOURCE_LD;
	LEDS(3) <= LO1_LD;
	-- Sweep and active port
	PORT_SELECT2 <= not sweep_port_select and portswitch_en;
	PORT2_SELECT <= not sweep_port_select and portswitch_en;
	PORT_SELECT1 <= sweep_port_select and portswitch_en;
	PORT1_SELECT <= sweep_port_select and portswitch_en;
	BAND_SELECT_HIGH <= not sweep_band;
	BAND_SELECT_LOW <= sweep_band;
	PORT1_MIX2_EN <= port1mix_en;
	PORT1_MIX1_EN <= not port1mix_en;
	PORT2_MIX2_EN <= port2mix_en;
	PORT2_MIX1_EN <= not port2mix_en;
	REF_MIX2_EN <= refmix_en;
	REF_MIX1_EN <= not refmix_en;
	LEDS(4) <= not (not sweep_reset and not sweep_port_select and portswitch_en);
	LEDS(5) <= not (not sweep_reset and sweep_port_select and portswitch_en);
	-- Uncommitted LEDs
	LEDS(7 downto 6) <= user_leds(1 downto 0);	
	--LEDS(7) <= '0';
	MCU_INTR <= intr;
	--LEDS(6) <= intr;
	
	MainCLK : PLL
	port map(
		-- Clock in ports
		CLK_IN1 => CLK,
		-- Clock out ports
		CLK_OUT1 => clk160,
		-- Status and control signals
		RESET  => RESET,
		LOCKED => clk_locked
	);
	
	inv_clk_locked <= not clk_locked and not RESET;
	
	Inst_ResetDelay: ResetDelay
	GENERIC MAP(CLK_DELAY => 100)
	PORT MAP(
		CLK => clk160,
		IN_RESET => inv_clk_locked,
		OUT_RESET => int_reset
	);
	
	Sync_AUX1 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => MCU_AUX1,
		SYNC_OUT => aux1_sync
	);
	Sync_AUX2 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => MCU_AUX2,
		SYNC_OUT => aux2_sync
	);
	Sync_AUX3 : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => MCU_AUX3,
		SYNC_OUT => aux3_sync
	);
	Sync_LO_LD : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => LO1_LD,
		SYNC_OUT => lo_ld_sync
	);
	Sync_SOURCE_LD : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => SOURCE_LD,
		SYNC_OUT => source_ld_sync
	);	
	Sync_NSS : Synchronizer
	GENERIC MAP(stages => 2)
	PORT MAP(
		CLK => clk160,
		SYNC_IN => MCU_NSS,
		SYNC_OUT => nss_sync
	);	
	

	Source: MAX2871
	GENERIC MAP(CLK_DIV => 10)
	PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		REG4 => source_reg_4,
		REG3 => source_reg_3,
		REG1 => source_reg_1,
		REG0 => source_reg_0,
		RELOAD => reload_plls,
		CLK_OUT => fpga_source_SCK,
		MOSI => fpga_source_MOSI,
		LE => fpga_source_LE,
		DONE => source_reloaded
	);
	LO1: MAX2871
	GENERIC MAP(CLK_DIV => 10)
	PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		REG4 => lo_reg_4,
		REG3 => lo_reg_3,
		REG1 => lo_reg_1,
		REG0 => lo_reg_0,
		RELOAD => reload_plls,
		CLK_OUT => fpga_LO1_SCK,
		MOSI => fpga_LO1_MOSI,
		LE => fpga_LO1_LE,
		DONE => lo_reloaded
	);
	plls_reloaded <= source_reloaded and lo_reloaded;
	plls_locked <= source_ld_sync and lo_ld_sync;

	Port1ADC: MCP33131
	GENERIC MAP(CLK_DIV => 2,
				CONVCYCLES => 77)
	PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		START => adc_trigger_sample,
		READY => adc_port1_ready,
		DATA => adc_port1_data,
		MIN => adc_minmax(15 downto 0),
		MAX => adc_minmax(31 downto 16),
		RESET_MINMAX => adc_reset_minmax,
		SDO => PORT1_SDO,
		CONVSTART => PORT1_CONVSTART,
		SCLK => PORT1_SCLK
	);
	Port2ADC: MCP33131
	GENERIC MAP(CLK_DIV => 2,
				CONVCYCLES => 77)
	PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		START => adc_trigger_sample,
		READY => open, -- synchronous ADCs, ready indicated by port 1 ADC
		DATA => adc_port2_data,
		MIN => adc_minmax(47 downto 32),
		MAX => adc_minmax(63 downto 48),
		RESET_MINMAX => adc_reset_minmax,
		SDO => PORT2_SDO,
		CONVSTART => PORT2_CONVSTART,
		SCLK => PORT2_SCLK
	);
	RefADC: MCP33131
	GENERIC MAP(CLK_DIV => 2,
				CONVCYCLES => 77)
	PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		START => adc_trigger_sample,
		READY => open, -- synchronous ADCs, ready indicated by port 1 ADC
		DATA => adc_ref_data,
		MIN => adc_minmax(79 downto 64),
		MAX => adc_minmax(95 downto 80),
		RESET_MINMAX => adc_reset_minmax,
		SDO => REF_SDO,
		CONVSTART => REF_CONVSTART,
		SCLK => REF_SCLK
	);
	
	
	Windower: Windowing PORT MAP(
		CLK => clk160,
		RESET => sampling_start,
		WINDOW_TYPE => sampling_window,
		PORT1_RAW => adc_port1_data,
		PORT2_RAW => adc_port2_data,
		REF_RAW => adc_ref_data,
		ADC_READY => adc_port1_ready,
		PORT1_WINDOWED => port1_windowed,
		PORT2_WINDOWED => port2_windowed,
		REF_WINDOWED => ref_windowed,
		WINDOWING_DONE => windowing_ready,
		NSAMPLES => sampling_samples
	);
	
	Sampler: Sampling
	GENERIC MAP(CLK_CYCLES_PRE_DONE => 0)
	PORT MAP(
		CLK => clk160,
		RESET => sweep_reset,
		ADC_PRESCALER => sampling_prescaler,
		PHASEINC => sampling_phaseinc,
		PORT1 => port1_windowed,
		PORT2 => port2_windowed,
		REF => ref_windowed,
		ADC_START => adc_trigger_sample,
		NEW_SAMPLE => windowing_ready,
		DONE => sampling_done,
		PRE_DONE => open,
		START => sampling_start,
		SAMPLES => sampling_samples,
		PORT1_I => sampling_result(287 downto 240),
		PORT1_Q => sampling_result(239 downto 192),
		PORT2_I => sampling_result(191 downto 144),
		PORT2_Q => sampling_result(143 downto 96),
		REF_I => sampling_result(95 downto 48),
		REF_Q => sampling_result(47 downto 0),
		ACTIVE => sampling_busy
	);

	sweep_reset <= not aux3_sync;

	SweepModule: Sweep PORT MAP(
		CLK => clk160,
		RESET => sweep_reset,
		NPOINTS => sweep_points,
		CONFIG_ADDRESS => sweep_config_address,
		CONFIG_DATA => sweep_config_data,
		USER_NSAMPLES => sampling_user_samples,
		NSAMPLES => sampling_samples,
		SAMPLING_BUSY => sampling_busy,
		SAMPLING_DONE => sampling_done,
		START_SAMPLING => sampling_start,
		PORT_SELECT => sweep_port_select,
		BAND_SELECT => sweep_band,
		MAX2871_DEF_4 => def_reg_4,
		MAX2871_DEF_3 => def_reg_3,
		MAX2871_DEF_1 => def_reg_1,
		MAX2871_DEF_0 => def_reg_0,
		SOURCE_REG_4 => source_reg_4,
		SOURCE_REG_3 => source_reg_3,
		SOURCE_REG_1 => source_reg_1,
		SOURCE_REG_0 => source_reg_0,
		LO_REG_4 => lo_reg_4,
		LO_REG_3 => lo_reg_3,
		LO_REG_1 => lo_reg_1,
		LO_REG_0 => lo_reg_0,
		RELOAD_PLL_REGS => reload_plls,
		PLL_RELOAD_DONE => plls_reloaded,
		PLL_LOCKED => plls_locked,
		SWEEP_HALTED => sweep_halted,
		SWEEP_RESUME => sweep_resume,
		ATTENUATOR => ATTENUATION,
		SOURCE_FILTER => source_filter,
		EXCITE_PORT1 => sweep_excite_port1,
		EXCITE_PORT2 => sweep_excite_port2,
		DEBUG_STATUS => debug,
		RESULT_INDEX => sampling_result(303 downto 288)
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
	LO1_CLK <= MCU_SCK when aux2_sync = '1' else fpga_LO1_SCK;
	LO1_MOSI <= MCU_MOSI when aux2_sync = '1' else fpga_LO1_MOSI;
	LO1_LE <= MCU_NSS when aux2_sync = '1' else fpga_LO1_LE;
	-- select MISO source
	MCU_MISO <= SOURCE_MUX when aux1_sync = '1' else
					LO1_MUX when aux2_sync = '1' else
					fpga_miso when MCU_NSS = '0' else
					'Z';

	lo_unlocked <= not lo_ld_sync;
	source_unlocked <= not source_ld_sync;

	SPI: SPICommands PORT MAP(
		CLK => clk160,
		RESET => int_reset,
		SCLK => MCU_SCK,
		MOSI => MCU_MOSI,
		MISO => fpga_miso,
		NSS => fpga_select,
		NEW_SAMPLING_DATA => sampling_done,
		SAMPLING_RESULT => sampling_result,
		ADC_MINMAX => adc_minmax,
		SOURCE_UNLOCKED => source_unlocked,
		LO_UNLOCKED => lo_unlocked,
		MAX2871_DEF_4 => def_reg_4,
		MAX2871_DEF_3 => def_reg_3,
		MAX2871_DEF_1 => def_reg_1,
		MAX2871_DEF_0 => def_reg_0,
		SWEEP_DATA => sweep_config_write_data,
		SWEEP_ADDRESS => sweep_config_write_address,
		SWEEP_WRITE => sweep_config_write,
		SWEEP_POINTS => sweep_points,
		NSAMPLES => sampling_user_samples,
		PORT1_EN => port1mix_en,
		PORT2_EN => port2mix_en,
		REF_EN => refmix_en,
		AMP_SHDN => AMP_PWDN,
		SOURCE_RF_EN => SOURCE_RF_EN,
		LO_RF_EN => LO1_RF_EN,
		SOURCE_CE_EN => SOURCE_CE,
		LO_CE_EN => LO1_CE,
		PORTSWITCH_EN => portswitch_en,
		LEDS => user_leds,
		WINDOW_SETTING => sampling_window,
		ADC_PRESCALER => sampling_prescaler,
		ADC_PHASEINC => sampling_phaseinc,
		INTERRUPT_ASSERTED => intr,
		RESET_MINMAX => adc_reset_minmax,
		SWEEP_HALTED => sweep_halted,
		SWEEP_RESUME => sweep_resume,
		EXCITE_PORT1 => sweep_excite_port1,
		EXCITE_PORT2 => sweep_excite_port2,
		DFT_BIN1_PHASEINC => dft_bin1_phaseinc,
		DFT_DIFFBIN_PHASEINC => dft_diffbin_phaseinc,
		DFT_RESULT_READY => dft_ready,
		DFT_OUTPUT => dft_output,
		DFT_NEXT_OUTPUT => dft_next_output,
		DFT_ENABLE => dft_enable,
		DEBUG_STATUS => debug
	);
	
	dft_reset <= not dft_enable;
	
	SA_DFT: DFT GENERIC MAP(BINS => 96)
	PORT MAP(
		CLK => clk160,
		RESET => dft_reset,
		PORT1 => port1_windowed,
		PORT2 => port2_windowed,
		NEW_SAMPLE => windowing_ready,
		NSAMPLES => sampling_samples,
		BIN1_PHASEINC => dft_bin1_phaseinc,
		DIFFBIN_PHASEINC => dft_diffbin_phaseinc,
		RESULT_READY => dft_ready,
		OUTPUT => dft_output,
		NEXT_OUTPUT => dft_next_output
	);
	
	ConfigMem : SweepConfigMem
	PORT MAP (
		clka => clk160,
		ena => '1',
		wea => sweep_config_write,
		addra => sweep_config_write_address,
		dina => sweep_config_write_data,
		clkb => clk160,
		addrb => sweep_config_address,
		doutb => sweep_config_data
	);

end Behavioral;

