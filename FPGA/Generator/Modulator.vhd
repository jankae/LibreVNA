----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    23:19:57 06/06/2022 
-- Design Name: 
-- Module Name:    Modulator - Behavioral 
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
use std.textio.all;
use ieee.std_logic_textio.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Modulator is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  ACTIVE : in STD_LOGIC;
			  -- Determines sample rate
           SAMPLE_FREQ_WORD : in  STD_LOGIC_VECTOR (15 downto 0);
			  -- Input data, latched when SAMPLE_LATCH goes high
           SAMPLE_DATA : in  STD_LOGIC_VECTOR (7 downto 0);
           SAMPLE_LATCH : in  STD_LOGIC;
			  -- internal FIFO overflows, previous sample has been overwritten, active until reset
           OVERFLOW : out  STD_LOGIC;
			  -- internal FIFO empty, reset when the next sample is added
           UNDERFLOW : out  STD_LOGIC;
			  -- number of internally stored samples after which THRESHOLD_CROSSED gets asserted
           THRESHOLD_LEVEL : in  STD_LOGIC_VECTOR (10 downto 0);
			  -- high when the FIFO contains at least THRESHOLD_LEVEL number of samples
           THRESHOLD_CROSSED : out  STD_LOGIC;
			  -- center frequency of the FM (in terms of PLL reference frequency, fixed point with 27 digits after decimal point)
           FREQ_CENTER : in  STD_LOGIC_VECTOR (32 downto 0);
			  -- frequency deviation from center at maximum modulation
			  -- (in terms of PLL reference frequency, fixed point with 27 digits after decimal point -> maximum value ~0.5)
           FREQ_DEVIATION : in  STD_LOGIC_VECTOR (25 downto 0);
			  -- attenuator setting for "no modulation"
           MIN_ATTENUATION : in  STD_LOGIC_VECTOR (6 downto 0);
			  -- AM depth in percent
           AMPLITUDE_DEPTH : in  STD_LOGIC_VECTOR (6 downto 0);
			  -- modulated frequency (in terms of PLL reference frequency, fixed point with 27 digits after decimal point)
           FREQUENCY : out  STD_LOGIC_VECTOR (32 downto 0);
			  -- modulated attenuator setting
           ATTENUATOR : out  STD_LOGIC_VECTOR (6 downto 0);
			  -- signals that a new output has been generated
           NEW_OUTPUT : out  STD_LOGIC);
end Modulator;

architecture Behavioral of Modulator is
	COMPONENT SampleMemory
	  PORT (
		 clka : IN STD_LOGIC;
		 wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		 addra : IN STD_LOGIC_VECTOR(10 DOWNTO 0);
		 dina : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		 clkb : IN STD_LOGIC;
		 addrb : IN STD_LOGIC_VECTOR(10 DOWNTO 0);
		 doutb : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	  );
	END COMPONENT;
	
	COMPONENT wide_mult
	PORT (
		 clk : IN STD_LOGIC;
		 a : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
		 b : IN STD_LOGIC_VECTOR(26 DOWNTO 0);
		 ce : IN STD_LOGIC;
		 p : OUT STD_LOGIC_VECTOR(39 DOWNTO 0)
	  );
	END COMPONENT;
	
	COMPONENT AMMult
	PORT (
		clk : IN STD_LOGIC;
		a : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		b : IN STD_LOGIC_VECTOR(6 DOWNTO 0);
		ce : IN STD_LOGIC;
		p : OUT STD_LOGIC_VECTOR(14 DOWNTO 0)
	);
	END COMPONENT;

	signal fm_mult_a : STD_LOGIC_VECTOR(12 DOWNTO 0);
	signal fm_mult_b : STD_LOGIC_VECTOR(26 DOWNTO 0);
	signal fm_mult_p : STD_LOGIC_VECTOR(39 DOWNTO 0);
	
	signal am_mult_a : STD_LOGIC_VECTOR(7 DOWNTO 0);
	signal am_mult_b : STD_LOGIC_VECTOR(6 DOWNTO 0);
	signal am_mult_p : STD_LOGIC_VECTOR(14 DOWNTO 0);

	signal mult_ce : STD_LOGIC;
	
	signal mult_pipe : integer range 0 to 9;

	signal write_pos : unsigned(10 downto 0);
	signal read_pos : unsigned(10 downto 0);
	
	signal sample : std_logic_vector(7 downto 0);
	
	signal clk_sample_cnt : unsigned(26 downto 0);
	
	type AMdepthTable is array(0 to 127) of std_logic_vector(6 downto 0);
	
	impure function InitWindowDataFromFile (RomFileName : in string) return AMdepthTable is
	FILE romfile : text is in RomFileName;
	variable RomFileLine : line;
	variable rom : AMdepthTable;
	begin
	for i in AMdepthTable'range loop
	readline(romfile, RomFileLine);
	read(RomFileLine, rom(i));
	end loop;
	return rom;
	end function;
	
	constant AMdepth : AMdepthTable := InitWindowDataFromFile("AMdepth.dat");
	
	signal am_attenuation : unsigned(6 downto 0);
	
