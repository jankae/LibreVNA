----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    17:27:54 05/05/2020 
-- Design Name: 
-- Module Name:    Sampling - Behavioral 
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

entity Sampling is
	 Generic(CLK_CYCLES_PRE_DONE : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  ADC_PRESCALER : in STD_LOGIC_VECTOR(7 downto 0);
			  PHASEINC : in STD_LOGIC_VECTOR(11 downto 0);
           PORT1 : in  STD_LOGIC_VECTOR (15 downto 0);
           PORT2 : in  STD_LOGIC_VECTOR (15 downto 0);
           REF : in  STD_LOGIC_VECTOR (15 downto 0);
			  ADC_START : out STD_LOGIC;
           NEW_SAMPLE : in  STD_LOGIC;
           DONE : out  STD_LOGIC;
           PRE_DONE : out  STD_LOGIC;
           START : in  STD_LOGIC;
           SAMPLES : in  STD_LOGIC_VECTOR (9 downto 0);
			  WINDOW_TYPE : in STD_LOGIC_VECTOR (1 downto 0);
           PORT1_I : out  STD_LOGIC_VECTOR (47 downto 0);
           PORT1_Q : out  STD_LOGIC_VECTOR (47 downto 0);
           PORT2_I : out  STD_LOGIC_VECTOR (47 downto 0);
           PORT2_Q : out  STD_LOGIC_VECTOR (47 downto 0);
           REF_I : out  STD_LOGIC_VECTOR (47 downto 0);
           REF_Q : out  STD_LOGIC_VECTOR (47 downto 0);
			  ACTIVE : out STD_LOGIC);
end Sampling;

architecture Behavioral of Sampling is
COMPONENT SinCos
  PORT (
    clk : IN STD_LOGIC;
    phase_in : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    cosine : OUT STD_LOGIC_VECTOR(15 DOWNTO 0);
    sine : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
  );
