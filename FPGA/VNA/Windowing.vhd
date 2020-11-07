----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    18:18:17 11/06/2020 
-- Design Name: 
-- Module Name:    Windowing - Behavioral 
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

entity Windowing is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           WINDOW_TYPE : in  STD_LOGIC_VECTOR (1 downto 0);
           PORT1_RAW : in  STD_LOGIC_VECTOR (15 downto 0);
           PORT2_RAW : in  STD_LOGIC_VECTOR (15 downto 0);
           REF_RAW : in  STD_LOGIC_VECTOR (15 downto 0);
           ADC_READY : in  STD_LOGIC;
           PORT1_WINDOWED : out  STD_LOGIC_VECTOR (17 downto 0);
           PORT2_WINDOWED : out  STD_LOGIC_VECTOR (17 downto 0);
           REF_WINDOWED : out  STD_LOGIC_VECTOR (17 downto 0);
           WINDOWING_DONE : out  STD_LOGIC;
           NSAMPLES : in  STD_LOGIC_VECTOR (12 downto 0));
end Windowing;

architecture Behavioral of Windowing is
COMPONENT window
PORT(
	CLK : IN std_logic;
	INDEX : IN std_logic_vector(6 downto 0);
	WINDOW_TYPE : IN std_logic_vector(1 downto 0);          
	VALUE : OUT std_logic_vector(15 downto 0)
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
	signal window_index : std_logic_vector(6 downto 0);
	signal window_value : std_logic_vector(15 downto 0);
	signal window_sample_cnt : integer range -8 to 8191;
	signal window_index_inc : integer range 0 to 8;
	signal window_sample_compare : integer range 0 to 8191;
	signal window_sample_cnt_inc : integer range 0 to 8;
	
	signal mult_a : std_logic_vector(17 downto 0);
	signal mult_b : std_logic_vector(17 downto 0);
	signal mult_p : std_logic_vector(47 downto 0);
	signal mult_enable : std_logic;
	
	type states is (CalcWindowInc, WaitingForADC, CalcPort1, CalcPort2, CalcRef, MultDelay1, MultDelay2, StorePort1, StorePort2, StoreRef);
	signal state : states;
begin

	Mult : DSP_SLICE
	PORT MAP (
		clk => CLK,
		ce => mult_enable,
		sel => "0",
		a => mult_a,
		b => mult_b,
		c => (others => '0'),
		p => mult_p
	);
	WindowROM: window PORT MAP(
		CLK => CLK,
		INDEX => window_index,
		WINDOW_TYPE => WINDOW_TYPE,
		VALUE => window_value
	);
	-- sign extend multiplier inputs
	mult_a(17 downto 16) <= mult_a(15) & mult_a(15);
	mult_b(17 downto 16) <= mult_b(15) & mult_b(15);
	
	mult_a(15 downto 0) <= window_value;

	process(CLK)
	begin
		if rising_edge(CLK) then
			if RESET = '1' then
				WINDOWING_DONE <= '0';
				state <= CalcWindowInc;
				mult_enable <= '0';
				mult_b(15 downto 0) <= (others => '0');
				window_index <= (others => '0');
				window_sample_cnt <= 0;
				window_sample_compare <= to_integer(unsigned(NSAMPLES));
			else
				case state is
					when CalcWindowInc =>
						case window_sample_compare is
						when 1 =>
							-- 16 samples, increment on every sample by 8
							window_sample_cnt_inc <= 1;
							window_index_inc <= 8;
						when 2 to 3 =>
							-- 32-48 samples, increment by 4
							window_sample_cnt_inc <= 2;
							window_index_inc <= 4;
						when 4 to 7 =>
							-- 64-112 samples, increment by 2
							window_sample_cnt_inc <= 4;
							window_index_inc <= 2;									
						when others =>
							-- 128 or more samples, increment by 1
							window_sample_cnt_inc <= 8;
							window_index_inc <= 1;
					end case;
					state <= WaitingForADC;
				when WaitingForADC =>
					WINDOWING_DONE <= '0';
					mult_enable <= '0';
					mult_b(15 downto 0) <= (others => '0');
					if ADC_READY = '1' then
						state <= CalcPort1;
					end if;
				when CalcPort1 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= PORT1_RAW;
					state <= CalcPort2;
				when CalcPort2 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= PORT2_RAW;
					state <= CalcRef;
				when CalcRef =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= REF_RAW;
					state <= MultDelay1;
				when MultDelay1 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= (others => '0');
					state <= MultDelay2;
				when MultDelay2 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= (others => '0');
					state <= StorePort1;
				when StorePort1 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= (others => '0');
					PORT1_WINDOWED <= mult_p(30 downto 13); 
					state <= StorePort2;
				when StorePort2 =>
					WINDOWING_DONE <= '0';
					mult_enable <= '1';
					mult_b(15 downto 0) <= (others => '0');
					PORT2_WINDOWED <= mult_p(30 downto 13);
					state <= StoreRef;
				when StoreRef =>
					WINDOWING_DONE <= '1';
					mult_enable <= '0';
					mult_b(15 downto 0) <= (others => '0');
					REF_WINDOWED <= mult_p(30 downto 13);
					-- update window increment
					if window_sample_cnt + window_sample_cnt_inc < window_sample_compare then
						window_sample_cnt <= window_sample_cnt + window_sample_cnt_inc;
					else
						window_sample_cnt <= window_sample_cnt + window_sample_cnt_inc - window_sample_compare;
						window_index <= std_logic_vector( unsigned(window_index) + window_index_inc );
					end if;					
					state <= WaitingForADC;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

