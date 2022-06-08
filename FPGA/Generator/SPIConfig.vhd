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
			  SOURCE_FILTER : out STD_LOGIC_VECTOR(1 downto 0);
			  SOURCE_POWER : out STD_LOGIC_VECTOR(1 downto 0);
			  SOURCE_ATTENUATION : out STD_LOGIC_VECTOR(6 downto 0);
			  SOURCE_BANDSELECT : out STD_LOGIC; -- 0: highband, 1: lowband
			  SOURCE_PORTSELECT : out STD_LOGIC; -- 0: Port 1, 1: Port 2
           SOURCE_UNLOCKED : in  STD_LOGIC;

			  -- SOURCE VCO lookup table
           SOURCE_VCO_INDEX : out  STD_LOGIC_VECTOR (5 downto 0);
			  SOURCE_VCO_MAXFREQ : out STD_LOGIC_VECTOR (15 downto 0);
			  SOURCE_VCO_WRITE : out STD_LOGIC;
			  
			  -- Modulation FIFO signals
			  MOD_FIFO_DATA : out STD_LOGIC_VECTOR (7 downto 0);
			  MOD_FIFO_WRITE : out STD_LOGIC;
			  MOD_FIFO_UNDERFLOW : in STD_LOGIC;
			  MOD_FIFO_OVERFLOW : in STD_LOGIC;
			  MOD_FIFO_THRESHOLD_CROSSED : in STD_LOGIC;
			  MOD_FIFO_THRESHOLD : out STD_LOGIC_VECTOR (10 downto 0);
			  
			  -- Modulation control signals
			  MOD_ENABLE : out STD_LOGIC;
			  MOD_PHASE_INC : out STD_LOGIC_VECTOR (15 downto 0);
			  MOD_CENTER_FREQ : out STD_LOGIC_VECTOR (32 downto 0);
			  MOD_DEVIATION_FREQ : out STD_LOGIC_VECTOR (25 downto 0);
			  MOD_AM_DEPTH : out STD_LOGIC_VECTOR (6 downto 0);
			  MOD_VCO_MIN : out STD_LOGIC_VECTOR (31 downto 0);
			  
			  AMP_SHDN : out STD_LOGIC;
			  SOURCE_RF_EN : out STD_LOGIC;
			  SOURCE_CE_EN : out STD_LOGIC;
			  PORTSWITCH_EN : out STD_LOGIC;
			  LEDS : out STD_LOGIC_VECTOR(2 downto 0);
			  INTERRUPT_ASSERTED : out STD_LOGIC);
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
	type SPI_states is (FirstWord, WriteVCOTable, WriteModulationData, WriteRegister);
	signal state : SPI_states;
	signal selected_register : integer range 0 to 31;
	
	signal last_NSS : std_logic;
	
	signal VCO_table_write : std_logic;
	signal mod_first_byte : std_logic;
	signal mod_second_byte : std_logic;
	signal mod_data_LSB : std_logic_vector(7 downto 0);
	
	-- Configuration registers
	signal interrupt_mask : std_logic_vector(15 downto 0);
	signal interrupt_status : std_logic_vector(15 downto 0);
	
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
	
	MOD_FIFO_WRITE <= mod_first_byte or mod_second_byte;
	SOURCE_VCO_WRITE <= VCO_table_write;
	
	process(CLK, RESET)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				AMP_SHDN <= '1';
				SOURCE_RF_EN <= '0';
				SOURCE_CE_EN <= '0';
				PORTSWITCH_EN <= '0';
				LEDS <= (others => '1');
				interrupt_status <= (others => '0');
				interrupt_mask <= (others => '0');
				INTERRUPT_ASSERTED <= '0';
				last_NSS <= '1';
				MOD_ENABLE <= '0';
			else
				interrupt_status <= "00000000000" & MOD_FIFO_THRESHOLD_CROSSED & MOD_FIFO_UNDERFLOW & MOD_FIFO_OVERFLOW & SOURCE_UNLOCKED & "0";
				if (interrupt_status and interrupt_mask) = "0000000000000000" then
					INTERRUPT_ASSERTED <= '0';
				else
					INTERRUPT_ASSERTED <= '1';
				end if;
				if mod_first_byte = '1' then
					mod_first_byte <= '0';
					mod_second_byte <= '1';
					MOD_FIFO_DATA <= mod_data_LSB;
				end if;
				if mod_second_byte = '1' then
					mod_second_byte <= '0';
				end if;
				if VCO_table_write = '1' then
					VCO_table_write <= '0';
				end if;
				last_NSS <= NSS;
				if NSS = '0' and last_NSS = '1' then
					word_cnt <= 0;
					spi_buf_in <= interrupt_status;
					state <= FirstWord;
				elsif spi_complete = '1' then
					word_cnt <= word_cnt + 1;
					case state is
					when FirstWord =>
						-- initial word determines action
						case spi_buf_out(15 downto 13) is
							when "000" => state <= WriteVCOTable;
											-- also extract the point number
											SOURCE_VCO_INDEX <= spi_buf_out(5 downto 0);
							when "010" => state <= FirstWord;
											spi_buf_in <= "1111000010100101";
							when "100" => state <= WriteRegister;
											selected_register <= to_integer(unsigned(spi_buf_out(4 downto 0)));
							when "001" => state <= WriteModulationData;
							when others => state <= FirstWord;
						end case;
					when WriteRegister =>
						-- write received data into previously selected register
						case selected_register is
							when 0 => interrupt_mask <= spi_buf_out;
							when 1 => SOURCE_FILTER <= spi_buf_out(1 downto 0);
										SOURCE_POWER <= spi_buf_out(3 downto 2);
										SOURCE_ATTENUATION <= spi_buf_out(10 downto 4);
										SOURCE_BANDSELECT <= spi_buf_out(11);
										SOURCE_PORTSELECT <= spi_buf_out(12);
										SOURCE_CE_EN <= spi_buf_out(13);
										SOURCE_RF_EN <= spi_buf_out(14);
										AMP_SHDN <= spi_buf_out(15);
							when 2 => MOD_ENABLE <= spi_buf_out(0);
										MOD_AM_DEPTH <= spi_buf_out(7 downto 1);
										LEDS <= not spi_buf_out(15 downto 13);
							when 3 => MOD_PHASE_INC <= spi_buf_out;
							when 4 => MOD_CENTER_FREQ(15 downto 0) <= spi_buf_out;
							when 5 => MOD_CENTER_FREQ(31 downto 16) <= spi_buf_out;
							when 6 => MOD_DEVIATION_FREQ(15 downto 0) <= spi_buf_out;			
							when 7 => MOD_CENTER_FREQ(32) <= spi_buf_out(15);
										MOD_DEVIATION_FREQ(25 downto 16) <= spi_buf_out(9 downto 0);
							when 8 => MOD_VCO_MIN(15 downto 0) <= spi_buf_out;
							when 9 => MOD_VCO_MIN(31 downto 16) <= spi_buf_out;	
							when 10 => MOD_FIFO_THRESHOLD <= spi_buf_out(10 downto 0);
							when others => 
						end case;
						selected_register <= selected_register + 1;
					when WriteVCOTable =>
						SOURCE_VCO_MAXFREQ <= spi_buf_out;
						VCO_table_write <= '1';
					when WriteModulationData =>
						-- add two new bytes to the modulation data
						MOD_FIFO_DATA <= spi_buf_out(15 downto 8);
						mod_first_byte <= '1';
						mod_data_LSB <= spi_buf_out(7 downto 0);
					end case;
				end if;
			end if;
		end if;
	end process;

end Behavioral;

