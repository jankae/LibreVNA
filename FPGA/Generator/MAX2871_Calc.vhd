----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    23:02:23 05/17/2022 
-- Design Name: 
-- Module Name:    MAX2871_Calc - Behavioral 
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

entity MAX2871_Calc is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  -- set to 1 to start calculation
           CALC : in  STD_LOGIC;
			  -- desired frequency in terms of reference frequency, fixed point with 27 digits after decimal point
           FREQ : in  STD_LOGIC_VECTOR (32 downto 0);
			  -- minimum possible VCO frequency, fixed point with 27 digits after decimal point
           VCO_MIN : in  STD_LOGIC_VECTOR (31 downto 0);
			  -- outputs 1 for one clk cycle when calculation has finished
           DONE : out  STD_LOGIC;
			  -- MAX2871 register values
           REG0 : out  STD_LOGIC_VECTOR (31 downto 0);
           REG1 : out  STD_LOGIC_VECTOR (31 downto 0);
           REG3 : out  STD_LOGIC_VECTOR (31 downto 0);
           REG4 : out  STD_LOGIC_VECTOR (31 downto 0);
			  -- power selection for RFOUT A
           POWER : in  STD_LOGIC_VECTOR (1 downto 0);
			  -- currently selected VCO
			  VCO_SELECT : out STD_LOGIC_VECTOR (5 downto 0);
			  -- maximum frequency (in terms of reference frequency) up to which the currently selected VCO can be used
			  -- fixed point with 10 digits after decimal point
			  VCO_MAX_FREQ : in STD_LOGIC_VECTOR (15 downto 0)
			  );
end MAX2871_Calc;

architecture Behavioral of MAX2871_Calc is
	COMPONENT RationalApproximation
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		RATIO : IN std_logic_vector(26 downto 0);
		START : IN std_logic;          
		NUM : OUT std_logic_vector(11 downto 0);
		DENOM : OUT std_logic_vector(11 downto 0);
		READY : OUT std_logic
		);
	END COMPONENT;
	
	signal vco_div : integer range 0 to 7;
	signal freq_buf : std_logic_vector(32 downto 0);
	signal vco_min_buf : std_logic_vector(31 downto 0);
	signal power_buf : std_logic_vector(1 downto 0);
	
	signal vco_cnt : integer range 0 to 63;
	
	signal approx_start : std_logic;
	signal approx_ready : std_logic;

	type States is (Idle, VCODiv, VCO, CalcDone);
	signal state : States;
begin
	Approx: RationalApproximation PORT MAP(
		CLK => CLK,
		RESET => RESET,
		NUM => REG0(14 downto 3),
		DENOM => REG1(14 downto 3),
		RATIO => freq_buf(26 downto 0),
		START => approx_start,
		READY => approx_ready
	);
	
	VCO_SELECT <= std_logic_vector(to_unsigned(vco_cnt, 6));
	
	REG0(31) <= '0'; -- always use FRAC mode
	REG0(30 downto 21) <= (others => '0'); -- higher bits of N divider are not used
	REG0(20 downto 15) <= freq_buf(32 downto 27); -- take integer part of freq for N divider
	REG0(2 downto 0) <= "000"; -- select register 0
	
	REG1(31) <= '0'; -- reserved
	REG1(30 downto 29) <= "10"; -- Charge pump linearity: 20%
	REG1(28 downto 27) <= "00"; -- Charge pump: Normal mode
	REG1(26 downto 15) <= (others => '0'); -- phase value is unused
	REG1(2 downto 0) <= "001"; -- select register 1
	
	REG3(31 downto 26) <= std_logic_vector(to_unsigned(vco_cnt, 6)); -- VCO selection
	REG3(25) <= '1'; -- disable VAS
	REG3(24) <= '0'; -- disable VAS temp
	REG3(23 downto 19) <= (others => '0'); -- reserved
	REG3(18) <= '0'; -- enable cycle slip reduction
	REG3(17) <= '0'; -- Do not delay LD
	REG3(16 downto 15) <= "00"; -- Clock divider mode: Mute until lock delay
	REG3(14 downto 3) <= (others => '0'); -- CDIV unused
	REG3(2 downto 0) <= "011"; -- select register 3
	
	REG4(31 downto 29) <= "011"; -- reserved
	REG4(28 downto 26) <= "000"; -- enable LDO, VCO and reference input
	REG4(25 downto 24) <= "11"; -- BS = 1023 (bits 8-9)
	REG4(23) <= '0'; -- divided N counter feedback
	REG4(22 downto 20) <= std_logic_vector(to_unsigned(vco_div, 3)); -- VCO divider selection
	REG4(19 downto 12) <= (others => '1'); -- BS = 1023 (bits 0-7)
	REG4(11) <= '0'; -- enable VCO
	REG4(10) <= '0'; -- disable RFOUT mute
	REG4(9) <= '0'; -- RFOUTB divided output
	REG4(8) <= '0'; -- RFOUTB disabled
	REG4(7 downto 6) <= "00"; -- RFOUTB -4dBm
	REG4(5) <= '1'; -- RFOUTA enabled
	REG4(4 downto 3) <= power_buf; -- RFOUTA power
	REG4(2 downto 0) <= "100"; -- select register 4
	
	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				state <= Idle;
				DONE <= '0';
				approx_start <= '0';
			else
				case state is
					when Idle =>
						DONE <= '0';
						freq_buf <= FREQ;
						vco_min_buf <= VCO_MIN;
						power_buf <= POWER;
						vco_div <= 0;
						vco_cnt <= 0;
						approx_start <= '0';
						if CALC = '1' then
							state <= VCODiv;
						end if;
					when VCODiv =>
						DONE <= '0';
						if unsigned(freq_buf) < unsigned(vco_min_buf) then
							vco_div <= vco_div + 1;
							freq_buf <= freq_buf(32 downto 1) & "0";
						else
							state <= VCO;
						end if;
						approx_start <= '0';
					when VCO =>
						approx_start <= '1';
						DONE <= '0';
						if unsigned(VCO_MAX_FREQ) < unsigned(freq_buf(15 downto 0)) then
							-- select next VCO
							vco_cnt <= vco_cnt + 1;
						else
							-- correct VCO selected, check if divider calculation has finished
							if approx_ready = '1' then
								state <= CalcDone;
							end if;
						end if;
					when CalcDone =>
						approx_start <= '0';
						DONE <= '1';
						state <= Idle;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

