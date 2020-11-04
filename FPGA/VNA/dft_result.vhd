----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    22:01:17 11/03/2020 
-- Design Name: 
-- Module Name:    dft_result - Behavioral 
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

entity dft_result is
	 Generic (depth : integer);
    Port ( CLK : in  STD_LOGIC;
           READ_ADDRESS : in integer range 0 to depth-1;
           WRITE_ADDRESS : in integer range 0 to depth-1;
           DATA_IN : in  STD_LOGIC_VECTOR (191 downto 0);
           DATA_OUT : out  STD_LOGIC_VECTOR (191 downto 0);
           WE : in  STD_LOGIC);
end dft_result;

architecture rtl of dft_result is
	type memory is array(depth-1 downto 0) of std_logic_vector(191 downto 0);
	signal ram : memory;
begin
	process(CLK)
	begin
		if rising_edge(CLK) then
			DATA_OUT <= ram(READ_ADDRESS);
			if(WE = '1') then
				ram(WRITE_ADDRESS) <= DATA_IN;
			end if;
		end if;
	end process;
end rtl;

