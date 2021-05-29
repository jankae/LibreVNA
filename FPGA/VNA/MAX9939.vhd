----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    20:59:58 05/29/2021 
-- Design Name: 
-- Module Name:    MAX9939 - Behavioral 
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

entity MAX9939 is
	 Generic(CLK_DIV : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           GAIN : in  STD_LOGIC_VECTOR (3 downto 0);
           READY : out  STD_LOGIC;
			  CS : out STD_LOGIC;
			  SCLK : out STD_LOGIC;
			  DIN : out STD_LOGIC);
end MAX9939;

architecture Behavioral of MAX9939 is
	signal div_cnt : integer range 0 to (CLK_DIV/2)-1;
	signal chip_select : std_logic;
	type States is (Idle, CLKlow, CLKhigh);
	signal shift_register : std_logic_vector(7 downto 0);
	signal bitcount : integer range 0 to 8;
	signal state : States;
	signal current_gain : std_logic_vector(3 downto 0);
begin

	READY <= chip_select;
	CS <= chip_select;
	
	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			DIN <= shift_register(0);
			if(RESET = '1') then
				state <= Idle;
				chip_select <= '1';
				SCLK <= '0';
				current_gain <= "1111"; -- this is an invalid value, any valid value at GAIN input will trigger the transfer
			else
				case state is
					when Idle =>
						SCLK <= '0';
						chip_select <= '1';
						if(current_gain /= GAIN) then
							current_gain <= GAIN;
							shift_register <= "000" & GAIN & "1";
							bitcount <= 0;
							state <= CLKlow;
							div_cnt <= 0;
						end if;
					when CLKlow =>
						SCLK <= '0';
						chip_select <= '0';
						if div_cnt = CLK_DIV/2 - 1 then
							if bitcount = 8 then
								state <= Idle;
							else
								bitcount <= bitcount + 1;
								div_cnt <= 0;
								state <= CLKhigh;
							end if;
						else
							div_cnt <= div_cnt + 1;
						end if;
					when CLKhigh =>
						SCLK <= '1';
						chip_select <= '0';
						if div_cnt = CLK_DIV/2 - 1 then
							div_cnt <= 0;
							state <= CLKlow;
							shift_register <= "0" & shift_register(7 downto 1);
						else
							div_cnt <= div_cnt + 1;
						end if;
				end case;
			end if;
		end if;
	end process;

end Behavioral;

