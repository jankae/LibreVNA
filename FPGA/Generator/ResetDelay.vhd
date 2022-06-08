----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    20:06:31 05/12/2020 
-- Design Name: 
-- Module Name:    ResetDelay - Behavioral 
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

entity ResetDelay is
	 Generic(CLK_DELAY : integer);
    Port ( CLK : in  STD_LOGIC;
           IN_RESET : in  STD_LOGIC;
           OUT_RESET : out  STD_LOGIC);
end ResetDelay;

architecture Behavioral of ResetDelay is
	signal clk_cnt : integer range 0 to CLK_DELAY-1;
begin

	process(CLK, IN_RESET)
	begin
		if rising_edge(CLK) then
			if IN_RESET = '1' then
				clk_cnt <= 0;
				OUT_RESET <= '1';
			else
				if clk_cnt < CLK_DELAY-1 then
					clk_cnt <= clk_cnt + 1;
				else
					OUT_RESET <= '0';
				end if;
			end if;
		end if;
	end process;


end Behavioral;

