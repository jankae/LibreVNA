----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    13:28:54 09/16/2020 
-- Design Name: 
-- Module Name:    window - Behavioral 
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

entity window is
    Port ( CLK : in  STD_LOGIC;
           INDEX : in  STD_LOGIC_VECTOR (6 downto 0);
           WINDOW_TYPE : in  STD_LOGIC_VECTOR (1 downto 0);
           VALUE : out  STD_LOGIC_VECTOR (15 downto 0));
end window;

architecture Behavioral of window is
	type window_data is array(127 downto 0) of std_logic_vector(15 downto 0);
	
	impure function InitWindowDataFromFile (RomFileName : in string) return window_data is
	FILE romfile : text is in RomFileName;
	variable RomFileLine : line;
	variable rom : window_data;
	begin
	for i in window_data'range loop
	readline(romfile, RomFileLine);
	read(RomFileLine, rom(i));
	end loop;
	return rom;
	end function;
	
	constant hann : window_data := InitWindowDataFromFile("Hann.dat");
	constant kaiser : window_data := InitWindowDataFromFile("Kaiser.dat");
	constant flattop : window_data := InitWindowDataFromFile("Flattop.dat");
	signal i : integer range 0 to 127;
begin

	i <= to_integer(unsigned(INDEX));

	process(CLK)
	begin
		if rising_edge(CLK) then
			case WINDOW_TYPE is
				when "00" => VALUE <= "0001000000000000";
				when "01" => VALUE <= kaiser(i);
				when "10" => VALUE <= hann(i);
				when "11" => VALUE <= flattop(i);
				when others => VALUE <= (others => '0');
			end case;
		end if;
	end process;
end Behavioral;

