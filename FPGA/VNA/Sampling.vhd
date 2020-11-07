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
           PORT1 : in  STD_LOGIC_VECTOR (17 downto 0);
           PORT2 : in  STD_LOGIC_VECTOR (17 downto 0);
           REF : in  STD_LOGIC_VECTOR (17 downto 0);
			  ADC_START : out STD_LOGIC;
           NEW_SAMPLE : in  STD_LOGIC;
           DONE : out  STD_LOGIC;
           PRE_DONE : out  STD_LOGIC;
           START : in  STD_LOGIC;
           SAMPLES : in  STD_LOGIC_VECTOR (12 downto 0);
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
COMPONENT DSP_SLICE
	PORT (
		clk : IN STD_LOGIC;
		ce : IN STD_LOGIC;
		sel : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		a : IN STD_LOGIC_VECTOR(17 DOWNTO 0);
		b : IN STD_LOGIC_VECTOR(17 DOWNTO 0);
		c : IN STD_LOGIC_VECTOR(47 DOWNTO 0);
		p : OUT STD_LOGIC_VECTOR(47 DOWNTO 0)
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

	signal p1_I : std_logic_vector(47 downto 0);
	signal p1_Q : std_logic_vector(47 downto 0);
	signal p2_I : std_logic_vector(47 downto 0);
	signal p2_Q : std_logic_vector(47 downto 0);
	signal r_I : std_logic_vector(47 downto 0);
	signal r_Q : std_logic_vector(47 downto 0);
	signal clk_cnt : integer range 0 to 255;
	signal sample_cnt : integer range 0 to 131071;
	signal samples_to_take : integer range 0 to 131071;
	
	signal phase : std_logic_vector(11 downto 0);
	signal sine : std_logic_vector(15 downto 0);
	signal cosine : std_logic_vector(15 downto 0);
	
	signal mult_a : std_logic_vector(17 downto 0);
	signal mult_b : std_logic_vector(17 downto 0);
	signal mult_c : std_logic_vector(47 downto 0);
	signal mult_p : std_logic_vector(47 downto 0);
	
	signal mult_enable : std_logic;
	signal mult_accumulate : std_logic_vector(0 downto 0);
	
	type States is (Idle, Sampling, P1Q, P2I, P2Q, RI, RQ, SaveP1Q, SaveP2I, SaveP2Q, SaveRI, SaveRQ, Ready);
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

	Mult : DSP_SLICE
	PORT MAP (
		clk => CLK,
		ce => mult_enable,
		sel => mult_accumulate,
		a => mult_a,
		b => mult_b,
		c => mult_c,
		p => mult_p
	);
	
	-- sign extend b input of multiplier (sin/cos)
	mult_b(17 downto 16) <= mult_b(15) & mult_b(15);
	
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
				phase <= (others => '0');
				mult_enable <= '0';
				mult_accumulate <= "0";
			else
				-- when not idle, generate pulses for ADCs
				if state /= Idle then
					if clk_cnt = unsigned(ADC_PRESCALER) - 1 then
						if sample_cnt < samples_to_take then
							ADC_START <= '1';
						end if;
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
						mult_enable <= '0';
						mult_accumulate <= "0";
						if START = '1' then
							state <= Sampling;
							samples_to_take <= to_integer(unsigned(SAMPLES & "0000"));
						end if;
					when Sampling =>
						DONE <= '0';
						PRE_DONE <= '0';
						ACTIVE <= '1';
						mult_enable <= '0';
						if NEW_SAMPLE = '1' then
							sample_cnt <= sample_cnt + 1;
							mult_enable <= '1';
							mult_a <= PORT1;
							mult_b(15 downto 0) <= cosine;
							mult_c <= p1_I;
							state <= P1Q;
						end if;
					when P1Q =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						mult_a <= PORT1;
						mult_b(15 downto 0) <= sine;
						mult_c <= p1_Q;
						state <= P2I;
					when P2I =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						mult_a <= PORT2;
						mult_b(15 downto 0) <= cosine;
						mult_c <= p2_I;
						state <= P2Q;
					when P2Q =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						mult_a <= PORT2;
						mult_b(15 downto 0) <= sine;
						mult_c <= p2_Q;
						state <= RI;
					when RI =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						mult_a <= REF;
						mult_b(15 downto 0) <= cosine;
						mult_c <= r_I;
						state <= RQ;						
					when RQ =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						mult_a <= REF;
						mult_b(15 downto 0) <= sine;
						mult_c <= r_Q;
						-- first result is available
						p1_I <= mult_p;
						state <= SaveP1Q;
					when SaveP1Q =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						p1_Q <= mult_p;
						state <= SaveP2I;
					when SaveP2I =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						p2_I <= mult_p;
						state <= SaveP2Q;
					when SaveP2Q =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						p2_Q <= mult_p;
						state <= SaveRI;
					when SaveRI =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '1';
						r_I <= mult_p;
						state <= SaveRQ;
					when SaveRQ =>
						ACTIVE <= '1';
						DONE <= '0';
						PRE_DONE <= '0';
						mult_enable <= '0';
						r_Q <= mult_p;
						-- from now on accumulate results
						mult_accumulate <= "1";
						phase <= std_logic_vector(unsigned(phase) + unsigned(PHASEINC));
						if sample_cnt < samples_to_take then
							state <= Sampling;
						else
							state <= Ready;
						end if;
					when Ready =>
						ACTIVE <= '1';
						DONE <= '1';
						PRE_DONE <= '1';
						mult_enable <= '0';
						PORT1_I <= p1_I;
						PORT1_Q <= p1_Q;
						PORT2_I <= p2_I;
						PORT2_Q <= p2_Q;
						REF_I <= r_I;
						REF_Q <= r_Q;
						state <= Idle;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

