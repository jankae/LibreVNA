----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    00:02:23 05/10/2022 
-- Design Name: 
-- Module Name:    RationalApproximation - Behavioral 
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

entity RationalApproximation is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           NUM : out  STD_LOGIC_VECTOR (11 downto 0);
           DENOM : out  STD_LOGIC_VECTOR (11 downto 0);
           RATIO : in  STD_LOGIC_VECTOR (26 downto 0);
           START : in  STD_LOGIC;
           READY : out  STD_LOGIC);
end RationalApproximation;

architecture Behavioral of RationalApproximation is
	COMPONENT wide_mult
	PORT (
		 clk : IN STD_LOGIC;
		 a : IN STD_LOGIC_VECTOR(12 DOWNTO 0);
		 b : IN STD_LOGIC_VECTOR(26 DOWNTO 0);
		 ce : IN STD_LOGIC;
		 p : OUT STD_LOGIC_VECTOR(39 DOWNTO 0)
	  );
	END COMPONENT;
	
	signal a : integer range 0 to 8192;
	signal b : integer range 0 to 8192;
	signal c : integer range 0 to 8192;
	signal d : integer range 0 to 8192;
	
	signal compare_ratio : unsigned(39 downto 0);
	signal compare_median : unsigned(39 downto 0);
		
	signal mult_a : std_logic_vector(12 downto 0);
	signal mult_p : std_logic_vector(39 downto 0);
	signal mult_ce : std_logic;
	signal mult_pipe : integer range 0 to 5;
	
	type States is (Idle, StartMultiplier, WaitMultiplier, CheckResult, Done);
	signal state : States;
begin

	Mult : wide_mult
	PORT MAP (
		clk => CLK,
		a => mult_a,
		b => RATIO,
		ce => mult_ce,
		p => mult_p
	);

	compare_ratio <= unsigned(mult_p);

	READY <= '1' when state = Done else '0';
	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				a <= 0;
				b <= 1;
				c <= 1;
				d <= 1;
				NUM <= (others => '0');
				DENOM <= (others => '0');
				state <= Idle;
				mult_ce <= '0';
			else
				if start = '0' then
					state <= Idle;
					mult_ce <= '0';
				else 
					-- start is active
					case state is
						when Idle =>
							a <= 0;
							b <= 1;
							c <= 1;
							d <= 1;
							state <= StartMultiplier;
						when StartMultiplier =>
							mult_a <= std_logic_vector(to_unsigned(b + d, 13));
							compare_median <= unsigned(std_logic_vector(to_unsigned(a + c, 13)) & b"000_0000_0000_0000_0000_0000_0000");
							mult_ce <= '1';
							mult_pipe <= 5;
							state <= WaitMultiplier;
						when WaitMultiplier =>
							if unsigned(mult_a) < 4096 then
								if mult_pipe = 0 then
									state <= CheckResult;
									mult_ce <= '0';
								else
									mult_pipe <= mult_pipe - 1;
								end if;
							else
								-- got too far, return best approximation
								NUM <= std_logic_vector(to_unsigned(a, 12));
								DENOM <= std_logic_vector(to_unsigned(b, 12));
								state <= Done;
								mult_ce <= '0';
							end if;
						when CheckResult =>
							if compare_ratio = compare_median then
								-- mult_a still contains b + d
								if unsigned(mult_a) < 4096 then
									NUM <= std_logic_vector(to_unsigned(a + c, 12));
									DENOM <= mult_a(11 downto 0);
								elsif d > b then
									NUM <= std_logic_vector(to_unsigned(c, 12));
									DENOM <= std_logic_vector(to_unsigned(d, 12));
								else
									NUM <= std_logic_vector(to_unsigned(a, 12));
									DENOM <= std_logic_vector(to_unsigned(b, 12));
								end if;
								state <= Done;
							else
								if compare_ratio > compare_median then
									a <= a + c;
									b <= b + d;
								else
									c <= a + c;
									d <= b + d;
								end if;
								state <= StartMultiplier;
							end if;
						when others =>
					end case;
				end if;
			end if;
		end if;
	end process;

end Behavioral;

