----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:51:11 05/05/2020 
-- Design Name: 
-- Module Name:    SPICommands - Behavioral 
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity SPICommands is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           SCLK : in  STD_LOGIC;
           MOSI : in  STD_LOGIC;
           MISO : out  STD_LOGIC;
           NSS : in  STD_LOGIC;
           NEW_SAMPLING_DATA : in  STD_LOGIC;
           SAMPLING_RESULT : in  STD_LOGIC_VECTOR (287 downto 0);
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
			  INTERRUPT_ASSERTED : out STD_LOGIC;
			  RESET_MINMAX : out STD_LOGIC;
			  SWEEP_HALTED : in STD_LOGIC;
			  SWEEP_RESUME : out STD_LOGIC;
			  DEBUG_STATUS : in STD_LOGIC_VECTOR(10 downto 0));
end SPICommands;

architecture Behavioral of SPICommands is
	COMPONENT spi_slave
	Generic(W : integer);
	PORT(
		SPI_CLK : in  STD_LOGIC;
		MISO : out  STD_LOGIC;
		MOSI : in  STD_LOGIC;
		CS : in  STD_LOGIC;
		BUF_OUT : out STD_LOGIC_VECTOR (W-1 downto 0) := (others => '0');
		BUF_IN : in STD_LOGIC_VECTOR (W-1 downto 0);
		CLK : in  STD_LOGIC;
		COMPLETE : out STD_LOGIC
	);
	END COMPONENT;
	
	-- SPI control signals
	signal spi_buf_out : std_logic_vector(15 downto 0);
	signal spi_buf_in : std_logic_vector(15 downto 0);
	signal spi_complete : std_logic;
	signal word_cnt : integer range 0 to 19;
	type SPI_states is (Invalid, WriteSweepConfig, ReadResult, WriteRegister, ReadTest);
	signal state : SPI_states;
	signal selected_register : integer range 0 to 15;
	
	signal sweep_config_write : std_logic;
	signal unread_sampling_data : std_logic;
	signal data_overrun : std_logic;
	-- Configuration registers
	signal interrupt_mask : std_logic_vector(15 downto 0);
	signal interrupt_status : std_logic_vector(15 downto 0);
	
	signal latched_result : std_logic_vector(271 downto 0);
	signal sweepconfig_buffer : std_logic_vector(79 downto 0);