begin
	Mem : SampleMemory
	PORT MAP (
		clka => CLK,
		wea(0) => SAMPLE_LATCH,
		addra => std_logic_vector(write_pos),
		dina => SAMPLE_DATA,
		clkb => CLK,
		addrb => std_logic_vector(read_pos),
		doutb => sample
	);
	
	fm_mult_b <= "0" & FREQ_DEVIATION;
	am_mult_b <= AMPLITUDE_DEPTH;
	
	FM_Mult: wide_mult
	PORT MAP (
		clk => CLK,
		a => fm_mult_a,
		b => fm_mult_b,
		ce => mult_ce,
		p => fm_mult_p
	);
	
	AM_Mult : AMMult
	PORT MAP (
		clk => CLK,
		a => am_mult_a,
		b => am_mult_b,
		ce => mult_ce,
		p => am_mult_p
	);

	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if RESET = '1' then
				write_pos <= (others => '0');
				read_pos <= (others => '1');
				OVERFLOW <= '0';
				UNDERFLOW <= '0';
				THRESHOLD_CROSSED <= '0';
				clk_sample_cnt <= (others => '0');
				mult_pipe <= 0;
			else
				-- update threshold
				if write_pos - read_pos >= unsigned(THRESHOLD_LEVEL) then
					THRESHOLD_CROSSED <= '1';
				else
					THRESHOLD_CROSSED <= '0';
				end if;
				if SAMPLE_LATCH = '1' then
					UNDERFLOW <= '0';
					-- adding new input sample, advance write position
					if write_pos = read_pos then
						-- some data has been overwritten
						OVERFLOW <= '1';
					end if;
					write_pos <= write_pos + 1;
				end if;
				if ACTIVE = '1' then
					clk_sample_cnt <= clk_sample_cnt + unsigned(SAMPLE_FREQ_WORD);
				end if;
				if clk_sample_cnt(26) = '1' then
					-- take the next sample
					clk_sample_cnt(26) <= '0';
					if read_pos + 1 = write_pos then
						UNDERFLOW <= '1';
					else
						read_pos <= read_pos + 1;
					end if;
					mult_pipe <= 9;
					mult_ce <= '1';
					fm_mult_a <= "00000" & sample;
					am_mult_a <= sample;
				end if;
				if mult_pipe > 0 then
					mult_pipe <= mult_pipe - 1;
				end if;
				if mult_pipe = 4 then
					-- multiplier result is ready
					mult_ce <= '0';
					FREQUENCY <= std_logic_vector(unsigned(FREQ_CENTER) + unsigned(fm_mult_p(33 downto 8)));
					am_attenuation <= unsigned(AMdepth(to_integer(unsigned(am_mult_p(14 downto 8)))));
				end if;
				if mult_pipe = 3 then
					am_attenuation <= unsigned(MIN_ATTENUATION) + am_attenuation;
				end if;
				if mult_pipe = 2 then
					if am_attenuation < unsigned(MIN_ATTENUATION) then
						-- attenuator overflowed, use maximum values instead
						ATTENUATOR <= (others => '1');
					else
						ATTENUATOR <= std_logic_vector(am_attenuation);
					end if;
					NEW_OUTPUT <= '1';
				end if;
				if mult_pipe = 1 then
					NEW_OUTPUT <= '0';
				end if;
			end if;
		end if;
	end process;

end Behavioral;