END COMPONENT;
COMPONENT SinCosMult
  PORT (
    clk : IN STD_LOGIC;
    a : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    b : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    p : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;
COMPONENT window
PORT(
	CLK : IN std_logic;
	INDEX : IN std_logic_vector(6 downto 0);
	WINDOW_TYPE : IN std_logic_vector(1 downto 0);          
	VALUE : OUT std_logic_vector(15 downto 0)
	);
END COMPONENT;

	signal p1_I : signed(47 downto 0);
	signal p1_Q : signed(47 downto 0);
	signal p2_I : signed(47 downto 0);
	signal p2_Q : signed(47 downto 0);
	signal r_I : signed(47 downto 0);
	signal r_Q : signed(47 downto 0);
	signal clk_cnt : integer range 0 to 255;
	signal sample_cnt : integer range 0 to 131071;
	signal samples_to_take : integer range 0 to 131071;
	
	signal phase : std_logic_vector(11 downto 0);
	signal sine : std_logic_vector(15 downto 0);
	signal cosine : std_logic_vector(15 downto 0);
	
	signal windowed_sine : std_logic_vector(31 downto 0);
	signal windowed_cosine : std_logic_vector(31 downto 0);
	
	signal window_index : std_logic_vector(6 downto 0);
	signal window_value : std_logic_vector(15 downto 0);
	signal window_sample_cnt : integer range 0 to 1023;
	
	signal mult1_I : std_logic_vector(31 downto 0);
	signal mult1_Q : std_logic_vector(31 downto 0);
	signal mult2_I : std_logic_vector(31 downto 0);
	signal mult2_Q : std_logic_vector(31 downto 0);
	signal multR_I : std_logic_vector(31 downto 0);
	signal multR_Q : std_logic_vector(31 downto 0);
	
	type States is (Idle, Sampling, WaitForMult, Accumulating, Ready);
	signal state : States;
begin
-- Always fails for simulation, comment out
--	assert (phase_inc * CLK_FREQ / (4096*CLK_DIV) = IF_FREQ)
--		report "Phase increment not exact"
--		severity FAILURE;
		
	LookupTable : SinCos
	PORT MAP (
		clk => CLK,
		phase_in => phase,
		cosine => cosine,
		sine => sine
	);
	Port1_I_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => PORT1,
		b => windowed_cosine(31 downto 16),
		p => mult1_I
	);
	Port1_Q_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => PORT1,
		b => windowed_sine(31 downto 16),
		p => mult1_Q
	);
	Port2_I_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => PORT2,
		b => windowed_cosine(31 downto 16),
		p => mult2_I
	);
	Port2_Q_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => PORT2,
		b => windowed_sine(31 downto 16),
		p => mult2_Q
	);
	Ref_I_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => REF,
		b => windowed_cosine(31 downto 16),
		p => multR_I
	);
	Ref_Q_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => REF,
		b => windowed_sine(31 downto 16),
		p => multR_Q
	);
	
	Sine_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => window_value,
		b => sine,
		p => windowed_sine
	);
	Cosine_Mult : SinCosMult
	PORT MAP (
		clk => CLK,
		a => window_value,
		b => cosine,
		p => windowed_cosine
	);
	WindowROM: window PORT MAP(
		CLK => CLK,
		INDEX => window_index,
		WINDOW_TYPE => WINDOW_TYPE,
		VALUE => window_value
	);
		
	process(CLK, RESET)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				state <= Idle;
				ADC_START <= '0';
				DONE <= '0';
				PRE_DONE <= '0';
				ACTIVE <= '0';
				clk_cnt <= 0;
				sample_cnt <= 0;
				window_sample_cnt <= 0;
				window_index <= (others => '0');
				phase <= (others => '0');
			else
				-- when not idle, generate pulses for ADCs
				if state /= Idle then
					if clk_cnt = unsigned(ADC_PRESCALER) - 1 then
						ADC_START <= '1';
						clk_cnt <= 0;
					else
						clk_cnt <= clk_cnt + 1;
						ADC_START <= '0';
					end if;
				else
					ADC_START <= '0';
				end if;
				-- handle state transitions
				case state is
					when Idle =>
						sample_cnt <= 0;
						DONE <= '0';
						PRE_DONE <= '0';
						ACTIVE <= '0';
						clk_cnt <= 0;
						phase <= (others => '0');
						p1_I <= (others => '0');
						p1_Q <= (others => '0');
						p2_I <= (others => '0');
						p2_Q <= (others => '0');
						r_I <= (others => '0');
						r_Q <= (others => '0');
						phase <= (others => '0');
						if START = '1' then
							state <= Sampling;
							samples_to_take <= to_integer(unsigned(SAMPLES & "0000000") - 1);
						end if;
					when Sampling =>
						DONE <= '0';
						PRE_DONE <= '0';
						ACTIVE <= '1';
						if NEW_SAMPLE = '1' then
							state <= WaitForMult;
						end if;
					when WaitForMult =>
						DONE <= '0';
						PRE_DONE <= '0';
						ACTIVE <= '1';
						state <= Accumulating;
					when Accumulating =>
						-- multipliers are finished with the sample
						p1_I <= p1_I + signed(mult1_I);
						p1_Q <= p1_Q + signed(mult1_Q);
						p2_I <= p2_I + signed(mult2_I);
						p2_Q <= p2_Q + signed(mult2_Q);
						r_I <= r_I + signed(multR_I);
						r_Q <= r_Q + signed(multR_Q);
						-- advance phase
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						phase <= std_logic_vector(unsigned(phase) + unsigned(PHASEINC));
						if sample_cnt < samples_to_take then
							sample_cnt <= sample_cnt + 1;
							state <= Sampling;
						else
							state <= Ready;
						end if;
						-- keep track of window index
						if window_sample_cnt < unsigned(SAMPLES) - 1 then
							window_sample_cnt <= window_sample_cnt + 1;
						else
							window_sample_cnt <= 0;
							window_index <= std_logic_vector( unsigned(window_index) + 1 );
						end if;
					when Ready =>
						ACTIVE <= '1';
						DONE <= '1';
						PRE_DONE <= '1';
						PORT1_I <= std_logic_vector(p1_I);
						PORT1_Q <= std_logic_vector(p1_Q);
						PORT2_I <= std_logic_vector(p2_I);
						PORT2_Q <= std_logic_vector(p2_Q);
						REF_I <= std_logic_vector(r_I);
						REF_Q <= std_logic_vector(r_Q);
						state <= Idle;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