begin
	SPI: spi_slave
	GENERIC MAP(w => 16)
	PORT MAP(
		SPI_CLK => SCLK,
		MISO => MISO,
		MOSI => MOSI,
		CS => NSS,
		BUF_OUT => spi_buf_out,
		BUF_IN => spi_buf_in,
		CLK => CLK,
		COMPLETE =>spi_complete 
	);
	
	interrupt_status <= DEBUG_STATUS & SWEEP_HALTED & data_overrun & unread_sampling_data & SOURCE_UNLOCKED & LO_UNLOCKED;
	INTERRUPT_ASSERTED <= '1' when (interrupt_status and interrupt_mask) /= "0000000000000000" else
									'0';

	SWEEP_WRITE(0) <= sweep_config_write;

	process(CLK, RESET)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				sweep_config_write <= '0';
				data_overrun <= '0';
				SWEEP_POINTS <= (others => '0');
				NSAMPLES <= (others => '0');
				--SETTLING_TIME <= (others => '0');
				PORT1_EN <= '0';
				PORT2_EN <= '0';
				REF_EN <= '0';
				AMP_SHDN <= '1';
				SOURCE_RF_EN <= '0';
				LO_RF_EN <= '0';
				SOURCE_CE_EN <= '0';
				LO_CE_EN <= '0';
				PORTSWITCH_EN <= '0';
				LEDS <= (others => '1');
				WINDOW_SETTING <= "00";
				unread_sampling_data <= '0';
				interrupt_mask <= (others => '0');
				ADC_PRESCALER <= std_logic_vector(to_unsigned(112, 8));
				ADC_PHASEINC <= std_logic_vector(to_unsigned(1120, 12));
				RESET_MINMAX <= '0';
			else
				if sweep_config_write = '1' then
					sweep_config_write <= '0';
				end if;
				if NEW_SAMPLING_DATA = '1' then
					unread_sampling_data <= '1';
					if unread_sampling_data = '1' then
						data_overrun <= '1';
					end if;
				end if;
				if NSS = '1' then
					word_cnt <= 0;
					spi_buf_in <= interrupt_status;
					RESET_MINMAX <= '0';
					SWEEP_RESUME <= '0';
				elsif spi_complete = '1' then
					word_cnt <= word_cnt + 1;
					if word_cnt = 0 then
						-- initial word determines action
						case spi_buf_out(15 downto 13) is
							when "000" => state <= WriteSweepConfig;
											-- also extract the point number
											SWEEP_ADDRESS <= spi_buf_out(12 downto 0);
							when "001" => state <= Invalid;
											SWEEP_RESUME <= '1';
							when "010" => state <= ReadTest;
											spi_buf_in <= "1111000010100101";
							when "011" => state <= Invalid;
											RESET_MINMAX <= '1';
							when "100" => state <= WriteRegister;
											selected_register <= to_integer(unsigned(spi_buf_out(3 downto 0)));
							when "110" => state <= ReadResult;
											latched_result <= SAMPLING_RESULT(287 downto 16);
											spi_buf_in <= SAMPLING_RESULT(15 downto 0);
											unread_sampling_data <= '0';
							when "111" => state <= ReadResult; -- can use same state as read result, but the latched data will contain the min/max ADC values
											latched_result(79 downto 0) <= ADC_MINMAX(95 downto 16);
											spi_buf_in <= ADC_MINMAX(15 downto 0);
							when others => state <= Invalid;
						end case;
					else
						if state = WriteRegister then
							-- write received data into previously selected register
							case selected_register is
								when 0 => interrupt_mask <= spi_buf_out;
								when 1 => SWEEP_POINTS <= spi_buf_out(12 downto 0);
								when 2 => NSAMPLES <= spi_buf_out(12 downto 0);
								when 3 => PORTSWITCH_EN <= spi_buf_out(0);
											PORT1_EN <= spi_buf_out(15);
											PORT2_EN <= spi_buf_out(14);
											REF_EN <= spi_buf_out(13);
											AMP_SHDN <= not spi_buf_out(12);
											SOURCE_RF_EN <= spi_buf_out(11);
											LO_RF_EN <= spi_buf_out(10);
											LEDS <= not spi_buf_out(9 downto 7);
											WINDOW_SETTING <= spi_buf_out(6 downto 5);
											SOURCE_CE_EN <= spi_buf_out(4);
											LO_CE_EN <= spi_buf_out(3);
											EXCITE_PORT1 <= spi_buf_out(1);
											EXCITE_PORT2 <= spi_buf_out(2);
								when 4 => ADC_PRESCALER <= spi_buf_out(7 downto 0);
								when 5 => ADC_PHASEINC <= spi_buf_out(11 downto 0);
								when 8 => MAX2871_DEF_0(15 downto 0) <= spi_buf_out;
								when 9 => MAX2871_DEF_0(31 downto 16) <= spi_buf_out;
								when 10 => MAX2871_DEF_1(15 downto 0) <= spi_buf_out;
								when 11 => MAX2871_DEF_1(31 downto 16) <= spi_buf_out;
								when 12 => MAX2871_DEF_3(15 downto 0) <= spi_buf_out;
								when 13 => MAX2871_DEF_3(31 downto 16) <= spi_buf_out;
								when 14 => MAX2871_DEF_4(15 downto 0) <= spi_buf_out;
								when 15 => MAX2871_DEF_4(31 downto 16) <= spi_buf_out;
								when others => 
							end case;
							selected_register <= selected_register + 1;
						elsif state = WriteSweepConfig then
							if word_cnt = 6 then
								-- Sweep config data is complete pass on
								SWEEP_DATA <= sweepconfig_buffer & spi_buf_out;
								sweep_config_write <= '1';
							else
								-- shift next word into buffer
								sweepconfig_buffer <= sweepconfig_buffer(63 downto 0) & spi_buf_out;
							end if;
						elsif state = ReadResult then
							-- pass on next word of latched result
							spi_buf_in <= latched_result(15 downto 0);
							latched_result <= "0000000000000000" & latched_result(271 downto 16);
						end if;
					end if;
				end if;
			end if;
		end if;
	end process;

end Behavioral;

